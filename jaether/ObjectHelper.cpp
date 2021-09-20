#include "ObjectHelper.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace jaether {

	JObject::JObject(vContext* ctx, V<vClass> cls, bool gc) {
		_ctx = ctx;
		if (gc) {
			_obj = VMAKEGC(vOBJECT, ctx, ctx, cls);
		} else {
			_obj = VMAKE(vOBJECT, ctx, ctx, cls);
		}
	}

	JString::JString(vContext* ctx, const std::wstring& text, bool gc, int source) : JObject(ctx) {
		const bool properStrings = false;
		const bool cvtEndian = true;
		auto& classes = ctx->getClasses();
		auto it = classes.find("java/lang/String");
		V<vNATIVEARRAY> value;
		V<vClass> klass;
		if (it != classes.end()) {
			klass = V<vClass>(it->second);
		} else {
			throw std::runtime_error("string class not loaded");
		}
		if (gc) {
			value = VMAKEGC(vNATIVEARRAY, ctx, ctx, 5, (vUINT)(text.length())); // 5 = JCHAR
			_obj = VMAKEGC(vOBJECT, ctx, ctx, klass);
		} else {
			value = VMAKE(vNATIVEARRAY, ctx, ctx, 5, (vUINT)(text.length())); // 5 = JCHAR
			_obj = VMAKE(vOBJECT, ctx, ctx, klass);
		}
		_obj(ctx)->x.set<vLONG>(1090);

		for (size_t i = 0, j = text.length(); i < j; i++) {
			value(ctx)->set<vJCHAR>(ctx, i, text[i]);
		}
		value(ctx)->x.set<vLONG>(1089);
		value(ctx)->type = 8; // 8 = BYTE
		value(ctx)->size <<= 1;
		
		JObject object(ctx, _obj);
		JArray<vBYTE> arr(ctx, value);

		object["value"].set<vOBJECTREF>(Ref(value));
		object["coder"].set<vINT>(1);
	}

	std::string JString::str() const {
		vBYTE coder = 1;
		try {
			coder = (*this)["coder"].b;
		} catch (FieldNotFoundException&) {
			//...
		}
		try {
			vCOMMON& value = (*this)["value"];
			JArray<char> arr(_ctx, value);
			if (coder == 0) {
				return std::string(arr.data(), arr.data() + arr.length());
			} else {			// UTF16
#ifdef _WIN32
				int size = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)arr.data(), (int)(arr.length() >> coder), NULL, 0, NULL, NULL);
				char* buffer = new char[size];
				memset(buffer, 0, size);
				WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)arr.data(), -1, buffer, size, NULL, NULL);
				std::string str(buffer, buffer + size);
				delete[] buffer;
				return str;
#else
				return "unresolved";
#endif
			}
		} catch (FieldNotFoundException&) {
			return "<invalid string>";
		}
	}
}