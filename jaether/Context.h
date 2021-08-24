#pragma once
#include <utility>
#include <stdint.h>

struct vDummy {
	vDummy(int i){}
};

struct vContext {
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

	uintptr_t Offset() const {
		return 0;
	}
};