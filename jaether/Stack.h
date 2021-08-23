#pragma once
#include "Pointer.h"
#include "Types.h"
#include <type_traits>
#include <stdio.h>
#include <assert.h>

class vStack {
	V<vBYTE> _memory;
	vULONG _index;
	vULONG _size;
public:
	vStack(size_t size = sizeof(vCOMMON) * 65536) {
		_memory = VMAKEARRAY(vBYTE, size);
		_index = 0;
		_size = size;
		memset(_memory.Real(), 0, size);
	}

	~vStack() {
		_memory.Release(true);
	}

	template<class T> vStack& push(const T& value) {
		const size_t size = sizeof(vCOMMON);
		vBYTE* ptr = &_memory[(size_t)_index];
		memset(ptr, 0, size);
		memcpy(ptr, &value, sizeof(T));
		if (!std::is_same_v<T, vCOMMON>) {
			vCOMMON* vc = (vCOMMON*)ptr;
			vc->type = vTypes::type<T>();
		}
		_index += size;
		//dbgStack("push");
		assert(_index >= 0 && _index <= _size);
		return *this;
	}

	template<class T> T pop() {
		const size_t size = sizeof(vCOMMON);
		_index -= size;
		//dbgStack("pop");
		T val = *(T*)&_memory[(size_t)_index];
		assert(_index >= 0 && _index <= _size);
		return val;
	}

	void dbgStack(const char* op) {
		printf("--------Stack %4s (%5d, %p)--------\n", op, (int)_index, this);
		for (vLONG i = 0; i < (vLONG)_index; i++) {
			if (i > 0 && i % (sizeof(vCOMMON)) == 0)
				printf("\n");
			printf("%02X ", _memory[i]);
		}
		printf("\n");
	}
};