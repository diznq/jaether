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
			vBYTE* ptr = (vBYTE*)&_memory( ctx,  index);
			memset(ptr, 0, size);
			memcpy(ptr, &value, sizeof(T));
			if constexpr (!std::is_same_v<T, vCOMMON>) {
				vCOMMON* vc = (vCOMMON*)ptr;
				if (!(
					(vc->type == vTypes::type<vOBJECTREF>() || vc->type == vTypes::type<vSTRING>()) 
					&& vTypes::type<T>() == vTypes::type<vREF>()))
					vc->type = vTypes::type<T>();
			}
			return *this;
		}

		template<class T> T& get(vContext* ctx, const size_t index) const {
			const size_t size = sizeof(vCOMMON);
			assert(index < _size);
			return *(T*)&_memory( ctx,  index);
		}
	};

}