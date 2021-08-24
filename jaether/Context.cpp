#include "Context.h"

void* vContext::Alloc(size_t size) {
	char* ptr = new char[size];
	ptr -= Offset();
	return ptr;
}

void vContext::Free(void* mem, bool arr) {
	char* ptr = (char*)mem;
	delete[] ptr;
}