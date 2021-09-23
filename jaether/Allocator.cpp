#define _CRT_SECURE_NO_WARNINGS
#include "Allocator.h"
#include "Types.h"
#include "Pointer.h"

namespace jaether {

	Allocator::Allocator(size_t poolSize, size_t align) {
		const int MOD = (1 << align) - 1;
		poolSize += (align - (poolSize & MOD)) & MOD;
		_pool.resize(poolSize);
		_align = align;
		memset(_pool.data(), 0, _pool.size());
		_free.resize(_pool.size() >> align);
		_sizes.resize(_pool.size() >> align);
		for (size_t i = 0; i < _pool.size() >> align; i++) {
			_free[i] = i > 0;
			_sizes[i] = i == 0 ? 1 : 0;
		}
		_free.back() = false;
		_sizes.back() = 1;
		_firstFree = 1;
	}

	void* Allocator::allocRaw(size_t mem, bool gc) {
		for (int GCI = 0; GCI < 2; GCI++) {
			const int MOD = (1 << _align) - 1;
			if (mem < MOD) mem = static_cast<size_t>(MOD + 1LL);
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
							_sizes[k] = static_cast<unsigned int>(required - m);
						}
						size_t offset = i << _align;
						_firstFree = i + required;
						void* finalAddr = (void*)(_pool.data() + offset);
						if (gc) {
							//printf("Allocated memory at %p, offset: %llx, size: %llu\n", _pool + offset, offset, required);
							_gc.insert(static_cast<unsigned int>(offset));
							//printf("Create GC object: %08X\n", static_cast<unsigned int>(offset));
						}
						return finalAddr;
					} else {
						i += required;
					}
				} else {
					i += static_cast<size_t>(_sizes[i]);
				}
			}
			size_t totalGc = 0;
			for (int GCC = 0; GCC < 2; GCC++) {
				size_t collect = gcCycle();
				totalGc += collect;
#ifdef JVM_DEBUG
				printf("Collected %llu garbage in round %d, total cleanup: %llu, objects: %llu\n", collect, GCC, totalGc, _gc.size());
#endif
			}
		}
		printf("Alloc failed, requested memory: %llu bytes\n", mem);
		throw std::exception("not enough memory");
		return 0;
	}

	size_t Allocator::gcCycle() {
		const size_t step = 1ULL << _align;
		std::set<unsigned int> candidates = _gc;	// hard copy
		for (size_t i = 0, id = 0, j = _free.size(); id < j; i += step, id++) {
			if (_free[id]) continue;
			vCOMMON& v = *(vCOMMON*)((char*)_pool.data() + i);
			// candidate
			if (v.type == vTypes::type<vREF>() && v.a.a <= _pool.size()) {
				//printf("Found reference: %08llu\n", v.a.a);
				auto it = candidates.find(static_cast<unsigned int>(v.a.a));
				if (it != candidates.end()) {	// if found, consider referenced and no longer GC candidate
					//printf("%08llu is referenced at %08llX!\n", v.a.a, i);
					candidates.erase(it);
				}
			}
		}
		// only candidates with zero references should survive
		//printf("GC candidates: %lld, total objects: %lld\n", candidates.size(), _gc.size());
		size_t collected = 0;
		for (auto candidate : candidates) {
			void* va = (void*)static_cast<uintptr_t>(candidate);
			void* ptr = (void*)((char*)_pool.data() + candidate);
			int* TYPE = (int*)ptr;
			bool rem = false;
			if (TYPE[0] == JAETHER_ARR_TAG) {	// treat nativearray in special way
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)va);
				vNATIVEARRAY* parr = arr.ptr(this);
				//printf("Releasing array at #%u, type: %d, size: %u, origin: %d\n", candidate, parr->type, parr->size, parr->x.i);
				collected += arr.ptr(this)->release(this);
				collected += arr.release(this);
				rem = true;
			} else if (TYPE[0] == JAETHER_OBJ_TAG) {	// treat object special way
				//printf("Releasing object at #%u\n", candidate);
				V<vOBJECT> obj((vOBJECT*)va);
				collected += obj.ptr(this)->release(this);
				collected += obj.release(this);
				rem = true;
			}
			if (rem) {
				auto it = _gc.find(candidate);
				if (it != _gc.end()) _gc.erase(it);
			}
		}
		//printf("There are %llu living objects\n", _gc.size());
		return collected;
	}

	size_t Allocator::freeRaw(void* mem) {
		uintptr_t offset = (uintptr_t)mem - (uintptr_t)_pool.data();
		uintptr_t id = offset >> _align;
		uintptr_t blocks = static_cast<uintptr_t>(_sizes[id]);
		//printf("Freed memory at %p, offset: %llx, size: %llu\n", mem, offset, blocks);
		_managedSize -= blocks;
		if (id < _firstFree) _firstFree = id;
		for (size_t i = id; i < id + blocks && i < _free.size(); i++) {
			_free[i] = true;
			_sizes[i] = 0;
		}
		return blocks << _align;
	}

	void Allocator::touchVirtual(void* mem) {
		uintptr_t offset = (uintptr_t)mem;
		unsigned int id = (unsigned int)(offset >> _align);
		_touched.insert(id);
		//_touched.insert(id + 1);
	}

	void Allocator::save(std::ofstream& os) {
		vUINT gcSize = (vUINT)_gc.size();
		vUINT sizesSize = (vUINT)_sizes.size();
		vUINT vsegSize = (vUINT)_touched.size();
		vUINT freeSize = (vUINT)_free.size();
		os.write((const char*)&gcSize, sizeof(gcSize));
		for (auto it : _gc) {
			os.write((const char*)&it, sizeof(it));
		}

		os.write((const char*)&sizesSize, sizeof(sizesSize));
		for (auto it : _sizes) {
			os.write((const char*)&it, sizeof(it));
		}

		os.write((const char*)&vsegSize, sizeof(vsegSize));
		for (auto it : _touched) {
			os.write((const char*)&it, sizeof(it));
		}

		os.write((const char*)&freeSize, sizeof(freeSize));
		os.write((const char*)_free.data(), freeSize);

		os.write((const char*)&_managedSize, sizeof(_managedSize));
		os.write((const char*)&_peakSize, sizeof(_peakSize));
		os.write((const char*)&_cycles, sizeof(_cycles));
		os.write((const char*)&_allocs, sizeof(_allocs));
		os.write((const char*)&_firstFree, sizeof(_firstFree));

		size_t _size = _pool.size();
		os.write((const char*)&_size, sizeof(_size));
		os.write((const char*)_pool.data(), _size);
	}

	void Allocator::load(std::ifstream& is) {
		vUINT gcSize = 0; 
		vUINT sizesSize = 0;
		vUINT vsegSize = 0;
		vUINT freeSize = 0;
		is.read((char*)&gcSize, sizeof(gcSize));
		for (vUINT i = 0; i < gcSize; i++) {
			unsigned int it = 0; is.read((char*)&it, sizeof(it));
			_gc.insert(it);
		}

		is.read((char*)&sizesSize, sizeof(sizesSize));
		if (sizesSize > _sizes.size()) _sizes.resize(sizesSize);
		is.read((char*)_sizes.data(), sizeof(unsigned int) * sizesSize);

		is.read((char*)&vsegSize, sizeof(vsegSize));
		for (vUINT i = 0; i < vsegSize; i++) {
			unsigned int it = 0; is.read((char*)&it, sizeof(it));
			_touched.insert(it);
		}

		is.read((char*)&freeSize, sizeof(freeSize));
		if (freeSize > _free.size()) _free.resize(freeSize);
		is.read((char*)_free.data(), freeSize);


		is.read((char*)&_managedSize, sizeof(_managedSize));
		is.read((char*)&_peakSize, sizeof(_peakSize));
		is.read((char*)&_cycles, sizeof(_cycles));
		is.read((char*)&_allocs, sizeof(_allocs));
		is.read((char*)&_firstFree, sizeof(_firstFree));

		size_t sz = 0;
		is.read((char*)&sz, sizeof(sz));
		if (sz > _pool.size()) {
			_pool.resize(sz);
		}
		is.read((char*)_pool.data(), sz);
	}
}