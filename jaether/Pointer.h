#pragma once
#include <stdint.h>
#include "Context.h"

#define VMAKE(type, ctx, ...) V<type>(ctx->allocType<type>(__VA_ARGS__))
#define VMAKEARRAY(type, ctx, i) V<type>(ctx->allocArray<type>(i))

namespace jaether {

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
		V(A* addr) : _addr((A*)((uintptr_t)addr)) { }

		uintptr_t U() const { return (uintptr_t)_addr; }

		static V<A> nullPtr() {
			return V<A>();
		}

		bool release(vContext* ctx, bool arr = false) {
			if (!isValid()) return false;
			ctx->freeType<A>(real(ctx), arr);
			_addr = (A*)~(uintptr_t)0;
			return true;
		}

		bool isValid() const {
			return ~U();
		}

		A* real(vContext* ctx) const {
			return (A*)(((uintptr_t)_addr) + ctx->offset());
		}

		A* ptr(vContext* ctx) const {
			return real(ctx);
		}

		A* v(vContext* ctx = 0) const {
			return (A*)((uintptr_t)_addr);
		}

		/*A* operator->() const {
			return Real();
		}

		A& operator*() const {
			return *Real();
		}*/

		V<A> operator+(const size_t index) const {
			return V<A>(v() + index);
		}

		V<A>& operator+=(const size_t index) {
			_addr += index;
			return *this;
		}

		A& operator[](const VCtxIdx& index) const {
			return real(index.ctx)[index.index];
		}

		operator bool() const {
			return isValid();
		}
	};

}