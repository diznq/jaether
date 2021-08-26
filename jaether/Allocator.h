#pragma once
#include <vector>
#include <string.h>

class Allocator {
	unsigned char *_pool;
	size_t _size;
	std::vector<bool> _free;
	std::vector<size_t> _sizes;
	size_t _align;
	size_t _allocs = 0;
	size_t _cycles = 0;
	size_t _managedSize = 0;
	size_t _peakSize = 0;
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

	void* Alloc(size_t mem) {
		size_t required = mem / _align + 1;
		if (mem % _align == 0) required--;
		_managedSize += required;
		if (_managedSize > _peakSize)
			_peakSize = _managedSize;
		_allocs++;
		//printf("Request alloc %llu bytes (%lld blocks)\n", mem, required);
		for (size_t i = 0, j = _free.size(); i < j;_cycles++) {
			bool suitable = false;
			if (_free[i]) {
				size_t blocks = required;
				for (size_t k = i, l = i + required; k < l && k < j; k++) {
					if (!_free[k]) break;
					blocks--;
				}
				//printf("Blocks found at %llx: %lld\n", i * _align, blocks);
				if (blocks == 0) {
					for (size_t k = i, l = i + required, m=0; k < l && k < j; k++,m++) {
						_free[k] = false;
						_sizes[k] = required - m;
					}
					size_t offset = i * _align;
					//printf("Allocated memory at %p, offset: %llx, size: %llu\n", _pool + offset, offset, required);
					return (void*)(_pool + offset);
				} else {
					i += required;
				}
			} else {
				i += _sizes[i];
			}
		}
		return 0;
	}

	void Free(void* mem) {
		uintptr_t offset = (uintptr_t)mem - (uintptr_t)_pool;
		uintptr_t id = offset / _align;
		uintptr_t blocks = _sizes[id];
		//printf("Freed memory at %p, offset: %llx, size: %llu\n", mem, offset, blocks);
		_managedSize -= blocks;
		for (size_t i = id; i < id + blocks && i < _free.size(); i++) {
			_free[i] = true;
			_sizes[i] = 0;
		}
	}

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