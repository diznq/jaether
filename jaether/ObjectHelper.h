#pragma once
#include "Types.h"
#include "Class.h"
#include "Context.h"
#include <exception>

namespace jaether {

	class FieldNotFoundException : public std::exception {
	public:
		FieldNotFoundException() : std::exception("Field not found") {
			// ...
		}
	};

	class JObject {
	private:
		vContext* _ctx = 0;
		V<vOBJECT> _obj;
	public:
		JObject(vContext* ctx, V<vOBJECTREF> objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref.Ptr(ctx)->r.a);
		}
		JObject(vContext* ctx, vOBJECTREF objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref.r.a);
		}
		JObject(vContext* ctx, vCOMMON objref) : _ctx(ctx) {
			_obj = V<vOBJECT>((vOBJECT*)objref.objref.r.a);
		}
		JObject(vContext* ctx, V<vOBJECT> obj) : _ctx(ctx), _obj(obj) {

		}
		vCOMMON& operator[](const vUSHORT idx) const {
			vCOMMON* ptr = _obj.Ptr(_ctx)->cls.Ptr(_ctx)->getObjField(_ctx, _obj, idx);
			if (!ptr) throw FieldNotFoundException();
			return *ptr;
		}
		vCOMMON& operator[](const char* idx) const {
			vCOMMON* ptr = _obj.Ptr(_ctx)->cls.Ptr(_ctx)->getObjField(_ctx, _obj, idx);
			if (!ptr) throw FieldNotFoundException();
			return *ptr;
		}
		void* Ptr() const {
			return _obj.Ptr(_ctx);
		}
		operator bool() const {
			return _obj.IsValid();
		}
	};


	template<class T>
	class JArray {
	private:
		vContext* _ctx = 0;
		V<vNATIVEARRAY> _obj;
	public:
		JArray(vContext* ctx, V<vOBJECTREF> objref) : _ctx(ctx) {
			_obj = V<vNATIVEARRAY>((vNATIVEARRAY*)objref.Ptr(ctx)->r.a);
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
			return _obj.Ptr(_ctx)->get<T>(_ctx, idx);
		}
		size_t length() const {
			return _obj.Ptr(_ctx)->size;
		}
		vBYTE type() const {
			return _obj.Ptr(_ctx)->type;
		}
		vUSHORT cls() const {
			return _obj.Ptr(_ctx)->cls;
		}
		T* data() const {
			return (T*)_obj.Ptr(_ctx)->data.Ptr(_ctx);
		}
		void* Ptr() const {
			return _obj.Ptr(_ctx);
		}
		operator bool() const {
			return _obj.IsValid();
		}
	};
}