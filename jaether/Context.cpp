#include "Context.h"
#include "sha256.h"

namespace jaether {

	vContext::vContext(Allocator* alloc, bool secure) {
		_alloc = alloc;
		_hashContext = new SHA256_CTX;
		_secure = secure;
		sha256_init((SHA256_CTX*)_hashContext);
	}

	vContext::~vContext() {
		delete _alloc;
		delete _hashContext;
	}

	void* vContext::alloc(size_t size) {
		char* ptr = (char*)_alloc->Alloc(size); // new char[size];
		ptr -= offset();
		return ptr;
	}

	void vContext::free(void* mem, bool arr) {
		char* ptr = (char*)mem;
		_alloc->Free(ptr);
	}

	void vContext::onInstruction() {
		_ops++;
		if (!_secure) return;
		SHA256_CTX* sha = (SHA256_CTX*)_hashContext;
		void* start = _alloc->GetBase();
		sha256_update(sha, (const BYTE*)start, _alloc->GetSize());
	}

	void vContext::getSignature(unsigned char* out) {
		SHA256_CTX* sha = (SHA256_CTX*)_hashContext;
		sha256_final(sha, out);
	}

	uintptr_t vContext::offset() const {
#ifdef _DEBUG
		return (uintptr_t)_alloc->GetBase();
#else
		return (uintptr_t)_alloc->GetBase();
#endif
	}

}