#pragma once
#include <stdint.h>
#include "Context.h"
#include "Allocator.h"

#define CENTRALIZE_RESOLVE

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

	enum class W {
		T
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
			size_t relsize = ctx->freeType<A>(real(ctx, W::T), arr);
			_addr = (A*)(uintptr_t)0;
			return relsize;
		}

		size_t release(Allocator* alloc, bool arr = false) {
			if (!isValid()) return 0;
			size_t relsize = alloc->freeRaw(real(alloc, W::T));
			_addr = (A*)(uintptr_t)0;
			return relsize;
		}

		bool isValid() const {
			return U();
		}

		// readables

		inline const A* real(Allocator* ctx) const {
			uintptr_t res = (uintptr_t)_addr;
			return (A*)ctx->resolve(res);
		}

		inline const A* real(Allocator* ctx, size_t offset) const {
			uintptr_t res = (uintptr_t)_addr + offset * sizeof(A);
			return (A*)ctx->resolve(res);
		}

		inline const A* real(vContext* ctx) const {
			uintptr_t res = (uintptr_t)_addr;
			return (A*)ctx->resolve(res);
		}

		inline const A* real(vContext* ctx, size_t offset) const {
			uintptr_t res = (uintptr_t)_addr + offset * sizeof(A);
			return (A*)ctx->resolve(res);
		}

		// writables

		inline A* real(Allocator* ctx, W w) const {
			uintptr_t res = (uintptr_t)_addr;
			ctx->touchVirtual((void*)res, sizeof(A));
			return (A*)ctx->resolve(res);
		}

		inline A* real(Allocator* ctx, size_t offset, W w) const {
			uintptr_t res = (uintptr_t)_addr + offset * sizeof(A);
			ctx->touchVirtual((void*)res, sizeof(A));
			return (A*)ctx->resolve(res);
		}

		inline A* real(vContext* ctx, W w) const {
			uintptr_t res = (uintptr_t)_addr;
			ctx->touchVirtual((void*)res, sizeof(A));
			return (A*)ctx->resolve(res);
		}

		inline A* real(vContext* ctx, size_t offset, W w) const {
			uintptr_t res = (uintptr_t)_addr + offset * sizeof(A);
			ctx->touchVirtual((void*)res, sizeof(A));
			return (A*)ctx->resolve(res);
		}

		A* v(vContext* ctx = 0) const {
			return (A*)((uintptr_t)_addr);
		}

		const A* operator()(vContext* ctx) const {
			return real(ctx);
		}

		A* operator()(vContext* ctx, W w) const {
			return real(ctx, W::T);
		}

		const A& operator()(vContext* ctx, size_t index) const {
			return *real(ctx, index);
		}

		A& operator()(vContext* ctx, size_t index, W w) const {
			return *real(ctx, index, W::T);
		}

		V<A> operator+(const size_t index) const {
			return V<A>(v() + index);
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