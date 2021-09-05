#pragma once
#include "Types.h"
#include "Class.h"
#include "Context.h"
#include <exception>
#include <string>

namespace jaether {

	class FieldNotFoundException : public std::exception {
	public:
		FieldNotFoundException() : std::exception("Field not found") {
			// ...
		}
	};

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

		const int TAG() const {
			return _obj(_ctx)->TAG;
		}

		void checkTag() const {
			if (!_obj.isValid()) {
				throw std::runtime_error("invalid object reference");
			} else if(TAG() != JAETHER_OBJ_TAG) {
				printf("invalid tag detected: %d\n", TAG());
				throw std::runtime_error("invalid object tag: " + std::to_string(TAG()));
			}
		}

		vCOMMON& operator[](const char* idx) const {
			vCOMMON* ptr = _obj(_ctx)->cls(_ctx)->getObjField(_ctx, _obj, idx);
			if (!ptr) throw FieldNotFoundException();
			return *ptr;
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
			return _obj(_ctx)->fields;
		}

		vCOMMON& x() const {
			return _obj(_ctx)->x;
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
			return (T*)_obj(_ctx)->data(_ctx);
		}
		void* ptr() const {
			return _obj(_ctx);
		}
		operator bool() const {
			return _obj.isValid();
		}
	};

	class JString : public JObject {
	public:
		using JObject::JObject;
		std::string str() const;
	};
}