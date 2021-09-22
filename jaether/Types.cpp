#include "Types.h"
#include "Class.h"

namespace jaether {

	vOBJECT::vOBJECT(vContext* ctx, const V<vClass>& klass) : cls(klass) {
		vClass* kls = cls(ctx);
		size_t attrCount = 0;
		for (vUSHORT i = 0; i < klass(ctx)->_fieldCount; i++) {
			vFIELD& fld = klass(ctx)->_fields(ctx, (size_t)i);
			if (fld.access & 8) continue;
			attrCount++;
		}
		V<vCOMMON> Fields = VMAKEARRAY(vCOMMON, ctx, attrCount);
		// printf("Klass of %p: %s\n", this, klass(ctx)->getName(ctx));
		this->cls = klass;
		for (vUSHORT i = 0; i < attrCount; i++) {
			memset(&Fields(ctx, i), 0, sizeof(vCOMMON));
		}
		fieldsObj = Fields;
	}

	vNATIVEARRAY::vNATIVEARRAY(vContext* ctx, vBYTE type, vUINT size) {
		size_t arrSize = (size_t)unitSize(type) * size;
		V<vBYTE> Data = VMAKEARRAY(vBYTE, ctx, arrSize);
		this->type = type;
		this->size = size;
		this->cls = 0;
		memset(Data(ctx), 0, arrSize);
		dataObj = Data;
	}

	std::string vFIELD::getName(vContext* ctx) {
		auto szName = cls(ctx)->toString(ctx, name)(ctx)->s(ctx);
		return std::string((const char*)szName);
	}

	std::string vFIELD::getDesc(vContext* ctx) {
		auto szName = cls(ctx)->toString(ctx, desc)(ctx)->s(ctx);
		return std::string((const char*)szName);
	}

}