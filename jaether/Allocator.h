#pragma once
#include <vector>
#include <set>
#include <string.h>

namespace jaether {

	class Allocator {
		unsigned char* _pool;
		size_t _size;
		std::vector<bool> _free;
		std::vector<size_t> _sizes;
		std::set<unsigned int> _touched;
		size_t _align;
		size_t _allocs = 0;
		size_t _cycles = 0;
		size_t _managedSize = 0;
		size_t _peakSize = 0;
		size_t _firstFree = 0;
	public:
		Allocator(size_t poolSize, size_t align = 4) {
			const int MOD = (1 << align) - 1;
			poolSize += (align - (poolSize & MOD)) & MOD;
			_size = poolSize;
			_pool = new unsigned char[_size];
			_align = align;
			memset(_pool, 0, _size);
			_free.resize(_size >> align);
			_sizes.resize(_size >> align);
			for (size_t i = 0; i < _size >> align; i++) {
				_free[i] = i > 0;
				_sizes[i] = i == 0 ? 1 : 0;
			}
			_firstFree = 1;
		}

		void* getBase() {
			return (void*)_pool;
		}

		void* allocRaw(size_t mem);

		void freeRaw(void* mem);

		size_t getSize() const {
			return _size;
		}

		size_t getManagedSize() const {
			return _managedSize * _align;
		}

		size_t getPeakSize() const {
			return _peakSize * _align;
		}

		size_t getAvgCycles() const {
			return _cycles / _allocs;
		}

		void touchVirtual(void* memory);

		size_t flushTouched() {
			size_t sz = _touched.size();
			_touched.clear();
			return sz;
		}

		std::set<unsigned int>& getTouchedVSegments() {
			return _touched;
		}

		size_t getAlignment() const {
			return _align;
		}
	};

}