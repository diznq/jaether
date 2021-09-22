#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <cmath>
#include <map>
#include <string>
#include <chrono>
#include <any>

#include "Opcodes.h"
#include "Pointer.h"
#include "Types.h"
#include "Stack.h"
#include "Memory.h"
#include "Frame.h"
#include "Class.h"

namespace jaether {

	class vCPU {
		bool _running = true;
		std::map<std::string, vNATIVE> _natives;
	public:
		vCPU();

		V<vClass> load(vContext* ctx, const std::string& path, const int nesting=0);
		void addNative(const std::string& path, const std::string& desc, const vNATIVE& native);
		void registerNatives();

		V<vClass> lazyLoad(vContext* ctx, const std::string& path, const int nesting=0);
		std::map<std::string, vNATIVE>::iterator findNative(const std::string& path);

		std::map<std::string, vNATIVE>& getNatives() { return _natives; }

		bool active() const;

		size_t run(vContext* ctx, const V<vFrame>& frame, const int nesting=0);
		std::chrono::steady_clock::time_point getTime() const;

		vOBJECTREF createString(vContext* ctx, vClass* _class, vMemory* _constPool, vUSHORT strIndex, vUSHORT* backref, bool gc = true, const int nesting = 0, const int source = 0);
		vOBJECTREF createString(vContext* ctx, const std::wstring& text, vMemory* _constPool = 0, vUSHORT* backref = 0, bool gc = true, const int nesting = 0, const int source = 0);
		vOBJECTREF createString(vContext* ctx, const std::string& text, bool gc = true, const int nesting = 0, const int source = 0);
		vOBJECTREF createObject(vContext* ctx, const char* className, bool gc = true, const int nesting = 0);

		vOBJECTREF& getJavaClass(vContext* ctx, const char* className, vOBJECTREF* ref=0, bool gc = false);
			
		template<class T> T read(vBYTE* ip) const {
			return *(T*)ip;
		}

		vUSHORT readUSI(vBYTE* ip) const;

		vUINT readUI(vBYTE* ip) const;

		template<class T> vINT compare(T a, T b) {
			if (a == b) return 0;
			if (a > b) return 1;
			return -1;
		}

		template<class T> 
		T& getObject(vContext* ctx, const std::string& key, std::function<T(vContext*)> orElse = nullptr) {
			return ctx->getObject(ctx, key, orElse);
		}
	};

}