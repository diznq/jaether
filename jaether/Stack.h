#pragma once
#include "Pointer.h"
#include "Types.h"

class vStack {
	V<vBYTE> _memory;
	V<vULONG> _index;
	V<vULONG> _size;
public:
	vStack(size_t size = sizeof(vCOMMON) * 65536) {
		_index = VMAKE(vULONG);
		_size = VMAKE(vULONG);
		_memory = VMAKEARRAY(vBYTE, size);
		*_index = 0;
		*_size = size;
	}

	~vStack() {
		delete _index.Real();
		delete _size.Real();
		delete[] _memory.Real();
	}

	template<class T> vStack& push(const T& value) {
		const size_t size = sizeof(vCOMMON);
		vBYTE* ptr = &_memory[(size_t)*_index];
		memset(ptr, 0, size);
		memcpy(ptr, &value, sizeof(T));
		(*_index) += size;
		assert(*_index >= 0 && *_index <= *_size);
		return *this;
	}

	template<class T> T pop() {
		const size_t size = sizeof(vCOMMON);
		(*_index) -= size;
		T val = *(T*)&_memory[(size_t)*_index];
		assert(*_index >= 0 && *_index <= *_size);
		return val;
	}
};