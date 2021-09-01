#pragma once
#include <utility>
#include <stdint.h>
#include <map>
#include <string>
#include "Allocator.h"


namespace jaether {

	class vClass;

	struct vDummy {
		vDummy(int i) {}
	};

	struct vContext {
	private:
		bool _secure = false;
		void* _hashCtx = 0;
		size_t _ops = 0;
		Allocator* _alloc = 0;
		std::map<std::string, vClass*> _classes;
	public:
		vContext(size_t mem = 16 * 1024 * 1024, bool secure = false);
		~vContext();
		void* Alloc(size_t mem);
		void Free(void* mem, bool arr = false);
		template<typename T, typename... Args>
		T* AllocType(Args&&... args) {
			char* mem = ((char*)Alloc(sizeof(T)));
			new ((T*)(mem + Offset())) T(
				std::forward<Args>(args)...
			);
			return (T*)mem;
		}
		template<typename T>
		T* AllocArray(size_t size) {
			return (T*)Alloc(sizeof(T) * size);
		}
		template<typename T>
		void FreeType(T* obj, bool arr = false) {
			Free((void*)obj, arr);
		}

		void OnInstruction();
		void GetSignature(unsigned char* buffer);

		uintptr_t Offset() const;

		Allocator* GetAllocator() const {
			return _alloc;
		}

		std::map<std::string, vClass*>& GetClasses() {
			return _classes;
		}

		size_t Ops() const {
			return _ops;
		}
	};

}