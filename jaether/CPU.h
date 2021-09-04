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
		std::map<std::string, std::any> _storage;
	public:
		vCPU();

		V<vClass> load(vContext* ctx, const std::string& path, const int nesting=0);
		void addNative(const std::string& path, const std::string& desc, const vNATIVE& native);
		void registerNatives();

		std::map<std::string, vClass*>::iterator lazyLoad(vContext* ctx, const std::string& path, const int nesting=0);

		bool active() const;

		size_t run(vContext* ctx, const V<vFrame>& frame, const int nesting=0);
		std::chrono::steady_clock::time_point getTime() const;

		vOBJECTREF createString(vContext* ctx, vClass* _class, vStack* _stack, vMemory* _constPool, vUSHORT strIndex, vUSHORT* backref, const int nesting = 0);
		vOBJECTREF createString(vContext* ctx, vStack* _stack, const std::wstring& text, vMemory* _constPool = 0, vUSHORT* backref = 0, const int nesting = 0);
		vOBJECTREF createObject(vContext* ctx, const char* className, const int nesting = 0);
			
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
			auto it = _storage.find(key);
			if (it != _storage.end()) {
				return std::any_cast<T&>(it->second);
			}
			if (!orElse) throw std::runtime_error("or else callback not found");
			return std::any_cast<T&>(_storage[key] = orElse(ctx));
		}
	};

}