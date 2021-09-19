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
	class vFrame;

#define JAETHER_CLASS_TAG 0x33000033

	typedef std::function<void(vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode)> vNATIVE;

	class vClass {
	public:
		int TAG = JAETHER_CLASS_TAG;
		V<vMemory> _constPool = V<vMemory>::nullPtr();
		V<vFIELD> _methods = V<vFIELD>::nullPtr();
		V<vFIELD> _fields = V<vFIELD>::nullPtr();
		V<vATTRIBUTE> _attributes = V<vATTRIBUTE>::nullPtr();
		V<vUSHORT> _interfaces = V<vUSHORT>::nullPtr();
		//V<vUSHORT> _fieldLookup = V<vUSHORT>::NullPtr();
		vUSHORT _name = 0;
		vUSHORT _super = 0;
		vUSHORT _accessFlags = 0;
		vUSHORT _fieldCount = 0;
		vUSHORT _methodCount = 0;
		vUSHORT _interfaceCount = 0;
		vUSHORT _attributeCount = 0;
		vUSHORT _fieldOffset = 0;
		vUSHORT _methodOffset = 0;
		vBYTE _initialized = 0;

		vClass(vContext* ctx, vCPU* cpu, const char* name, const int nesting);
		~vClass();
		void destroy(vContext* ctx);

		const char* getName(vContext* ctx);
		const char* getSuperName(vContext* ctx);

		template<class T> T read(vBYTE* ip) const {
			return *(T*)ip;
		}

		vUSHORT		readUSI(vBYTE* ip) const;
		vUINT		readUI(vBYTE* ip) const;
		vINT		readInt(vBYTE* ip) const;
		vDOUBLE		readDouble(vBYTE* ip) const;
		vFLOAT		readFloat(vBYTE* ip) const;
		vLONG		readLong(vBYTE* ip) const;
		vUSHORT		readUSI(std::ifstream& stream) const;
		vUINT		readUI(std::ifstream& stream) const;
		vINT		readInt(std::ifstream& stream) const;
		vDOUBLE		readDouble(std::ifstream& stream) const;
		vFLOAT		readFloat(std::ifstream& stream) const;
		vLONG		readLong(std::ifstream& stream) const;
		void		readAttribute(vContext* ctx, std::ifstream& f, vATTRIBUTE& attr);
		void		readField(vContext* ctx, std::ifstream& f, vFIELD& field);
		vATTRIBUTE* getAttribute(vContext* ctx, const vFIELD* field, const char* name);
		vFIELD*		getField(vContext* ctx, const char* name);
		vMETHOD*	getMethod(vContext* ctx, const char* name, const char* desc = 0);
		vCOMMON*	getObjField(vContext* ctx, V<vOBJECT> obj, const char* name);
		vCOMMON*	getObjField(vContext* ctx, V<vOBJECTREF> objref, const char* name);

		bool		instanceOf(vContext* ctx, V<vClass> cls);

		V<vUTF8BODY> toString(vContext* ctx, vUSHORT index, int selector = 0) const;
		std::string toStdString(vContext* ctx, vUSHORT index, int selector = 0) const;
		const char* toCString(vContext* ctx, vUSHORT index, int selector = 0) const;
		V<vBYTE>	getCode(vContext* ctx, vMETHOD* method);
		vOBJECTREF& getJavaClass(vCPU* cpu, vContext* ctx, vStack* stack, bool gc = false);
		vUINT		argsCount(vContext* ctx, vMETHOD* method);
		vUINT		argsCount(const char* desc);

		std::tuple<bool, vCOMMON> invoke(
			vContext* ctx,
			V<vClass> self,
			V<vClass> super,
			vCPU* cpu,
			vStack* _stack,
			vBYTE opcode,
			const std::string& methodName,
			const std::string& desc,
			int nesting = 0);

		std::tuple<bool, vFrame*> createFrame(
			vContext* ctx,
			V<vClass> self,
			V<vClass> super,
			vCPU* cpu,
			vStack* _stack,
			vBYTE opcode,
			const std::string& methodName,
			const std::string& desc,
			int nesting = 0);
	};

}