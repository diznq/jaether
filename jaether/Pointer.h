#pragma once
#include <stdint.h>
#include "Context.h"

#define VMAKE(type, ctx, ...) V<type>(ctx->AllocType<type>(__VA_ARGS__))
#define VMAKEARRAY(type, ctx, i) V<type>(ctx->AllocArray<type>(i))

struct vContext;

struct VCtxIdx {
	vContext* ctx;
	size_t index;
};

template<class A>
class V {
	A* _addr = 0;
public:
	V() : _addr((A*)~(uintptr_t)0) {}
	V(A* addr) : _addr((A*)((uintptr_t)addr)){ }

	uintptr_t U() const { return (uintptr_t)_addr; }

	static V<A> NullPtr() {
		return V<A>();
	}

	bool Release(vContext* ctx, bool arr = false) {
		if (!IsValid()) return false;
		ctx->FreeType<A>(Real(ctx), arr);
		_addr = (A*)~(uintptr_t)0;
		return true;
	}

	bool IsValid() const {
		return ~U();
	}

	A* Real(vContext* ctx) const {
		return (A*)(((uintptr_t)_addr) + ctx->Offset());
	}

	A* Ptr(vContext* ctx) const {
		return Real(ctx);
	}

	A* Virtual(vContext* ctx = 0) const {
		return (A*)((uintptr_t)_addr);
	}

	/*A* operator->() const {
		return Real();
	}

	A& operator*() const {
		return *Real();
	}*/

	V<A> operator+(const size_t index) const {
		return V<A>(Virtual() + index);
	}

	V<A>& operator+=(const size_t index) {
		_addr += index;
		return *this;
	}

	A& operator[](const VCtxIdx& index) const {
		return Real(index.ctx)[index.index];
	}

	operator bool() const {
		return IsValid();
	}
};