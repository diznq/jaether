#include "Types.h"
#include "Class.h"

namespace jaether {

	vOBJECT::vOBJECT(vContext* ctx, const V<vClass>& klass) : cls(klass) {
		fields = VMAKEARRAY(vCOMMON, ctx, cls.Ptr(ctx)->_fieldCount);
		for (vUSHORT i = 0; i < cls.Ptr(ctx)->_fieldCount; i++) {
			memset(&fields[VCtxIdx{ ctx, i }], 0, sizeof(vCOMMON));
		}
	}

	vNATIVEARRAY::vNATIVEARRAY(vContext* ctx, vBYTE type, vUINT size) {
		size_t arrSize = (size_t)unitSize(type) * size;
		data = VMAKEARRAY(vBYTE, ctx, arrSize);
		this->type = type;
		this->size = size;
		this->cls = 0;
		memset(data.Ptr(ctx), 0, arrSize);
	}

}