#include "Context.h"
#include "sha256.h"
vContext::vContext(size_t mem, bool secure) {
	_alloc = new Allocator(mem, 16);
	_pool = (void*)_alloc;
	_offset = (void*)_alloc->GetBase();
	_poolSize = mem;
	_hashCtx = new SHA256_CTX;
	_secure = secure;
	sha256_init((SHA256_CTX*)_hashCtx);
}

vContext::~vContext() {
	delete _alloc;
	_pool = 0;
	_offset = 0;
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
	void* start = _offset;
	sha256_update(sha, (const BYTE*)start, _poolSize);
}

void vContext::GetSignature(unsigned char* out) {
	SHA256_CTX* sha = (SHA256_CTX*)_hashCtx;
	sha256_final(sha, out);
}