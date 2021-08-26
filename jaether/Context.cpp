#include "Context.h"
#include "smmalloc.h"
#include "sha256.h"
vContext::vContext(size_t mem, bool secure) {
	sm_allocator space = _sm_allocator_create(1, mem);
	_pool = (void*)space;
	_offset = (void*)space->GetBucket(0).pData;
	_poolSize = mem;
	_hashCtx = new SHA256_CTX;
	_secure = secure;
	sha256_init((SHA256_CTX*)_hashCtx);
}

vContext::~vContext() {
	if(_pool)
		_sm_allocator_destroy((sm_allocator)_pool);
	_pool = 0;
	_offset = 0;
	delete _hashCtx;
}

void* vContext::Alloc(size_t size) {
	sm_allocator space = (sm_allocator)_pool;
	char* ptr = (char*)space->Alloc(size, 16); // new char[size];
	ptr -= Offset();
	printf("Alloc: %p, %lld\n", ptr, size);
	return ptr;
}

void vContext::Free(void* mem, bool arr) {
	sm_allocator space = (sm_allocator)_pool;
	char* ptr = (char*)mem;
	space->Free(ptr);
}

void vContext::OnInstruction() {
	if (!_secure) return;
	SHA256_CTX* sha = (SHA256_CTX*)_hashCtx;
	void* start = _offset;
	sha256_update(sha, (const BYTE*)start, _poolSize);
}

void vContext::GetSignature(unsigned char* out) {
	SHA256_CTX* sha = (SHA256_CTX*)_hashCtx;
	sha256_final(sha, out);
}