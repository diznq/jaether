#include "Class.h"
#include "Frame.h"
#include "CPU.h"
#include "ObjectHelper.h"

namespace jaether {

	vClass::vClass(vContext* ctx, vCPU* cpu, const char* name, const int nesting) {
		std::ifstream f(name, std::ios::binary);
		if (f) {
			vUINT magic = readUI(f);
			vUSHORT minor = readUSI(f), major = readUSI(f);
			vUSHORT consts = readUSI(f);
			_constCount = consts;
			vCOMMON ops[8];
			_constPool = VMAKE(vMemory, ctx, ctx, (size_t)consts);
			// Parse const pool
			for (vUSHORT i = 1; i < consts; i++) {
				vBYTE type = (vBYTE)f.get();
				switch (type) {
				case vCT_UTF8:
				{
					ops[0].usi = readUSI(f);
					V<vUTF8BODY> str = VMAKE(vUTF8BODY, ctx);
					str(ctx)->len = ops[0].usi;
					str(ctx)->s = VMAKEARRAY(vBYTE, ctx, (size_t)str(ctx)->len + 1);
					memset(str(ctx)->s.real(ctx), 0, (size_t)str(ctx)->len + 1);
					for (vUSHORT k = 0; k < ops[0].usi; k++) str(ctx)->s( ctx,  k) = (vBYTE)f.get();
					vUTF8 wrap;
					wrap.r.a = (vULONG)str.v(ctx);
					_constPool(ctx)->set<vUTF8>(ctx, i, wrap);
					//printf("Read string at %d: %s\n", i, str(ctx)->s(ctx));
					break;
				}
				case vCT_STRING:
					ops[0].str.strIndex = readUSI(f);
					_constPool(ctx)->set<vSTRING>(ctx, i, ops[0].str);
					break;
				case vCT_CLASS:
					ops[0].cls.clsIndex = readUSI(f);
					_constPool(ctx)->set<vCLASS>(ctx, i, ops[0].cls);
					break;
				case vCT_METHODREF:
				case vCT_IFACEMETHODREF:
				case vCT_FIELDREF:
					ops[0].mr.clsIndex = readUSI(f);
					ops[0].mr.nameIndex = readUSI(f);
					_constPool(ctx)->set<vMETHODREF>(ctx, i, ops[0].mr);
					break;
				case vCT_NAMEANDTYPE:
					ops[0].nt.nameIndex = readUSI(f);
					ops[0].nt.descIndex = readUSI(f);
					_constPool(ctx)->set<vNAMEANDTYPE>(ctx, i, ops[0].nt);
					break;
				case vCT_METHODTYPE:
					ops[0].mt.descIndex = readUSI(f);
					_constPool(ctx)->set<vMETHODTYPE>(ctx, i, ops[0].mt);
					break;
				case vCT_INT:
					_constPool(ctx)->set<vINT>(ctx, i, readInt(f));
					break;
				case vCT_FLOAT:
					_constPool(ctx)->set<vFLOAT>(ctx, i, readFloat(f));
					break;
				case vCT_LONG:
					_constPool(ctx)->set<vLONG>(ctx, i, readLong(f));
					i++;
					break;
				case vCT_DOUBLE:
					_constPool(ctx)->set<vDOUBLE>(ctx, i, readDouble(f));
					i++;
					break;
				case vCT_INVOKEDYNAMIC:
					ops[0].idyn.bootstrapMethodAttrIndex = readUSI(f);
					ops[0].idyn.nameIndex = readUSI(f);
					_constPool(ctx)->set<vINVOKEDYNAMIC>(ctx, i, ops[0].idyn);
					break;
				case vCT_METHODHANDLE:
					ops[0].mh.kind = (vBYTE)f.get();
					ops[0].mh.index = readUSI(f);
					_constPool(ctx)->set<vMETHODHANDLE>(ctx, i, ops[0].mh);
					break;
				default:
					fprintf(stderr, "[vClass::ctor] Unhandled tag type: %d\n", type);
					break;
				}
			}

			// Parse class info
			_accessFlags = readUSI(f);
			_name = readUSI(f);
			_super = readUSI(f);
			
			ctx->getClasses()[getName(ctx)] = (vClass*)(((uintptr_t)this) - ctx->offset());

			vClass* super = 0;

			if (_super) {
				auto vsuper = cpu->lazyLoad(ctx, getSuperName(ctx), nesting + 2);
				if (vsuper)
				{
					super = vsuper(ctx);
				}
			}


			size_t fieldOffset = (size_t)(super ? super->_fieldCount : 0);
			size_t attributeOffset = (size_t)(super ? super->_attributeCount : 0);
			size_t methodOffset = 0;
			//printf("Field offset: %llu, method offset: %llu\n", fieldOffset, methodOffset);

			_fieldOffset = (vUSHORT)fieldOffset;
			_methodOffset = (vUSHORT)methodOffset;
			_attributeOffset = (vUSHORT)_attributeOffset;

			_interfaceCount = readUSI(f);
			_interfaces = VMAKEARRAY(vUSHORT, ctx, (size_t)_interfaceCount);

			for (vUSHORT i = 0; i < _interfaceCount; i++) {
				_interfaces(ctx, (size_t)i) = readUSI(f);
			}

			_fieldCount = readUSI(f);
			_fields = VMAKEARRAY(vFIELD, ctx, (size_t)_fieldCount + fieldOffset);

			for (vUSHORT i = 0; i < _fieldCount; i++) {
				readField(ctx, f, _fields(ctx, (size_t)i));
			}

			for (size_t i = _fieldCount; super && i < _fieldCount + fieldOffset; i++) {
				vFIELD& field = super->_fields( ctx,  i - _fieldCount);
				_fields(ctx, i) = field;
			}

			_methodCount = readUSI(f);
			_methods = VMAKEARRAY(vFIELD, ctx, (size_t)_methodCount);

			for (vUSHORT i = 0; i < _methodCount; i++) {
				readField(ctx, f, _methods(ctx, (size_t)i));
			}

			_attributeCount = readUSI(f);
			_attributes = VMAKEARRAY(vATTRIBUTE, ctx, (size_t)_attributeCount);
			for (vUSHORT i = 0; i < _attributeCount; i++) {
				readAttribute(ctx, f, _attributes(ctx, (size_t)i));
			}

			for (size_t i = _attributeCount; super && i < _attributeCount + attributeOffset; i++) {
				vATTRIBUTE& attr = super->_attributes(ctx, i - _attributeCount);
				_attributes(ctx, i) = attr;
			}

			_fieldCount += _fieldOffset;
			_attributeCount += _attributeOffset;
		}
	}

	vClass::~vClass() {

	}

	void vClass::destroy(vContext* ctx) {
		_constPool(ctx)->destroy(ctx);
		_constPool.release(ctx);
		//_fieldLookup.Release(ctx, true);
		for (vUSHORT i = 0; i < _fieldCount; i++) {
			_fields(ctx, (size_t)i).attributes.release(ctx, true);
		}
		for (vUSHORT i = 0; i < _methodCount; i++) {
			_methods(ctx, (size_t)i).attributes.release(ctx, true);
		}
		_fields.release(ctx, true);
		_methods.release(ctx, true);
		_attributes.release(ctx, true);
	}

	const char* vClass::getName(vContext* ctx) {
		return (const char*)toString(ctx, _name)(ctx)->s.real(ctx);
	}

	const char* vClass::getSuperName(vContext* ctx) {
		if (_super == 0) return 0;
		return (const char*)toString(ctx, _super)(ctx)->s.real(ctx);
	}

	vUSHORT vClass::readUSI(vBYTE* ip) const {
		vUSHORT usi = read<vBYTE>(ip);
		usi <<= 8;
		usi |= read<vBYTE>(ip + 1);
		return usi;
	}

	vUINT vClass::readUI(vBYTE* ip) const {
		vUINT ui = read<vBYTE>(ip);
		ui <<= 8; ui |= read<vBYTE>(ip + 1);
		ui <<= 8; ui |= read<vBYTE>(ip + 2);
		ui <<= 8; ui |= read<vBYTE>(ip + 3);
		return ui;
	}

	vDOUBLE vClass::readDouble(vBYTE* ip) const {
		vBYTE mirror[8];
		for (int i = 0; i < 8; i++) {
			mirror[7 - i] = ip[i];
		}
		return *(vDOUBLE*)mirror;
	}

	vFLOAT vClass::readFloat(vBYTE* ip) const {
		vBYTE mirror[4];
		for (int i = 0; i < 4; i++) {
			mirror[3 - i] = ip[i];
		}
		return *(vFLOAT*)mirror;
	}

	vLONG vClass::readLong(vBYTE* ip) const {
		vBYTE mirror[8];
		for (int i = 0; i < 8; i++) {
			mirror[7 - i] = ip[i];
		}
		return *(vLONG*)mirror;
	}

	vINT vClass::readInt(vBYTE* ip) const {
		vBYTE mirror[4];
		for (int i = 0; i < 4; i++) {
			mirror[3 - i] = ip[i];
		}
		return *(vINT*)mirror;
	}

	vUSHORT vClass::readUSI(std::ifstream& stream) const {
		vBYTE buff[2];
		stream.read((char*)buff, 2);
		return readUSI(buff);
	}

	vUINT vClass::readUI(std::ifstream& stream) const {
		vBYTE buff[4];
		stream.read((char*)buff, 4);
		return readUI(buff);
	}

	vINT vClass::readInt(std::ifstream& stream) const {
		vBYTE buff[4];
		stream.read((char*)buff, 4);
		return readInt(buff);
	}

	vDOUBLE vClass::readDouble(std::ifstream& stream) const {
		vBYTE buff[8];
		stream.read((char*)buff, 8);
		return readDouble(buff);
	}

	vFLOAT vClass::readFloat(std::ifstream& stream) const {
		vBYTE buff[4];
		stream.read((char*)buff, 4);
		return readFloat(buff);
	}

	vLONG vClass::readLong(std::ifstream& stream) const {
		vBYTE buff[8];
		stream.read((char*)buff, 8);
		vLONG lng = readLong(buff);
		return lng;
	}

	void vClass::readAttribute(vContext* ctx, std::ifstream& f, vATTRIBUTE& attr) {
		attr.name = readUSI(f);
		attr.length = readUI(f);
		attr.info = VMAKEARRAY(vBYTE, ctx, (size_t)attr.length + 1);
		attr.cls = (vClass*)((uintptr_t)this - ctx->offset());
		f.read((char*)attr.info.real(ctx), (size_t)attr.length);
	}

	void vClass::readField(vContext* ctx, std::ifstream& f, vFIELD& field) {
		field.access = readUSI(f);
		field.name = readUSI(f);
		field.desc = readUSI(f);
		field.cls = (vClass*)((uintptr_t)this - ctx->offset());
		field.attributeCount = readUSI(f);
		field.attributes = VMAKEARRAY(vATTRIBUTE, ctx, (size_t)field.attributeCount);
		//printf("%s read field %s\n", getName(ctx), toString(ctx, field.name)(ctx)->s(ctx));
		memset(&field.value, 0, sizeof(vCOMMON));
		for (vUSHORT i = 0; i < field.attributeCount; i++) {
			readAttribute(ctx, f, field.attributes(ctx, (size_t)i));
		}
	}

	vATTRIBUTE* vClass::getAttribute(vContext* ctx, const vFIELD* field, const char* name) {
		for (vUSHORT i = 0; i < field->attributeCount; i++) {
			V<vUTF8BODY> str = toString(ctx, field->attributes(ctx, (size_t)i).name);
			if (!str.isValid()) continue;
			if (!strcmp((const char*)str(ctx)->s.real(ctx), name)) {
				return &field->attributes(ctx, (size_t)i);
			}
		}
		return 0;
	}

	vFIELD* vClass::getField(vContext* ctx, const char* name) {
		for (vUSHORT i = 0; i < _fieldCount; i++) {
			vFIELD& field = _fields(ctx, (size_t)i);
			if ((field.access & 8) != 8) continue;	// static flag
			V<vClass>& cls = field.cls;// ((vClass*)field.cls);
			V<vUTF8BODY> str = cls(ctx)->toString(ctx, field.name);
			if (!str.isValid()) continue;
			if (!strcmp((const char*)str(ctx)->s.real(ctx), name)) {
				return &_fields(ctx, (size_t)i);
			}
		}
		if (_super) {
			auto& classes = ctx->getClasses();
			auto superClass = classes.find(getSuperName(ctx));
			if (superClass != classes.end()) {
				return V<vClass>(superClass->second)(ctx)->getField(ctx, name);
			}
		}
		return 0;
	}

	vMETHOD* vClass::getMethod(vContext* ctx, const char* name, const char* desc) {
		for (vUSHORT i = 0; i < _methodCount; i++) {
			V<vUTF8BODY> str = toString(ctx, _methods(ctx, i).name);
			if (!str.isValid()) continue;
			if (!strcmp((const char*)str(ctx)->s.real(ctx), name)) {
				if (desc != 0) {
					V<vUTF8BODY> descstr = toString(ctx, _methods(ctx, (size_t)i).desc);
					if (!descstr.isValid()) continue;
					if (strcmp(desc, (const char*)descstr(ctx)->s.real(ctx))) continue;
				}
				return &_methods(ctx, (size_t)i);
			}
		}
		return 0;
	}

	vOBJECTREF& vClass::getJavaClass(vContext* ctx,  bool gc) {
		std::string saneName = getName(ctx);
		if (saneName.back() == ';') {
			saneName.pop_back();
			saneName = saneName.substr(1);
		}
		std::wstring wName;
		for (char& c : saneName) {
			wName += c == '/' ? L'.' : (wchar_t)c;
			if (c == '.') c = '/';
		}

		return ctx->getObject<vOBJECTREF>(ctx, "ldc:java/lang/Class:" + saneName, [this, wName, gc](vContext* ctx) -> vOBJECTREF {
			auto it = ctx->getClasses().find("java/lang/Class");
			if (it == ctx->getClasses().end()) throw std::runtime_error("java/lang/Class not loaded");
			V<vClass> klassClass(it->second);
			JObject wrap(ctx, klassClass, gc);
			wrap["name"].set(JString(ctx, wName, gc).ref());
			wrap.x().set<vLONG>(1060);
			return wrap.ref();
		});
	}


	std::string vClass::toStdString(vContext* ctx, vUSHORT index, int selector) const {
		return toCString(ctx, index, selector);
	}

	const char* vClass::toCString(vContext* ctx, vUSHORT index, int selector) const {
		return (const char*)toString(ctx, index, selector)(ctx)->s(ctx);
	}

	V<vUTF8BODY> vClass::toString(vContext* ctx, vUSHORT index, int selector) const {
		vCOMMON str = _constPool(ctx)->get<vCOMMON>(ctx, index);
		// printf("to string %d, type: %d\n", index, str.type);
		if (str.type == vTypes::type<vCLASS>()) {
			return toString(ctx, str.cls.clsIndex);
		}
		else if (str.type == vTypes::type<vNAMEANDTYPE>()) {
			return toString(ctx, selector == 0 ? str.nt.nameIndex : str.nt.descIndex);
		}
		else if (str.type == vTypes::type<vSTRING>()) {
			return toString(ctx, str.str.strIndex);
		}
		else if (str.type == vTypes::type<vUTF8>()) {
			return V<vUTF8BODY>((vUTF8BODY*)str.utf8.r.a);
		}
		return V<vUTF8BODY>::nullPtr();
	}

	CodeAttribute vClass::getCode(vContext* ctx, vMETHOD* method) {
		CodeAttribute ret;
		ret.codeLength = V<vBYTE>::nullPtr();
		if (!method) return ret;
		vATTRIBUTE* attrib = getAttribute(ctx, method, "Code");
		if (!attrib) return ret;
		if (attrib->length == 0) ret;
		vBYTE* data = attrib->info(ctx);
		ret.attributeLength = readUI(data);
		ret.maxStack = readUSI(data);
		ret.maxLocals = readUSI(data + 2);
		ret.codeLength = readUI(data + 4);
		ret.code = attrib->info + (size_t)8;
		ret.exceptionTableLength = readUSI(data + 8 + ret.codeLength);
		ret.exceptionTable = (ExceptionTable*)(attrib->info.v() + (size_t)(10 + ret.codeLength));
		return ret;
	}

	vUINT vClass::argsCount(vContext* ctx, vMETHOD* method) {
		if (!method) return 0;
		V<vUTF8BODY> desc = toString(ctx, method->desc);
		if (!desc.isValid()) return 0;
		const char* str = (const char*)desc(ctx)->s.real(ctx);
		return argsCount(str);
	}

	vUINT vClass::argsCount(const char* str) {
		vUINT count = 0;
		bool className = false;
		while (*str != ')') {
			char c = *str;
			if (c == '[') {
				str++;
				continue;
			}
			if (c != '(') {
				if (!className && c == 'L') {
					className = true;
					count++;
				}
				else if (className && c == ';') {
					className = false;
				}
				else if (!className) {
					count++;
				}
			}
			str++;
		}
		return count;
	}

	vCOMMON* vClass::getObjField(vContext* ctx, V<vOBJECT> obj, const char* name) {
		//printf("This: %p\n", this);
		for (vUSHORT k = 0, i = 0; k < _fieldCount; k++) {
			auto& field = _fields(ctx, (size_t)k);
			if (field.access & 8) continue; // static field
			//printf("Xdd %p\n", &field);
			V<vClass>& vcls = field.cls; // ((vClass*)field.cls);
			if (!vcls.isValid()) return 0;
			vClass* cls = (vcls)(ctx);
			if (cls->TAG != JAETHER_CLASS_TAG) throw std::runtime_error("invalid class tag: " + std::to_string(cls->TAG));
			const char* fieldName = (const char*)cls->toString(ctx, field.name)(ctx)->s.real(ctx);
			if (!strcmp(fieldName, name)) {
				return &obj(ctx)->fields()(ctx, i);
			}
			i++;
		}
		return 0;
	}

	vCOMMON* vClass::getObjField(vContext* ctx, V<vOBJECTREF> objref, const char* name) {
		return getObjField(ctx, V<vOBJECT>((vOBJECT*)objref(ctx)->r.a), name);
	}


	std::tuple<MethodResolveStatus, vCOMMON> vClass::invoke(
		vContext* ctx,
		V<vClass> self,
		V<vClass> super,
		vCPU* cpu,
		vStack* _stack,
		vBYTE opcode,
		const std::string& methodName,
		const std::string& desc,
		int nesting) {

		auto [exists, nnFrame] = createFrame(ctx, self, super, cpu, _stack, opcode, methodName, desc, nesting);
		if (exists == MethodResolveStatus::eMRS_NotFound) {
			vCOMMON empty;
			memset(&empty, 0, sizeof(vCOMMON));
			throw std::runtime_error("couldnt create frame");
			return std::make_tuple(MethodResolveStatus::eMRS_Found, empty);
		} else if (exists == MethodResolveStatus::eMRS_Native) {
			return std::make_tuple(MethodResolveStatus::eMRS_Native, vCOMMON{});
		}

		V<vFrame> nFrame((vFrame*)nnFrame);
		cpu->run(ctx, nFrame, nesting);
		vCOMMON subret;
		memset(&subret, 0, sizeof(subret));
		if (nFrame(ctx)->_returns) {
			subret = nFrame(ctx)->_stack(ctx)->pop<vCOMMON>(ctx);
			_stack->push<vCOMMON>(ctx, subret);
		}
		nFrame(ctx)->destroy(ctx);
		nFrame.release(ctx);
		return std::make_tuple(MethodResolveStatus::eMRS_Found, subret);
	}


	std::tuple<MethodResolveStatus, vFrame*> vClass::createFrame(
		vContext* ctx,
		V<vClass> self,
		V<vClass> super,
		vCPU* cpu,
		vStack* _stack,
		vBYTE opcode,
		const std::string& methodName,
		const std::string& desc,
		int nesting) {
		vMETHOD* method = getMethod(ctx, methodName.c_str(), desc.c_str());
		if (method) {
			std::string npath = std::string(getName(ctx)) + "/" + method->getName(ctx) + ":" + method->getDesc(ctx);
			auto nit = cpu->findNative(npath);
			if (nit != cpu->getNatives().end()) {
				auto cb = nit->second;
				cb(ctx, cpu, _stack, opcode);
				return std::make_tuple(MethodResolveStatus::eMRS_Native, (vFrame*)0);
			}
			V<vFrame> nFrame = VMAKE(vFrame, ctx, ctx, method, self);
			vUINT args = argsCount(ctx, method);
			std::vector<vCOMMON> vargs;
			for (vUINT i = 0; i < args; i++) {
				vargs.push_back(_stack->pop<vCOMMON>(ctx));
			}
			// longs and double take two slots, therefore it needs special handling
			for (vUINT i = 0, j = (opcode == invokestatic ? 0 : 1); i < (vUINT)vargs.size(); i++, j++) {
				vCOMMON& val = vargs[vargs.size() - i - 1];
				bool isCat2 = val.type == vTypes::type<vLONG>() || val.type == vTypes::type<vDOUBLE>();
				nFrame(ctx)->_local(ctx)->set<vCOMMON>(ctx, (size_t)j, val);
				if (isCat2) j++;
			}
			if (opcode != invokestatic)
				nFrame(ctx)
				->_local(ctx)
				->set<vCOMMON>(ctx, 0, _stack->pop<vCOMMON>(ctx));
			return std::make_tuple(MethodResolveStatus::eMRS_Found, nFrame.v(ctx));
		} else if (super.isValid()) {
			if (_super) {
				auto& classes = ctx->getClasses();
				auto it = classes.find(getSuperName(ctx));
				if (it != classes.end()) {
					V<vClass> kls = it->second;
					if (kls.isValid())
						return kls(ctx)->createFrame(ctx, kls, super, cpu, _stack, opcode, methodName, desc, nesting + 1);
				}
			}
			if (super.isValid())
				return super(ctx)->createFrame(ctx, super, V<vClass>::nullPtr(), cpu, _stack, opcode, methodName, desc, nesting + 1);
		}
		return std::make_tuple(MethodResolveStatus::eMRS_NotFound, V<vFrame>::nullPtr().v(ctx));
	}

	bool vClass::instanceOf(vContext* ctx, V<vClass> cls) {
		vClass* sThis = cls(ctx);
		if (sThis == this) return true;
		if (!_super) return false;
		if (_super) {
			auto& classes = ctx->getClasses();
			auto it = classes.find(getSuperName(ctx));
			if (it != classes.end()) {
				V<vClass> kls = it->second;
				return kls(ctx)->instanceOf(ctx, cls);
			}
		}
		return false;
	}

}