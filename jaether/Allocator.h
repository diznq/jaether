#pragma once
#include <vector>
#include <set>
#include <string.h>
#include <fstream>

namespace jaether {

	class Allocator {
		std::vector<unsigned char> _pool;
		std::vector<unsigned char> _free;
		std::vector<unsigned int> _sizes;
		std::set<unsigned int> _touched;
		std::set<unsigned int> _gc;
		size_t _align = 4;
		size_t _allocs = 0;
		size_t _cycles = 0;
		size_t _managedSize = 0;
		size_t _peakSize = 0;
		size_t _firstFree = 0;
	public:
		Allocator(size_t poolSize, size_t align = 4);

		void* getBase() {
			return (void*)_pool.data();
		}

		void* allocRaw(size_t mem, bool gc = false);

		size_t freeRaw(void* mem);

		size_t getSize() const {
			return _pool.size();
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

		void save(std::ofstream& os);
		void load(std::ifstream& is);
	};

}