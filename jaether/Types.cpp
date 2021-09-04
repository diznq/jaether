#include "Types.h"
#include "Class.h"

namespace jaether {

	vOBJECT::vOBJECT(vContext* ctx, const V<vClass>& klass) : cls(klass) {
		vClass* kls = cls(ctx);
		fields = VMAKEARRAY(vCOMMON, ctx, kls->_fieldCount + kls->_fieldOffset);
		// printf("Klass of %p: %s\n", this, klass(ctx)->getName(ctx));
		this->cls = klass;
		for (vUSHORT i = 0; i < kls->_fieldCount + kls->_fieldOffset; i++) {
			memset(&fields(ctx, i), 0, sizeof(vCOMMON));
		}
	}

	vNATIVEARRAY::vNATIVEARRAY(vContext* ctx, vBYTE type, vUINT size) {
		size_t arrSize = (size_t)unitSize(type) * size;
		data = VMAKEARRAY(vBYTE, ctx, arrSize);
		this->type = type;
		this->size = size;
		this->cls = 0;
		memset(data(ctx), 0, arrSize);
	}

}