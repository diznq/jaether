#include "Allocator.h"

namespace jaether {
	void* Allocator::allocRaw(size_t mem) {
		const int MOD = (1 << _align) - 1;
		size_t required = (mem >> _align) + 1;
		if ((mem & MOD) == 0) required--;
		_managedSize += required;
		if (_managedSize > _peakSize)
			_peakSize = _managedSize;
		_allocs++;
		for (size_t i = _firstFree, j = _free.size(); i < j; _cycles++) {
			bool suitable = false;
			if (_free[i]) {
				size_t blocks = required;
				for (size_t k = i, l = i + required - 1; l >= k && l < j; l--, _cycles++) {
					if (!_free[l]) break;
					blocks--;
				}
				//printf("Blocks found at %llx: %lld\n", i * _align, blocks);
				if (blocks == 0) {
					for (size_t k = i, l = i + required, m = 0; k < l && k < j; k++, m++) {
						_free[k] = false;
						_sizes[k] = required - m;
					}
					size_t offset = i << _align;
					_firstFree = i + required;
					//printf("Allocated memory at %p, offset: %llx, size: %llu\n", _pool + offset, offset, required);
					return (void*)(_pool + offset);
				}
				else {
					i += required;
				}
			}
			else {
				i += _sizes[i];
			}
		}
		printf("Allocation failed");
		return 0;
	}

	void Allocator::freeRaw(void* mem) {
		uintptr_t offset = (uintptr_t)mem - (uintptr_t)_pool;
		uintptr_t id = offset >> _align;
		uintptr_t blocks = _sizes[id];
		//printf("Freed memory at %p, offset: %llx, size: %llu\n", mem, offset, blocks);
		_managedSize -= blocks;
		if (id < _firstFree) _firstFree = id;
		for (size_t i = id; i < id + blocks && i < _free.size(); i++) {
			_free[i] = true;
			_sizes[i] = 0;
		}
	}

	void Allocator::touchVirtual(void* mem) {
		uintptr_t offset = (uintptr_t)mem;
		unsigned int id = (unsigned int)(offset >> _align);
		_touched.insert(id);
		_touched.insert(id + 1);
	}
}