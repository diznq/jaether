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
			_obj = V<vOBJECT>((vOBJECT*)objref.ptr(ctx)->r.a);
		}
		JObject(vContext* ctx, vOBJECTREF objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref.r.a);
		}
		JObject(vContext* ctx, vCOMMON objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref.objref.r.a);
		}
		JObject(vContext* ctx, V<vOBJECT> obj) : _ctx(ctx), _obj(obj) {

		}
		vCOMMON& operator[](const char* idx) const {
			vCOMMON* ptr = _obj.ptr(_ctx)->cls.ptr(_ctx)->getObjField(_ctx, _obj, idx);
			if (!ptr) throw FieldNotFoundException();
			return *ptr;
		}
		void* Ptr() const {
			return _obj.ptr(_ctx);
		}
		operator bool() const {
			return _obj.isValid();
		}
		V<vClass> GetClass() const {
			return _obj.ptr(_ctx)->cls;
		}
	};


	template<class T>
	class JArray {
	protected:
		vContext* _ctx = 0;
		V<vNATIVEARRAY> _obj;
	public:
		JArray(vContext* ctx, V<vOBJECTREF> objref) : _ctx(ctx) {
			_obj = V<vNATIVEARRAY>((vNATIVEARRAY*)objref.ptr(ctx)->r.a);
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
			return _obj.ptr(_ctx)->get<T>(_ctx, idx);
		}
		size_t length() const {
			return _obj.ptr(_ctx)->size;
		}
		vBYTE type() const {
			return _obj.ptr(_ctx)->type;
		}
		vUSHORT cls() const {
			return _obj.ptr(_ctx)->cls;
		}
		T* data() const {
			return (T*)_obj.ptr(_ctx)->data.ptr(_ctx);
		}
		void* Ptr() const {
			return _obj.ptr(_ctx);
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