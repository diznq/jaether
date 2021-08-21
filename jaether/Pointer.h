#pragma once
#include <stdint.h>

#define VMAKE(type, ...) V<type>(new type(__VA_ARGS__))
#define VMAKEARRAY(type, i) V<type>(new type[i])

template<class A>
class V {
	A* _addr = 0;
	void* _offset = 0;
public:
	V() : _addr(0), _offset(0) {}
	V(A* addr, void* offset = 0) : _addr(addr), _offset(offset) {}

	bool Release(bool arr = false) {
		if (!IsValid()) return false;
		if (arr) delete[] Real();
		else delete Real();
		_addr = (A*)(uintptr_t)(-(intptr_t)_offset);
		return true;
	}

	bool IsValid() const {
		return Real() != 0;
	}

	A* Real() const {
		return (A*)(((uintptr_t)_addr) + Offset());
	}

	A* Virtual() const {
		return (A*)_addr;
	}
	
	uintptr_t Offset() const {
		return (uintptr_t)_offset;
	}

	A* operator->() const {
		return Real();
	}

	A& operator*() const {
		return *Real();
	}

	V<A> operator+(const size_t index) const {
		return V<A>(_addr + index, _offset);
	}

	V<A>& operator+=(const size_t index) {
		_addr += index;
		return *this;
	}

	A& operator[](const size_t index) const {
		return Real()[index];
	}

	operator bool() const {
		return _addr != (A*)0;
	}
};