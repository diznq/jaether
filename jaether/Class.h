#pragma once
#include "Types.h"
#include "Pointer.h"
#include "Memory.h"
#include <fstream>

class vClass {
public:
	V<vMemory> _constPool;
	V<vFIELD> _methods;
	V<vFIELD> _fields;
	V<vATTRIBUTE> _attributes;
	V<vUSHORT> _interfaces;
	vUSHORT _name = 0;
	vUSHORT _super = 0;
	vUSHORT _accessFlags = 0;
	vUSHORT _fieldCount = 0;
	vUSHORT _methodCount = 0;
	vUSHORT _interfaceCount = 0;
	vUSHORT _attributeCount = 0;

	vClass(const char* name);
	~vClass();

	const char* getName();
	const char* getSuperName();

	template<class T> T read(vBYTE* ip) const {
		return *(T*)ip;
	}

	vUSHORT		readUSI(vBYTE* ip) const;
	vUINT		readUI(vBYTE* ip) const;
	vDOUBLE		readDouble(vBYTE* ip) const;
	vUSHORT		readUSI(std::ifstream& stream) const;
	vUINT		readUI(std::ifstream& stream) const;
	vDOUBLE		readDouble(std::ifstream& stream) const;
	void		readAttribute(std::ifstream& f, vATTRIBUTE& attr);
	void		readField(std::ifstream& f, vFIELD& field);
	vATTRIBUTE* getAttribute(const vFIELD* field, const char* name);
	vFIELD*		getField(const char* name);
	vMETHOD*	getMethod(const char* name, const char* desc = 0);

	V<vUTF8BODY> toString(vUSHORT index, int selector = 0) const;
	V<vBYTE>	getCode(vMETHOD* method);
	vUINT		argsCount(vMETHOD* method);
};