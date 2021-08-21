#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "Opcodes.h"

typedef int8_t		vCHAR;
typedef int16_t		vSHORT;
typedef int32_t		vINT;
typedef int64_t		vLONG;
typedef uint8_t		vBYTE;
typedef uint16_t	vUSHORT;
typedef uint32_t	vUINT;
typedef uint64_t	vULONG;
typedef uint32_t	vCAT1;
typedef uint64_t	vCAT2;
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

template<size_t N = 1024>
class vCPU {
	vBYTE* _program;
	V<vStack<N>> _stack;
public:
	vCPU() {
		_stack = VMAKE(vStack<N>);
	}
	size_t execute(vBYTE* ip) {
		vBYTE opcode = ip[0];
		switch (opcode) {
		case iconst_0:
			_stack->push<vCAT1>(0);
			return 0;
		case iconst_1:
			_stack->push<vCAT1>(1);
			return 0;
		case iconst_2:
			_stack->push<vCAT1>(2);
			return 0;
		case iconst_3:
			_stack->push<vCAT1>(3);
			return 0;
		case iconst_4:
			_stack->push<vCAT1>(4);
			return 0;
		case iconst_5:
			_stack->push<vCAT1>(5);
			return 0;
		case iconst_m1:
			_stack->push<vCAT1>(-1);
			return 0;
		case lconst_0:
			_stack->push<vCAT2>(0);
			return 0;
		case lconst_1:
			_stack->push<vCAT2>(1);
			return 0;
		case dconst_0:
			_stack->push<double>(0.0);
			return 0;
		case dconst_1:
			_stack->push<double>(1.0);
			return 0;
		case fconst_0:
			_stack->push<float>(0.0f);
			return 0;
		case fconst_1:
			_stack->push<float>(1.0f);
			return 0;
		case fconst_2:
			_stack->push<float>(2.0f);
			return 0;
		case bipush:
			_stack->push<vCAT1>(read<vBYTE>(ip + 1));
			return 1;
		case sipush:
			_stack->push<vCAT1>(read<vSHORT>(ip + 1));
			return 2;
		}
	}

	template<class T> T read(vBYTE* ip) const {
		return *(T*)ip;
	}
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