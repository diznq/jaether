#include "Context.h"
#include "sha256.h"

namespace jaether {

	vContext::vContext(size_t mem, bool secure) {
		_alloc = new Allocator(mem, 16);
		_hashCtx = new SHA256_CTX;
		_secure = secure;
		sha256_init((SHA256_CTX*)_hashCtx);
	}

	vContext::~vContext() {
		delete _alloc;
		delete _hashCtx;
	}

	void* vContext::Alloc(size_t size) {
		char* ptr = (char*)_alloc->Alloc(size); // new char[size];
		ptr -= Offset();
		return ptr;
	}

	void vContext::Free(void* mem, bool arr) {
		char* ptr = (char*)mem;
		_alloc->Free(ptr);
	}

	void vContext::OnInstruction() {
		_ops++;
		if (!_secure) return;
		SHA256_CTX* sha = (SHA256_CTX*)_hashCtx;
		void* start = _alloc->GetBase();
		sha256_update(sha, (const BYTE*)start, _alloc->GetSize());
	}

	void vContext::GetSignature(unsigned char* out) {
		SHA256_CTX* sha = (SHA256_CTX*)_hashCtx;
		sha256_final(sha, out);
	}

}