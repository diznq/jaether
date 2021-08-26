#pragma once
#include <utility>
#include <stdint.h>

struct vDummy {
	vDummy(int i){}
};

struct vContext {
private:
	void* _pool = 0;
	void* _offset = 0;
	bool _secure = false;
	void* _hashCtx = 0;
	size_t _poolSize = 0;
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

	uintptr_t Offset() const {
		return (uintptr_t)_offset;
	}
};