#pragma once
#include <utility>
#include <stdint.h>
#include <map>
#include <string>
#include <any>
#include <functional>
#include <stdexcept>
#include "Allocator.h"

//#define JVM_DEBUG
#ifdef JVM_DEBUG
#define RPRINTF(fmt, ...) fprintf(stdout, "%*s" fmt, nesting + (int)frames.size(), "", __VA_ARGS__)
#define DPRINTF(fmt, ...) fprintf(stdout, fmt, __VA_ARGS__)
#else
#define RPRINTF(...) /* */
#define DPRINTF(...) /* */
#endif

namespace jaether {

	template<class T> class V;
	class vClass;

	struct vContext {
	private:
		bool _secure = false;
		bool _fullInit = false;
		void* _hashContext = 0;
		size_t _ops = 0;
		Allocator* _alloc = 0;
		std::vector<std::string> _propsPairs;
		std::map<std::string, std::string> _propsMap;
		std::vector<std::string> _propsIndices;
		
		std::map<std::string, vClass*> _classes;
		std::map<std::string, std::any> _storage;

		void writeString(std::ostream& os, const std::string& str);
		std::string readString(std::istream& is);
		template<class T> void writeAny(std::ostream& os, T thing) {
			os.write((const char*)&thing, sizeof(T));
		}
		template<class T> T readAny(std::istream& is) {
			T thing;
			is.read((char*)&thing, sizeof(T));
			return thing;
		}
	public:
		friend class vCPU;
		vContext(Allocator* alloc, bool fullInit = true, bool secure = false);
		~vContext();
		void* alloc(size_t mem, bool gc = false);
		size_t free(void* mem, bool arr = false);

		void save(const char* path);
		void load(const char* path);

		void* setMemory(void* VirtDest, int value, size_t len) {
			return _alloc->setMemory(VirtDest, value, len);
		}

		void* copyMemory(void* VirtDest, const void* VirtSrc, size_t len, bool srcReal = false) const {
			return _alloc->copyMemory(VirtDest, VirtSrc, len, srcReal);
		}

		void* moveMemory(void* VirtDest, const void* VirtSrc, size_t len, bool srcReal = false) const {
			return _alloc->moveMemory(VirtDest, VirtSrc, len, srcReal);
		}

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


		template<typename T, typename... Args>
		T* allocTypeGc(Args&&... args) {
			char* mem = ((char*)alloc(sizeof(T), true));
			new ((T*)(mem + offset())) T(
				std::forward<Args>(args)...
			);
			return (T*)mem;
		}
		template<typename T>
		T* allocArrayGc(size_t size) {
			return (T*)alloc(sizeof(T) * size, true);
		}

		template<typename T>
		size_t freeType(T* obj, bool arr = false) {
			return free((void*)obj, arr);
		}

		void onInstruction();
		void getSignature(unsigned char* buffer);

		uintptr_t offset() const;

		void* getBase() const { return _alloc->getBase(); }

		Allocator* getAllocator() const {
			return _alloc;
		}

		std::map<std::string, vClass*>& getClasses() {
			return _classes;
		}

		size_t ops() const {
			return _ops;
		}

		void touchVirtual(void* memory);

		inline uintptr_t resolve(uintptr_t addr) const {
			return _alloc->resolve(addr);
		}

		template<class T>
		T& getObject(vContext* ctx, const std::string& key, std::function<T(vContext*)> orElse = nullptr) {
			auto it = ctx->_storage.find(key);
			if (it != ctx->_storage.end()) {
				return std::any_cast<T&>(it->second);
			}
			if (!orElse) throw std::runtime_error("or else callback not found");
			return std::any_cast<T&>(ctx->_storage[key] = orElse(ctx));
		}
	};

}