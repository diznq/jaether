#pragma once
#include <vector>
#include <set>
#include <string.h>

namespace jaether {

	class Allocator {
		unsigned char* _pool;
		size_t _size;
		std::vector<bool> _free;
		std::vector<unsigned int> _sizes;
		std::set<unsigned int> _touched;
		std::set<unsigned int> _gc;
		size_t _align;
		size_t _allocs = 0;
		size_t _cycles = 0;
		size_t _managedSize = 0;
		size_t _peakSize = 0;
		size_t _firstFree = 0;
	public:
		Allocator(size_t poolSize, size_t align = 4);

		void* getBase() {
			return (void*)_pool;
		}

		void* allocRaw(size_t mem, bool gc = false);

		size_t freeRaw(void* mem);

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

		size_t gcCycle();

		std::set<unsigned int>& getTouchedVSegments() {
			return _touched;
		}

		size_t getAlignment() const {
			return _align;
		}
	};

}