#pragma once
#include "Types.h"
#include "Pointer.h"
#include "Stack.h"
#include "Memory.h"
#include <map>
#include <functional>
#include <fstream>
#include <tuple>

namespace jaether {

	class vCPU;
	class vClass;

	typedef std::function<void(vContext* ctx, const std::string& className, vCPU* cpu, vStack* stack, vBYTE opcode)> vNATIVE;

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

		vClass(vContext* ctx, const char* name);
		~vClass();
		void destroy(vContext* ctx);

		const char* getName(vContext* ctx);
		const char* getSuperName(vContext* ctx);

		template<class T> T read(vBYTE* ip) const {
			return *(T*)ip;
		}

		vUSHORT		readUSI(vBYTE* ip) const;
		vUINT		readUI(vBYTE* ip) const;
		vDOUBLE		readDouble(vBYTE* ip) const;
		vUSHORT		readUSI(std::ifstream& stream) const;
		vUINT		readUI(std::ifstream& stream) const;
		vDOUBLE		readDouble(std::ifstream& stream) const;
		void		readAttribute(vContext* ctx, std::ifstream& f, vATTRIBUTE& attr);
		void		readField(vContext* ctx, std::ifstream& f, vFIELD& field);
		vATTRIBUTE* getAttribute(vContext* ctx, const vFIELD* field, const char* name);
		vFIELD* getField(vContext* ctx, const char* name);
		vMETHOD* getMethod(vContext* ctx, const char* name, const char* desc = 0);

		V<vUTF8BODY> toString(vContext* ctx, vUSHORT index, int selector = 0) const;
		V<vBYTE>	getCode(vContext* ctx, vMETHOD* method);
		vUINT		argsCount(vContext* ctx, vMETHOD* method);
		std::tuple<bool, vCOMMON> invoke(
			vContext* ctx,
			V<vClass> self,
			vCPU* cpu,
			vStack* _stack,
			vBYTE opcode,
			const std::string& methodName,
			const std::string& desc);
	};

}