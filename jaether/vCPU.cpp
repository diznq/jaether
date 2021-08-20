#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

typedef int8_t		vCHAR;
typedef int16_t		vSHORT;
typedef int32_t		vINT;
typedef int64_t		vLONG;
typedef uint8_t		vBYTE;
typedef uint16_t	vUSHORT;
typedef uint32_t	vUINT;
typedef uint64_t	vULONG;
typedef float vFLOAT;
typedef double vDOUBLE;

#define VMAKE(type, ...) V<type>(new type(__VA_ARGS__))
#define VMAKEARRAY(type, i) V<type>(new type[i])

template<class A>
class V {
	A* _addr = 0;
	void* _offset = 0;
public:
	V() : _addr(0), _offset(0) {}
	V(A* addr, void* offset=0) : _addr(addr), _offset(offset) {}

	A* Real() const {
		return (A*)(((uintptr_t)_addr) + (uintptr_t)_offset);
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

template<size_t N = 1024>
class vStack {
	V<vBYTE> _memory;
	V<vULONG> _index;
public:
	vStack() {
		_index = V<vULONG>(new vULONG);
		_memory = V<vBYTE>(new vBYTE[N]);
		*_index = 0;
	}

	~vStack() {
		delete _index.Real();
		delete[] _memory.Real();
	}

	template<class T> vStack& push(const T& value) {
		vBYTE* ptr = &_memory[(size_t)*_index];
		memcpy(ptr, &value, sizeof(T));
		(*_index) += sizeof(T);
		assert(*_index >= 0 && *_index <= N);
		return *this;
	}

	template<class T> T pop() {
		(*_index) -= sizeof(T);
		T val = *(T*)&_memory[(size_t)*_index];
		assert(*_index >= 0 && *_index <= N);
		return val;
	}
};

class vCPU {

};

int main() {
	auto stk = VMAKE(vStack<1024>);
	for (int i = 0; i < 256; i++) {
		stk->push<int>(i);
	}
	for (int i = 0; i < 256; i++) {
		printf("%d\n", stk->pop<int>());
	}
}