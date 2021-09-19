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