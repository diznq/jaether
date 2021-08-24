#pragma once
#include "Types.h"
#include "Pointer.h"
#include "Stack.h"
#include "Memory.h"
#include <map>
#include <functional>
#include <fstream>
#include <tuple>

class vCPU;
class vClass;

typedef std::function<void(const std::string& className, vCPU* cpu, vStack* stack, vBYTE opcode)> vNATIVE;

class vClass {
public:
	V<vMemory> _constPool = V<vMemory>::NullPtr();
	V<vFIELD> _methods = V<vFIELD>::NullPtr();
	V<vFIELD> _fields = V<vFIELD>::NullPtr();
	V<vATTRIBUTE> _attributes = V<vATTRIBUTE>::NullPtr();
	V<vUSHORT> _interfaces = V<vUSHORT>::NullPtr();
	V<vUSHORT> _fieldLookup = V<vUSHORT>::NullPtr();
	vUSHORT _name = 0;
	vUSHORT _super = 0;
	vUSHORT _accessFlags = 0;
	vUSHORT _fieldCount = 0;
	vUSHORT _methodCount = 0;
	vUSHORT _interfaceCount = 0;
	vUSHORT _attributeCount = 0;
	vBYTE _initialized = 0;

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
	std::tuple<bool, vCOMMON> invoke(	V<vClass> self,
					vCPU* cpu,
					vStack* _stack,
					vBYTE opcode,
					const std::string& methodName,
					const std::string& desc);
};