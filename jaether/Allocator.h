#pragma once
#include <vector>
#include <string.h>

class Allocator {
	unsigned char *_pool;
	size_t _size;
	std::vector<bool> _free;
	std::vector<size_t> _sizes;
	size_t _align;
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
		//printf("Request alloc %llu bytes\n", mem);
		size_t required = mem / _align + 1;
		if (mem % _align == 0) required--;
		for (size_t i = 0, j = _free.size(); i < j; i++) {
			bool suitable = false;
			if (_free[i]) {
				size_t blocks = required - 1;
				for (size_t k = i + 1, l = i + required; k < l && k < j; k++) {
					if (!_free[k]) break;
					blocks--;
				}
				//printf("Blocks found at %lld: %lld\n", i, blocks);
				if (blocks == 0) {
					_sizes[i] = required;
					for (size_t k = i, l = i + required; k < l && k < j; k++) {
						_free[k] = false;
					}
					size_t offset = i * _align;
					//printf("Allocated memory at %p, offset: %llx, size: %llu\n", _pool + offset, offset, required);
					return (void*)(_pool + offset);
				} else {
					i += required;
				}
			}
		}
		return 0;
	}

	void Free(void* mem) {
		uintptr_t offset = (uintptr_t)mem - (uintptr_t)_pool;
		uintptr_t id = offset / _align;
		uintptr_t blocks = _sizes[id];
		//printf("Freed memory at %p, offset: %llx, size: %llu\n", mem, offset, blocks);

		for (size_t i = id; i < id + blocks && i < _free.size(); i++) {
			_free[id + i] = true;
			_sizes[id + i] = 0;
		}
	}
};