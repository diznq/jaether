#pragma once
#include "Types.h"
#include "Class.h"
#include "Context.h"
#include <exception>
#include <string>

namespace jaether {

	class JObject {
	protected:
		vContext* _ctx = 0;
		V<vOBJECT> _obj;
	public:
		JObject(vContext* ctx, V<vOBJECTREF> objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref(ctx)->r.a);
			checkTag();
		}
		JObject(vContext* ctx, vOBJECTREF objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref.r.a);
			checkTag();
		}
		JObject(vContext* ctx, vCOMMON objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref.objref.r.a);
			checkTag();
		}
		JObject(vContext* ctx, V<vOBJECT> obj) : _ctx(ctx), _obj(obj) {

		}

		JObject(vContext* ctx, V<vClass> cls, bool gc = true);
	protected:
		JObject(vContext* ctx) : _ctx(ctx) {}
	public:

		bool isArray() const {
			return TAG() == JAETHER_ARR_TAG;
		}

		const int TAG() const {
			return _obj(_ctx)->TAG;
		}

		void checkTag() const {
			if (!_obj.isValid()) {
				throw std::runtime_error("invalid object reference");
			} else if(TAG() != JAETHER_OBJ_TAG && TAG() != JAETHER_ARR_TAG) {
				printf("obj: %llu\n", (vULONG)_obj.v());
				printf("invalid tag detected: %x\n", TAG());
				throw std::runtime_error("invalid object tag: " + std::to_string(TAG()));
			}
		}

		vCOMMON& operator[](const char* idx) const {
			if (isArray()) throw std::runtime_error("attempt to access member of an array");
			vCOMMON* ptr = _obj(_ctx)->cls(_ctx)->getObjField(_ctx, _obj, idx);
			if (!ptr) throw std::runtime_error("field not found");
			return *ptr;
		}

		vCOMMON& operator[](size_t idx) {
			if (isArray()) {
				auto arr = asArray();
				idx &= 0x7FFFFFFF;
				//idx /= arr(_ctx)->unitSize(arr(_ctx)->type);
				//printf("Array type: %d, idx: %llu, size: %lu\n", arr(_ctx)->type, idx, arr(_ctx)->size);
				if (idx >= arr(_ctx)->size) throw std::runtime_error("array out of bound index");
				return arr(_ctx)->get<vCOMMON>(_ctx, idx);
			}
			if (idx >= _obj(_ctx)->cls(_ctx)->_fieldCount)
				throw std::runtime_error("invalid field index");
			return _obj(_ctx)->fields()(_ctx, idx);
		}

		V<vNATIVEARRAY> asArray() {
			return V<vNATIVEARRAY>((vNATIVEARRAY*)_obj.v());
		}

		void* ptr() const {
			return _obj(_ctx);
		}

		operator bool() const {
			return _obj.isValid();
		}

		V<vClass> getClass() const {
			V<vClass> klass = _obj(_ctx)->cls;
			const int tag = klass(_ctx)->TAG;
			if (tag != JAETHER_CLASS_TAG) throw std::runtime_error("invalid class tag: " + std::to_string(tag));
			return klass;
		}

		V<vCOMMON> fields() const {
			return _obj(_ctx)->fields();
		}

		vCOMMON& x() {
			return _obj(_ctx)->x;
		}

		vOBJECTREF ref() const {
			vOBJECTREF objr; objr.r.a = (vULONG)_obj.v();
			return objr;
		}

		operator vOBJECTREF() const {
			return ref();
		}
	};


	template<class T>
	class JArray {
	protected:
		vContext* _ctx = 0;
		V<vNATIVEARRAY> _obj;
	public:
		JArray(vContext* ctx, V<vOBJECTREF> objref) : _ctx(ctx) {
			_obj = V<vNATIVEARRAY>((vNATIVEARRAY*)objref(ctx)->r.a);
		}
		JArray(vContext* ctx, vOBJECTREF objref) : _ctx(ctx) {
			_obj = V<vNATIVEARRAY>((vNATIVEARRAY*)objref.r.a);
		}
		JArray(vContext* ctx, vCOMMON objref) : _ctx(ctx) {
			_obj = V<vNATIVEARRAY>((vNATIVEARRAY*)objref.objref.r.a);
		}
		JArray(vContext* ctx, V<vNATIVEARRAY> obj) : _ctx(ctx), _obj(obj) {

		}
		JArray(vContext* ctx, size_t size, vUINT type = 1, bool gc = true) : _ctx(ctx) {
			if (gc) {
				_obj = VMAKEGC(vNATIVEARRAY, ctx, ctx, type, (vUINT)size);
			} else {
				_obj = VMAKE(vNATIVEARRAY, ctx, ctx, type, (vUINT)size);
			}
		}
		JArray clone() {
			return JArray(_ctx, _obj(_ctx)->clone(_ctx));
		}
		T& operator[](const size_t idx) const {
			return _obj(_ctx)->get<T>(_ctx, idx);
		}
		size_t length() const {
			return _obj(_ctx)->size;
		}
		vBYTE type() const {
			return _obj(_ctx)->type;
		}
		vUSHORT cls() const {
			return _obj(_ctx)->cls;
		}
		T* data() const {
			return (T*)_obj(_ctx)->data()(_ctx);
		}
		void* ptr() const {
			return _obj(_ctx);
		}
		vOBJECTREF ref() const {
			vOBJECTREF objr; objr.r.a = (vULONG)_obj.v();
			return objr;
		}
		operator vOBJECTREF() const {
			return ref();
		}
		operator bool() const {
			return _obj.isValid();
		}
	};

	class JString : public JObject {
	public:
		using JObject::JObject;
		JString(vContext* ctx, const std::wstring& wstr, bool gc = true, int source = 0);
		std::string str() const;
	};

	class JClass {
	private:
		vContext* _ctx = 0;
		V<vClass> _klass;
	public:
		JClass(vContext* ctx, V<vClass> kls) : _ctx(ctx), _klass(kls) {

		}
		
		vFIELD& operator[](const char* name) {
			vFIELD* fld = _klass(_ctx)->getField(_ctx, name);
			if (!fld) throw std::runtime_error("static field not found");
			return *fld;
		}
		
		const char* getName() {
			return _klass(_ctx)->getName(_ctx);
		}
		
		const char* getSuperName() {
			return _klass(_ctx)->getSuperName(_ctx);
		}

		bool instanceOf(JClass& tgt) {
			return _klass(_ctx)->instanceOf(_ctx, tgt._klass);
		}

		vClass* v() const {
			return _klass.v();
		}

		V<vClass> raw() {
			return _klass;
		}

		vOBJECTREF& getJavaClass(bool gc = false) {
			return _klass(_ctx)->getJavaClass(_ctx, gc);
		}
	};
}