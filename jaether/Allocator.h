#pragma once
#include <vector>
#include <string.h>

namespace jaether {

	class Allocator {
		unsigned char* _pool;
		size_t _size;
		std::vector<bool> _free;
		std::vector<size_t> _sizes;
		size_t _align;
		size_t _allocs = 0;
		size_t _cycles = 0;
		size_t _managedSize = 0;
		size_t _peakSize = 0;
		size_t _firstFree = 0;
	public:
		Allocator(size_t poolSize, size_t align = 16) {
			poolSize += (align - poolSize % align) % align;
			_size = poolSize;
			_pool = new unsigned char[_size];
			_align = align;
			memset(_pool, 0, _size);
			_free.resize(_size / align);
			_sizes.resize(_size / align);
			for (size_t i = 0; i < _size / align; i++) {
				_free[i] = true;
				_sizes[i] = 0;
			}
		}

		void* GetBase() {
			return (void*)_pool;
		}

		void* Alloc(size_t mem);

		void Free(void* mem);

		size_t GetSize() const {
			return _size;
		}

		size_t GetManagedSize() const {
			return _managedSize * _align;
		}

		size_t GetPeakSize() const {
			return _peakSize * _align;
		}

		size_t GetAvgCycles() const {
			return _cycles / _allocs;
		}
	};

}