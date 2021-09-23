#pragma once
#include "Pointer.h"
#include "Types.h"
#include <assert.h>
#include <iostream>
#include <type_traits>

namespace jaether {

	class vMemory {
		V<vCOMMON> _memory;
		vULONG _size;
	public:
		vMemory(vContext* ctx, size_t size = sizeof(vCOMMON) * 65536) {
			_memory = VMAKEARRAY(vCOMMON, ctx, size);
			_size = size;
			memset(_memory.real(ctx), 0, size);
		}

		~vMemory() {
		}

		void destroy(vContext* ctx) {
			_memory.release(ctx, true);
		}

		template<class T> vMemory& set(vContext* ctx, const size_t index, const T& value) {
			const size_t size = sizeof(vCOMMON);
			assert(index < _size);
			vBYTE* ptr = (vBYTE*)&_memory(ctx, index);
			ctx->setMemory((_memory + (size_t)index).v(), 0, size);
			ctx->copyMemory((_memory + (size_t)index).v(), &value, sizeof(T), true);
			vCOMMON* vc = (vCOMMON*)ptr;	
			if constexpr (!std::is_same_v<T, vCOMMON>) {
				vc->type = vTypes::type<T>();
			}
			//dbgMem(ctx);
			//if (vc->type == 0) throw std::runtime_error("invalid type");
			return *this;
		}

		template<class T> T& get(vContext* ctx, const size_t index) const {
			const size_t size = sizeof(vCOMMON);
			assert(index < _size);
			return *(T*)&_memory(ctx, index);
		}

		vULONG size() const { return _size; }

		void dbgMem(vContext* ctx) {
			if (_size >= 512 / sizeof(vCOMMON)) return;
			vBYTE* ptr = (vBYTE*)_memory(ctx);
			printf("------Memory-----\n");
			for (size_t i = 0; i < _size * sizeof(vCOMMON); i++) {
				printf("%02X ", ptr[i]);
				if (i % 16 == 15) {
					printf("\n");
				}
			}
			printf("\n");
		}
	};

}