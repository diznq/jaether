#pragma once
#include <stdint.h>
#include "Context.h"
#include "Allocator.h"

#define VMAKE(type, ctx, ...) V<type>(ctx->allocType<type>(__VA_ARGS__))
#define VMAKEGC(type, ctx, ...) V<type>(ctx->allocTypeGc<type>(__VA_ARGS__))
#define VMAKEARRAY(type, ctx, i) V<type>(ctx->allocArray<type>(i))
#define VMAKEGCARRAY(type, ctx, i) V<type>(ctx->allocArrayGc<type>(i))

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
		V() : _addr((A*)(uintptr_t)0) {}
		V(A* addr) : _addr((A*)((uintptr_t)addr)) { }

		uintptr_t U() const { return (uintptr_t)_addr; }

		static V<A> nullPtr() {
			return V<A>();
		}

		size_t release(vContext* ctx, bool arr = false) {
			if (!isValid()) return 0;
			size_t relsize = ctx->freeType<A>(real(ctx), arr);
			_addr = (A*)(uintptr_t)0;
			return relsize;
		}

		size_t release(Allocator* alloc, bool arr = false) {
			if (!isValid()) return 0;
			size_t relsize = alloc->freeRaw(real(alloc));
			_addr = (A*)(uintptr_t)0;
			return relsize;
		}

		bool isValid() const {
			return U();
		}

		A* real(Allocator* ctx) const {
			uintptr_t res = (uintptr_t)_addr;
			ctx->touchVirtual((void*)res);
			return (A*)((res)+(uintptr_t)ctx->getBase());
		}

		A* real(Allocator* ctx, size_t offset) const {
			uintptr_t res = (uintptr_t)_addr + offset * sizeof(A);
			ctx->touchVirtual((void*)res);
			return (A*)((res)+(uintptr_t)ctx->getBase());
		}

		A* real(vContext* ctx) const {
			uintptr_t res = (uintptr_t)_addr;
			ctx->touchVirtual((void*)res);
			return (A*)((res) + ctx->offset());
		}

		A* real(vContext* ctx, size_t offset) const {
			uintptr_t res = (uintptr_t)_addr + offset * sizeof(A);
			ctx->touchVirtual((void*)res);
			return (A*)((res)+ctx->offset());
		}

		A* ptr(vContext* ctx) const {
			return real(ctx);
		}

		A* ptr(Allocator* ctx) const {
			return real(ctx);
		}

		A* v(vContext* ctx = 0) const {
			return (A*)((uintptr_t)_addr);
		}

		A* operator()(vContext* ctx) const {
			return real(ctx);
		}
		
		A& operator()(vContext* ctx, size_t index) const {
			return *real(ctx, index);
		}

		V<A> operator+(const size_t index) const {
			return V<A>(v() + index * sizeof(A));
		}

		V<A>& operator+=(const size_t index) {
			_addr += index * sizeof(A);
			return *this;
		}

		A& operator[](const VCtxIdx& index) const {
			return real(index.ctx, index.index);
		}

		operator bool() const {
			return isValid();
		}
	};

}