#include "Context.h"
#include "sha256.h"
#include <fstream>

namespace jaether {

	vContext::vContext(Allocator* alloc, bool fullInit, bool secure) {
		_alloc = alloc;
		_fullInit = fullInit;
		_hashContext = new SHA256_CTX;
		_secure = secure;
		std::ifstream props("SystemProperties.txt");
		std::string line;
		while (std::getline(props, line)) {
			auto pos = line.find('=');
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			_propsPairs.push_back(key);
			_propsPairs.push_back(value);
			_propsMap[key] = value;
		}
		props.close();
		std::ifstream indices("SystemIndices.txt");
		while (std::getline(indices, line)) {
			_propsIndices.push_back(line);
		}
		indices.close();
		sha256_init((SHA256_CTX*)_hashContext);
	}

	vContext::~vContext() {
		delete _alloc;
		delete _hashContext;
	}

	void* vContext::alloc(size_t size, bool gc) {
		char* ptr = (char*)_alloc->allocRaw(size, gc); // new char[size];
		ptr -= offset();
		return ptr;
	}

	size_t vContext::free(void* mem, bool arr) {
		char* ptr = (char*)mem;
		return _alloc->freeRaw(ptr);
	}

	void vContext::onInstruction() {
		_ops++;
		if (!_secure) return;
		SHA256_CTX* sha = (SHA256_CTX*)_hashContext;
		void* start = _alloc->getBase();
		size_t align = _alloc->getAlignment();
		auto& segments = _alloc->getTouchedVSegments();
		const size_t segmentSize = (1ULL << align) + ((1ULL << align) >> 1);
		for (auto id : segments) {
			sha256_update(sha, (const BYTE*)&id, sizeof(id));
			unsigned int offset = id << align;
			sha256_update(sha, ((const BYTE*)start) + offset, segmentSize);
		}
	}

	void vContext::getSignature(unsigned char* out) {
		SHA256_CTX* sha = (SHA256_CTX*)_hashContext;
		sha256_final(sha, out);
	}

	void vContext::touchVirtual(void* memory) {
		if (!_secure) return;
		_alloc->touchVirtual(memory);
	}

	uintptr_t vContext::offset() const {
#ifdef _DEBUG
		return (uintptr_t)_alloc->getBase();
#else
		return (uintptr_t)_alloc->getBase();
#endif
	}

}