#include "Context.h"
#include "sha256.h"
#include "Types.h"
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

	void vContext::writeString(std::ostream& os, const std::string& str) {
		vUINT len = (vUINT)str.length();
		os.write((const char*)&len, sizeof(len));
		os.write(str.data(), (size_t)len);
	}

	std::string vContext::readString(std::istream& is) {
		vUINT len = 0;
		is.read((char*)&len, sizeof(len));
		char* buff = new char[len];
		is.read(buff, len);
		std::string str(buff, buff + len);
		delete[] buff;
		return str;
	}

	void vContext::save(const char* path) {
		std::ofstream f(path, std::ios::binary);
		if (!f) {
			throw std::runtime_error("couldn't open file for save");
		}
		writeAny(f, _ops);
		//writeAny(os)
		writeAny(f, _classes.size());
		//printf("Classes size: %llu\n", _classes.size());
		for (auto it : _classes) {
			writeString(f, it.first);
			writeAny(f, (vUINT)(uintptr_t)it.second);
			//printf("Class %s is at %p\n", it.first.c_str(), it.second);
		}
		writeAny(f, _storage.size());
		//printf("Storage size: %llu\n", _storage.size());
		for (auto it : _storage) {
			writeString(f, it.first);
			writeAny(f, std::any_cast<vOBJECTREF>(it.second));
			//printf("Object %s is at %p\n", it.first.c_str(), std::any_cast<vOBJECTREF>(it.second).r.a);
		}
		_alloc->save(f);
	}

	void vContext::load(const char* path) {
		std::ifstream f(path, std::ios::binary);
		if (!f) {
			throw std::runtime_error("couldn't open file for save");
		}
		_ops = readAny<size_t>(f);
		size_t sz = readAny<size_t>(f);
		for (size_t i = 0; i < sz; i++) {
			std::string key = readString(f);
			vClass* val = (vClass*)(uintptr_t)readAny<vUINT>(f);
			//printf("Class %s is at %p\n", key.c_str(), val);
			_classes[key] = val;
		}
		sz = readAny<size_t>(f);
		for (size_t i = 0; i < sz; i++) {
			std::string key = readString(f);
			vOBJECTREF val = readAny<vOBJECTREF>(f);
			_storage[key] = val;
			//printf("Object %s is at %p\n", key.c_str(), val.r.a);
		}
		_alloc->load(f);
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