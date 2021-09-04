#pragma once
#include <utility>
#include <stdint.h>
#include <map>
#include <string>
#include "Allocator.h"

#define JVM_DEBUG
#ifdef JVM_DEBUG
#define RPRINTF(fmt, ...) fprintf(stdout, "%*s" fmt, nesting + (int)frames.size(), "", __VA_ARGS__)
#define DPRINTF(fmt, ...) fprintf(stdout, fmt, __VA_ARGS__)
#else
#define RPRINTF(...) /* */
#define DPRINTF(...) /* */
#endif

namespace jaether {

	class vClass;

	struct vDummy {
		vDummy(int i) {}
	};

	struct vContext {
	private:
		bool _secure = false;
		void* _hashContext = 0;
		size_t _ops = 0;
		Allocator* _alloc = 0;
		std::map<std::string, vClass*> _classes;
	public:
		vContext(Allocator* alloc, bool secure = false);
		~vContext();
		void* alloc(size_t mem);
		void free(void* mem, bool arr = false);
		template<typename T, typename... Args>
		T* allocType(Args&&... args) {
			char* mem = ((char*)alloc(sizeof(T)));
			new ((T*)(mem + offset())) T(
				std::forward<Args>(args)...
			);
			return (T*)mem;
		}
		template<typename T>
		T* allocArray(size_t size) {
			return (T*)alloc(sizeof(T) * size);
		}
		template<typename T>
		void freeType(T* obj, bool arr = false) {
			free((void*)obj, arr);
		}

		void onInstruction();
		void getSignature(unsigned char* buffer);

		uintptr_t offset() const;

		Allocator* getAllocator() const {
			return _alloc;
		}

		std::map<std::string, vClass*>& getClasses() {
			return _classes;
		}

		size_t ops() const {
			return _ops;
		}
	};

}