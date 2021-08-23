#include "Class.h"
#include "Frame.h"
#include "CPU.h"

vClass::vClass(const char* name) {
	std::ifstream f(name, std::ios::binary);
	if (f) {
		vUINT magic = readUI(f);
		vUSHORT minor = readUSI(f), major = readUSI(f);
		vUSHORT consts = readUSI(f);
		vCOMMON ops[8];
		_constPool = VMAKE(vMemory, (size_t)consts);
		// Parse const pool
		for (vUSHORT i = 1; i < consts; i++) {
			vBYTE type = (vBYTE)f.get();
			switch (type) {
			case vCT_UTF8:
			{
				ops[0].usi = readUSI(f);
				V<vUTF8BODY> str = VMAKE(vUTF8BODY);
				str->len = ops[0].usi;
				str->s = VMAKEARRAY(vBYTE, (size_t)str->len + 1);
				memset(str->s.Real(), 0, (size_t)str->len + 1);
				for (vUSHORT i = 0; i < ops[0].usi; i++) str->s[i] = (vBYTE)f.get();
				vUTF8 wrap;
				wrap.r.a = (vULONG)str.Virtual();
				_constPool->set<vUTF8>(i, wrap);
				break;
			}
			case vCT_STRING:
				ops[0].str.strIndex = readUSI(f);
				_constPool->set<vSTRING>(i, ops[0].str);
				break;
			case vCT_CLASS:
				ops[0].cls.clsIndex = readUSI(f);
				_constPool->set<vCLASS>(i, ops[0].cls);
				break;
			case vCT_METHODREF:
			case vCT_FIELDREF:
				ops[0].mr.clsIndex = readUSI(f);
				ops[0].mr.nameIndex = readUSI(f);
				_constPool->set<vMETHODREF>(i, ops[0].mr);
				break;
			case vCT_NAMEANDTYPE:
				ops[0].nt.nameIndex = readUSI(f);
				ops[0].nt.descIndex = readUSI(f);
				_constPool->set<vNAMEANDTYPE>(i, ops[0].nt);
				break;
			case vCT_INT:
				_constPool->set<vUINT>(i, readUI(f));
				break;
			case vCT_DOUBLE:
				_constPool->set<vDOUBLE>(i, readDouble(f));
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

		_interfaceCount = readUSI(f);
		_interfaces = VMAKEARRAY(vUSHORT, (size_t)_interfaceCount);

		for (vUSHORT i = 0; i < _interfaceCount; i++) {
			_interfaces[i] = readUSI(f);
		}

		_fieldCount = readUSI(f);
		_fields = VMAKEARRAY(vFIELD, (size_t)_fieldCount);
		for (vUSHORT i = 0; i < _fieldCount; i++) {
			readField(f, _fields[i]);
		}

		_methodCount = readUSI(f);
		_methods = VMAKEARRAY(vFIELD, (size_t)_methodCount);
		for (vUSHORT i = 0; i < _methodCount; i++) {
			readField(f, _methods[i]);
		}

		_attributeCount = readUSI(f);
		_attributes = VMAKEARRAY(vATTRIBUTE, (size_t)_attributeCount);
		for (vUSHORT i = 0; i < _attributeCount; i++) {
			readAttribute(f, _attributes[i]);
		}

	}
}

vClass::~vClass() {
	_constPool.Release();
}

const char* vClass::getName() {
	return (const char*)toString(_name)->s.Real();
}

const char* vClass::getSuperName() {
	if (_super == 0) return 0;
	return (const char*)toString(_super)->s.Real();
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

void vClass::readAttribute(std::ifstream& f, vATTRIBUTE& attr) {
	attr.name = readUSI(f);
	attr.length = readUI(f);
	attr.info = VMAKEARRAY(vBYTE, (size_t)attr.length + 1);
	f.read((char*)attr.info.Real(), (size_t)attr.length);
}

void vClass::readField(std::ifstream& f, vFIELD& field) {
	field.access = readUSI(f);
	field.name = readUSI(f);
	field.desc = readUSI(f);
	field.attributeCount = readUSI(f);
	field.attributes = VMAKEARRAY(vATTRIBUTE, (size_t)field.attributeCount);
	for (vUSHORT i = 0; i < field.attributeCount; i++) {
		readAttribute(f, field.attributes[i]);
	}
}

vATTRIBUTE* vClass::getAttribute(const vFIELD* field, const char* name) {
	for (vUSHORT i = 0; i < field->attributeCount; i++) {
		V<vUTF8BODY> str = toString(field->attributes[i].name);
		if (!str.IsValid()) continue;
		if (!strcmp((const char*)str->s.Real(), name)) {
			return &field->attributes[i];
		}
	}
	return 0;
}

vFIELD* vClass::getField(const char* name) {
	for (vUSHORT i = 0; i < _fieldCount; i++) {
		V<vUTF8BODY> str = toString(_fields[i].name);
		if (!str.IsValid()) continue;
		if (!strcmp((const char*)str->s.Real(), name)) {
			return &_fields[i];
		}
	}
	return 0;
}

vMETHOD* vClass::getMethod(const char* name, const char* desc) {
	for (vUSHORT i = 0; i < _methodCount; i++) {
		V<vUTF8BODY> str = toString(_methods[i].name);
		if (!str.IsValid()) continue;
		if (!strcmp((const char*)str->s.Real(), name)) {
			if (desc != 0) {
				V<vUTF8BODY> descstr = toString(_methods[i].desc);
				if (!descstr.IsValid()) continue;
				if (strcmp(desc, (const char*)descstr->s.Real())) continue;
			}
			return &_methods[i];
		}
	}
	return 0;
}

V<vUTF8BODY> vClass::toString(vUSHORT index, int selector) const {
	vCOMMON str = _constPool->get<vCOMMON>(index);
	if (str.type == vTypes::type<vCLASS>()) {
		return toString(str.cls.clsIndex);
	} else if (str.type == vTypes::type<vNAMEANDTYPE>()) {
		return toString(selector == 0 ? str.nt.nameIndex : str.nt.descIndex);
	} else if (str.type == vTypes::type<vSTRING>()) {
		return toString(str.str.strIndex);
	} else if (str.type == vTypes::type<vUTF8>()) {
		return V<vUTF8BODY>((vUTF8BODY*)str.utf8.r.a);
	}
	return V<vUTF8BODY>::NullPtr();
}

V<vBYTE> vClass::getCode(vMETHOD* method) {
	if (!method) return V<vBYTE>::NullPtr();
	vATTRIBUTE* attrib = getAttribute(method, "Code");
	if (!attrib) return V<vBYTE>::NullPtr();
	return attrib->info;
}

vUINT vClass::argsCount(vMETHOD* method) {
	if (!method) return 0;
	V<vUTF8BODY> desc = toString(method->desc);
	if (!desc.IsValid()) return 0;
	const char* str = (const char*)desc->s.Real();
	vUINT count = 0;
	bool className = false;
	while (*str != ')') {
		char c = *str;
		if (c != '(') {
			if (!className && c == 'L') {
				className = true;
				count++;
			} else if (className && c == ';') {
				className = false;
			} else if(!className) {
				count++;
			}
		}
		str++;
	}
	return count;
}

std::tuple<bool, vCOMMON> vClass::invoke(
	V<vClass> self,
	vCPU* cpu,
	vStack* _stack,
	vBYTE opcode,
	const std::string& methodName,
	const std::string& desc) {
	vMETHOD* method = getMethod(methodName.c_str(), desc.c_str());
	if (method) {
		V<vFrame> nFrame = VMAKE(vFrame, method, self);
		vUINT args = argsCount(method);
		for (vUINT i = 0; i < args; i++) {
			vUINT j = args - i;
			if (opcode == invokestatic) j--;
			nFrame->_local->set<vCOMMON>((size_t)j, _stack->pop<vCOMMON>());
		}
		if (opcode != invokestatic) nFrame->_local->set<vCOMMON>(0, _stack->pop<vCOMMON>());
		cpu->run(nFrame);
		vCOMMON subret;
		memset(&subret, 0, sizeof(subret));
		if (nFrame->_returns) {
			subret = nFrame->_stack->pop<vCOMMON>();
			_stack->push<vCOMMON>(subret);
		}
		nFrame.Release();
		return std::make_tuple(true, subret);
	}
	return std::make_tuple(false, vCOMMON{});
}