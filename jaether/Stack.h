#pragma once
#include "Pointer.h"
#include "Types.h"
#include <type_traits>
#include <stdio.h>
#include <assert.h>

namespace jaether {

	class vStack {
		V<vBYTE> _memory;
		vULONG _index;
		vULONG _size;
	public:
		vStack(vContext* ctx, size_t size = sizeof(vCOMMON) * 65536) {
			_memory = VMAKEARRAY(vBYTE, ctx, size);
			_index = 0;
			_size = size;
			memset(_memory.Real(ctx), 0, size);
		}

		~vStack() {

		}

		void destroy(vContext* ctx) {
			_memory.Release(ctx, true);
		}

		template<class T> vStack& push(vContext* ctx, const T& value) {
			const size_t size = sizeof(vCOMMON);
			vBYTE* ptr = &_memory[VCtxIdx{ ctx, (size_t)_index }];
			char lastByte = ((char*)&value)[sizeof(T) - 1];
			bool isNegative = lastByte < 0;
			memset(ptr, isNegative ? 0xFF : 0, size);
			memcpy(ptr, &value, sizeof(T));
			if constexpr (!(std::is_same_v<T, vCOMMON>)) {
				vCOMMON* vc = (vCOMMON*)ptr;
				if (!(
					(vc->type == vTypes::type<vOBJECTREF>() || vc->type == vTypes::type<vSTRING>())
					&& vTypes::type<T>() == vTypes::type<vREF>()))
					vc->type = vTypes::type<T>();
			}
			_index += size;
			//dbgStack(ctx, "push");
			assert(_index <= _size);
			return *this;
		}

		template<class T> T& pop(vContext* ctx) {
			const size_t size = sizeof(vCOMMON);
			_index -= size;
			//dbgStack("pop");
			assert(_index <= _size);
			return *(T*)&_memory[VCtxIdx{ ctx, (size_t)_index }];
		}

		template<class T> T& get(vContext* ctx, size_t index) {
			const size_t offset = sizeof(vCOMMON) * index;
			assert(_offset <= _index);
			return *(T*)&_memory[VCtxIdx{ ctx, _index - offset - sizeof(vCOMMON) }];
		}

		void dbgStack(vContext* ctx, const char* op) {
			printf("--------Stack %4s (%5d, %p)--------\n", op, (int)_index, this);
			for (vLONG i = 0; i < (vLONG)_index; i++) {
				if (i > 0 && i % (sizeof(vCOMMON)) == 0)
					printf("\n");
				printf("%02X ", _memory[VCtxIdx{ ctx, (size_t)i }]);
			}
			printf("\n");
		}
	};

}