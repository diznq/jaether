#pragma once
#include "Pointer.h"
#include "Types.h"
class vMemory {
	V<vCOMMON> _memory;
	V<vULONG> _size;
public:
	vMemory(size_t size = sizeof(vCOMMON) * 65536) {
		_memory = V<vCOMMON>(new vCOMMON[size]);
		_size = V<vULONG>(new vULONG);
		*_size = size;
		memset(_memory.Real(), 0, size);
	}

	~vMemory() {
		delete _size.Real();
		delete[] _memory.Real();
	}

	template<class T> vMemory& set(const size_t index, const T& value) {
		const size_t size = sizeof(vCOMMON);
		assert(index < *_size);
		vBYTE* ptr = (vBYTE*)&_memory[index];
		memset(ptr, 0, size);
		memcpy(ptr, &value, sizeof(T));
		return *this;
	}

	template<class T> T get(const size_t index) {
		const size_t size = sizeof(vCOMMON);
		assert(index < *_size);
		T val = *(T*)&_memory[index];
		return val;
	}
};