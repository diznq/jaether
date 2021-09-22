#pragma once
#include "Pointer.h"
#include "Types.h"
#include <type_traits>
#include <exception>
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
			memset(_memory.real(ctx), 0, size);
		}

		~vStack() {

		}

		void destroy(vContext* ctx) {
			_memory.release(ctx, true);
		}

		void* vbase() const {
			return (void*)_memory.v();
		}

		template<class T> vStack& push(vContext* ctx, const T& value) {
			const size_t size = sizeof(vCOMMON);
			vBYTE* ptr = &_memory(ctx, (size_t)_index);
			char lastByte = ((char*)&value)[sizeof(T) - 1];
			bool isNegative = lastByte < 0;
			memset(ptr, isNegative ? 0xFF : 0, size);
			memcpy(ptr, &value, sizeof(T));
			vCOMMON* vc = (vCOMMON*)ptr;
			if constexpr (!(std::is_same_v<T, vCOMMON>)) {
				if (!(
					(vc->type == vTypes::type<vOBJECTREF>() || vc->type == vTypes::type<vSTRING>())
					&& vTypes::type<T>() == vTypes::type<vREF>())
				)
					vc->type = vTypes::type<T>();
			}
			//if (vc->type == 0) throw std::runtime_error("invalid type");
			_index += size;
			if (_index > _size) throw std::runtime_error("out of stack bounds");
			return *this;
		}

		template<class T> T& pop(vContext* ctx) {
			const size_t size = sizeof(vCOMMON);
			_index -= size;

			if (_index > _size) throw std::runtime_error("out of stack bounds");
			return *(T*)&_memory(ctx, (size_t)_index);
		}

		template<class T> T& get(vContext* ctx, size_t index) {
			const size_t offset = sizeof(vCOMMON) * index;
			if (offset > _index) {
				throw std::runtime_error("offset out of bounds");
			}
			return *(T*)&_memory( ctx,  _index - offset - sizeof(vCOMMON));
		}

		size_t purify(vContext* ctx, size_t start, size_t end) {
			if (start > end) {
				size_t cleaned = 0;
				for (; end < start; end++, cleaned++) {
					_memory(ctx, end) = 0;
				}
				return cleaned;
			}
			return 0;
		}

		size_t index() const {
			return _index;
		}

		void dbgStack(vContext* ctx, const char* op) {
			printf("--------Stack %4s (%5d, %p)--------\n", op, (int)_index, this);
			for (vLONG i = 0; i < (vLONG)_index; i++) {
				if (i > 0 && i % (sizeof(vCOMMON)) == 0)
					printf("\n");
				printf("%02X ", _memory(ctx, (size_t)i));
			}
			printf("\n");
		}
	};

}