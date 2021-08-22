#pragma once
#include "Pointer.h"
#include "Types.h"
#include <iostream>
#include <type_traits>

class vMemory {
	V<vCOMMON> _memory;
	vULONG _size;
public:
	vMemory(size_t size = sizeof(vCOMMON) * 65536) {
		_memory = VMAKEARRAY(vCOMMON, size);
		_size = size;
		memset(_memory.Real(), 0, size);
	}

	~vMemory() {
		_memory.Release(true);
	}

	template<class T> vMemory& set(const size_t index, const T& value) {
		const size_t size = sizeof(vCOMMON);
		assert(index < _size);
		vBYTE* ptr = (vBYTE*)&_memory[index];
		memset(ptr, 0, size);
		memcpy(ptr, &value, sizeof(T));
		if constexpr (!std::is_same_v<T, vCOMMON>) {
			vCOMMON* vc = (vCOMMON*)ptr;
			vc->type = vTypes::type<T>();
		}
		return *this;
	}

	template<class T> T get(const size_t index) const {
		const size_t size = sizeof(vCOMMON);
		assert(index < _size);
		T val = *(T*)&_memory[index];
		return val;
	}
};