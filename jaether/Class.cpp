#include "Class.h"
#include "Frame.h"
#include "CPU.h"

namespace jaether {

	vClass::vClass(vContext* ctx, vCPU* cpu, const char* name) {
		std::ifstream f(name, std::ios::binary);
		if (f) {
			vUINT magic = readUI(f);
			vUSHORT minor = readUSI(f), major = readUSI(f);
			vUSHORT consts = readUSI(f);
			vCOMMON ops[8];
			printf("Loading class %s, ver: %d.%d, magic: %08X, constants: %d\n", name, major, minor, magic, consts);
			_constPool = VMAKE(vMemory, ctx, ctx, (size_t)consts);
			_fieldLookup = VMAKEARRAY(vUSHORT, ctx, (size_t)consts);
			memset(_fieldLookup.Real(ctx), 0xFF, consts * sizeof(vUSHORT));
			// Parse const pool
			for (vUSHORT i = 1; i < consts; i++) {
				vBYTE type = (vBYTE)f.get();
				switch (type) {
				case vCT_UTF8:
				{
					ops[0].usi = readUSI(f);
					V<vUTF8BODY> str = VMAKE(vUTF8BODY, ctx);
					str.Ptr(ctx)->len = ops[0].usi;
					str.Ptr(ctx)->s = VMAKEARRAY(vBYTE, ctx, (size_t)str.Ptr(ctx)->len + 1);
					memset(str.Ptr(ctx)->s.Real(ctx), 0, (size_t)str.Ptr(ctx)->len + 1);
					for (vUSHORT k = 0; k < ops[0].usi; k++) str.Ptr(ctx)->s[VCtxIdx{ ctx, k }] = (vBYTE)f.get();
					vUTF8 wrap;
					wrap.r.a = (vULONG)str.Virtual(ctx);
					_constPool.Ptr(ctx)->set<vUTF8>(ctx, i, wrap);
					break;
				}
				case vCT_STRING:
					ops[0].str.strIndex = readUSI(f);
					_constPool.Ptr(ctx)->set<vSTRING>(ctx, i, ops[0].str);
					break;
				case vCT_CLASS:
					ops[0].cls.clsIndex = readUSI(f);
					_constPool.Ptr(ctx)->set<vCLASS>(ctx, i, ops[0].cls);
					break;
				case vCT_METHODREF:
				case vCT_IFACEMETHODREF:
				case vCT_FIELDREF:
					ops[0].mr.clsIndex = readUSI(f);
					ops[0].mr.nameIndex = readUSI(f);
					_constPool.Ptr(ctx)->set<vMETHODREF>(ctx, i, ops[0].mr);
					break;
				case vCT_NAMEANDTYPE:
					ops[0].nt.nameIndex = readUSI(f);
					ops[0].nt.descIndex = readUSI(f);
					_constPool.Ptr(ctx)->set<vNAMEANDTYPE>(ctx, i, ops[0].nt);
					break;
				case vCT_METHODTYPE:
					ops[0].mt.descIndex = readUSI(f);
					_constPool.Ptr(ctx)->set<vMETHODTYPE>(ctx, i, ops[0].mt);
					break;
				case vCT_INT:
					_constPool.Ptr(ctx)->set<vUINT>(ctx, i, readUI(f));
					break;
				case vCT_FLOAT:
					_constPool.Ptr(ctx)->set<vFLOAT>(ctx, i, readFloat(f));
					break;
				case vCT_LONG:
					_constPool.Ptr(ctx)->set<vULONG>(ctx, i, readLong(f));
					i++;
					break;
				case vCT_DOUBLE:
					_constPool.Ptr(ctx)->set<vDOUBLE>(ctx, i, readDouble(f));
					i++;
					break;
				case vCT_INVOKEDYNAMIC:
					ops[0].idyn.bootstrapMethodAttrIndex = readUSI(f);
					ops[0].idyn.nameIndex = readUSI(f);
					_constPool.Ptr(ctx)->set<vINVOKEDYNAMIC>(ctx, i, ops[0].idyn);
					break;
				case vCT_METHODHANDLE:
					ops[0].mh.kind = (vBYTE)f.get();
					ops[0].mh.index = readUSI(f);
					_constPool.Ptr(ctx)->set<vMETHODHANDLE>(ctx, i, ops[0].mh);
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

			vClass* super = 0;

			if (_super) {
				auto super_it = cpu->lazyLoad(ctx, getSuperName(ctx));
				if (super_it != ctx->GetClasses().end())
				{
					V<vClass> vsuper = super_it->second;
					super = vsuper.Ptr(ctx);
				}
			}


			size_t fieldOffset = 0; // (size_t)(super ? super->_fieldCount : 0);
			size_t methodOffset = 0; // (size_t)(super ? super->_methodCount : 0);
			//printf("Field offset: %llu, method offset: %llu\n", fieldOffset, methodOffset);

			_fieldOffset = (vUSHORT)fieldOffset;
			_methodOffset = (vUSHORT)methodOffset;

			_interfaceCount = readUSI(f);
			_interfaces = VMAKEARRAY(vUSHORT, ctx, (size_t)_interfaceCount);

			for (vUSHORT i = 0; i < _interfaceCount; i++) {
				_interfaces[VCtxIdx{ ctx, (size_t)i }] = readUSI(f);
			}

			_fieldCount = readUSI(f);
			_fields = VMAKEARRAY(vFIELD, ctx, (size_t)_fieldCount + fieldOffset);

			for (vUSHORT i = 0; i < _fieldCount; i++) {
				readField(ctx, f, _fields[VCtxIdx{ ctx, (size_t)i }]);
			}
			for (size_t i = _fieldCount; super && i < _fieldCount + fieldOffset; i++) {
				_fields[VCtxIdx{ ctx, i + (size_t)_fieldCount }] = super->_fields[VCtxIdx{ ctx, i - _fieldCount }];
			}

			_methodCount = readUSI(f);
			_methods = VMAKEARRAY(vFIELD, ctx, (size_t)_methodCount + methodOffset);

			for (vUSHORT i = 0; i < _methodCount; i++) {
				readField(ctx, f, _methods[VCtxIdx{ ctx, (size_t)i }]);
			}
			for (size_t i = 0; super && i < methodOffset; i++) {
				_methods[VCtxIdx{ ctx, i + (size_t)_methodCount }] = super->_methods[VCtxIdx{ ctx, i - _methodCount }];
			}

			_attributeCount = readUSI(f);
			_attributes = VMAKEARRAY(vATTRIBUTE, ctx, (size_t)_attributeCount);
			for (vUSHORT i = 0; i < _attributeCount; i++) {
				readAttribute(ctx, f, _attributes[VCtxIdx{ ctx, (size_t)i }]);
			}

			for (vUSHORT i = 0; i < consts; i++) {
				vCOMMON item = _constPool.Ptr(ctx)->get<vCOMMON>(ctx, i);
				if (item.type == vTypes::type<vMETHODREF>()) {
					bool found = false;
					for (vUSHORT i = 0; !found && i < _fieldCount + _fieldOffset; i++) {
						if (
							!strcmp(
								(const char*)toString(ctx, _fields[VCtxIdx{ ctx, (size_t)i }].name).Ptr(ctx)->s.Real(ctx),
								(const char*)toString(ctx, item.mr.nameIndex).Ptr(ctx)->s.Real(ctx)
							)
							) {
							_fieldLookup[VCtxIdx{ ctx, (size_t)item.mr.nameIndex }] = i;
							found = true;
							break;
						}
					}
				}
			}

		}
	}

	vClass::~vClass() {

	}

	void vClass::destroy(vContext* ctx) {
		_constPool.Ptr(ctx)->destroy(ctx);
		_constPool.Release(ctx);
		_fieldLookup.Release(ctx, true);
		for (vUSHORT i = 0; i < _fieldCount; i++) {
			_fields[VCtxIdx{ ctx, (size_t)i }].attributes.Release(ctx, true);
		}
		for (vUSHORT i = 0; i < _methodCount; i++) {
			_methods[VCtxIdx{ ctx, (size_t)i }].attributes.Release(ctx, true);
		}
		_fields.Release(ctx, true);
		_methods.Release(ctx, true);
		_attributes.Release(ctx, true);
	}

	const char* vClass::getName(vContext* ctx) {
		return (const char*)toString(ctx, _name).Ptr(ctx)->s.Real(ctx);
	}

	const char* vClass::getSuperName(vContext* ctx) {
		if (_super == 0) return 0;
		return (const char*)toString(ctx, _super).Ptr(ctx)->s.Real(ctx);
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

	vULONG vClass::readLong(vBYTE* ip) const {
		vBYTE mirror[8];
		for (int i = 0; i < 8; i++) {
			mirror[7 - i] = ip[i];
		}
		return *(vLONG*)mirror;
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

	vULONG vClass::readLong(std::ifstream& stream) const {
		vBYTE buff[8];
		stream.read((char*)buff, 8);
		vULONG lng = readLong(buff);
		return lng;
	}

	void vClass::readAttribute(vContext* ctx, std::ifstream& f, vATTRIBUTE& attr) {
		attr.name = readUSI(f);
		attr.length = readUI(f);
		attr.info = VMAKEARRAY(vBYTE, ctx, (size_t)attr.length + 1);
		f.read((char*)attr.info.Real(ctx), (size_t)attr.length);
	}

	void vClass::readField(vContext* ctx, std::ifstream& f, vFIELD& field) {
		field.access = readUSI(f);
		field.name = readUSI(f);
		field.desc = readUSI(f);
		field.attributeCount = readUSI(f);
		field.attributes = VMAKEARRAY(vATTRIBUTE, ctx, (size_t)field.attributeCount);
		memset(&field.value, 0, sizeof(vCOMMON));
		for (vUSHORT i = 0; i < field.attributeCount; i++) {
			readAttribute(ctx, f, field.attributes[VCtxIdx{ ctx, (size_t)i }]);
		}
	}

	vATTRIBUTE* vClass::getAttribute(vContext* ctx, const vFIELD* field, const char* name) {
		for (vUSHORT i = 0; i < field->attributeCount; i++) {
			V<vUTF8BODY> str = toString(ctx, field->attributes[VCtxIdx{ ctx, (size_t)i }].name);
			if (!str.IsValid()) continue;
			if (!strcmp((const char*)str.Ptr(ctx)->s.Real(ctx), name)) {
				return &field->attributes[VCtxIdx{ ctx, (size_t)i }];
			}
		}
		return 0;
	}

	vFIELD* vClass::getField(vContext* ctx, const char* name) {
		for (vUSHORT i = 0; i < _fieldCount; i++) {
			V<vUTF8BODY> str = toString(ctx, _fields[VCtxIdx{ ctx, (size_t)i }].name);
			if (!str.IsValid()) continue;
			if (!strcmp((const char*)str.Ptr(ctx)->s.Real(ctx), name)) {
				return &_fields[VCtxIdx{ ctx, (size_t)i }];
			}
		}
		return 0;
	}

	vMETHOD* vClass::getMethod(vContext* ctx, const char* name, const char* desc) {
		for (vUSHORT i = 0; i < _methodCount; i++) {
			V<vUTF8BODY> str = toString(ctx, _methods[VCtxIdx{ ctx, i }].name);
			if (!str.IsValid()) continue;
			if (!strcmp((const char*)str.Ptr(ctx)->s.Real(ctx), name)) {
				if (desc != 0) {
					V<vUTF8BODY> descstr = toString(ctx, _methods[VCtxIdx{ ctx, (size_t)i }].desc);
					if (!descstr.IsValid()) continue;
					if (strcmp(desc, (const char*)descstr.Ptr(ctx)->s.Real(ctx))) continue;
				}
				return &_methods[VCtxIdx{ ctx, (size_t)i }];
			}
		}
		return 0;
	}

	V<vUTF8BODY> vClass::toString(vContext* ctx, vUSHORT index, int selector) const {
		vCOMMON str = _constPool.Ptr(ctx)->get<vCOMMON>(ctx, index);
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
		return V<vUTF8BODY>::NullPtr();
	}

	V<vBYTE> vClass::getCode(vContext* ctx, vMETHOD* method) {
		if (!method) return V<vBYTE>::NullPtr();
		vATTRIBUTE* attrib = getAttribute(ctx, method, "Code");
		if (!attrib) return V<vBYTE>::NullPtr();
		if (attrib->length == 0) return V<vBYTE>::NullPtr();
		return attrib->info + (size_t)8;
	}

	vUINT vClass::argsCount(vContext* ctx, vMETHOD* method) {
		if (!method) return 0;
		V<vUTF8BODY> desc = toString(ctx, method->desc);
		if (!desc.IsValid()) return 0;
		const char* str = (const char*)desc.Ptr(ctx)->s.Real(ctx);
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
		for (vUSHORT i = 0; i < _fieldCount; i++) {
			const char* fieldName = (const char*)toString(ctx, _fields[VCtxIdx{ ctx, (size_t)i }].name).Ptr(ctx)->s.Real(ctx);
			if (!strcmp(fieldName, name)) {
				return &obj.Ptr(ctx)->fields[VCtxIdx{ ctx, i }];
			}
		}
		if (_super) {
			auto& _classes = ctx->GetClasses();
			auto it = _classes.find(getSuperName(ctx));
			if (it != _classes.end()) {
				V<vClass> super = it->second;
				return super.Ptr(ctx)->getObjField(ctx, obj, name);
			}
		}
		return 0;
	}

	vCOMMON* vClass::getObjField(vContext* ctx, V<vOBJECT> obj, vUSHORT idx) {
		vUSHORT realIndex = _fieldLookup[VCtxIdx{ ctx, idx }];
		if (realIndex == 0xFFFF) {
			return getObjField(ctx, obj, (const char*)toString(ctx, idx).Ptr(ctx)->s.Ptr(ctx));
		}
		return &obj.Ptr(ctx)->fields[VCtxIdx{ ctx, realIndex }];
	}

	vCOMMON* vClass::getObjField(vContext* ctx, V<vOBJECTREF> objref, const char* name) {
		return getObjField(ctx, V<vOBJECT>((vOBJECT*)objref.Ptr(ctx)->r.a), name);
	}

	vCOMMON* vClass::getObjField(vContext* ctx, V<vOBJECTREF> objref, vUSHORT idx) {
		return getObjField(ctx, V<vOBJECT>((vOBJECT*)objref.Ptr(ctx)->r.a), idx);
	}


	std::tuple<bool, vCOMMON> vClass::invoke(
		vContext* ctx,
		V<vClass> self,
		V<vClass> super,
		vCPU* cpu,
		vStack* _stack,
		vBYTE opcode,
		const std::string& methodName,
		const std::string& desc) {
		vMETHOD* method = getMethod(ctx, methodName.c_str(), desc.c_str());
		if (method) {
			V<vFrame> nFrame = VMAKE(vFrame, ctx, ctx, method, self);
			vUINT args = argsCount(ctx, method);
			for (vUINT i = 0; i < args; i++) {
				vUINT j = args - i;
				if (opcode == invokestatic) j--;
				nFrame.Ptr(ctx)->_local.Ptr(ctx)->set<vCOMMON>(ctx, (size_t)j, _stack->pop<vCOMMON>(ctx));
			}
			if (opcode != invokestatic) 
				nFrame.Ptr(ctx)
					->_local.Ptr(ctx)
					->set<vCOMMON>(ctx, 0, _stack->pop<vCOMMON>(ctx));
			cpu->run(ctx, nFrame);
			vCOMMON subret;
			memset(&subret, 0, sizeof(subret));
			if (nFrame.Ptr(ctx)->_returns) {
				subret = nFrame.Ptr(ctx)->_stack.Ptr(ctx)->pop<vCOMMON>(ctx);
				_stack->push<vCOMMON>(ctx, subret);
			}
			nFrame.Ptr(ctx)->destroy(ctx);
			nFrame.Release(ctx);
			return std::make_tuple(true, subret);
		} else if(super.IsValid()) {
			return super.Ptr(ctx)->invoke(ctx, super, V<vClass>::NullPtr(), cpu, _stack, opcode, methodName, desc);
		}
		vCOMMON empty;
		memset(&empty, 0, sizeof(vCOMMON));
		return std::make_tuple(false, empty);
	}

}