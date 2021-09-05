#include "Types.h"
#include "Class.h"

namespace jaether {

	vOBJECT::vOBJECT(vContext* ctx, const V<vClass>& klass) : cls(klass) {
		vClass* kls = cls(ctx);
		V<vCOMMON> Fields = VMAKEGCARRAY(vCOMMON, ctx, (size_t)(kls->_fieldCount));
		// printf("Klass of %p: %s\n", this, klass(ctx)->getName(ctx));
		this->cls = klass;
		for (vUSHORT i = 0; i < kls->_fieldCount; i++) {
			memset(&Fields(ctx, i), 0, sizeof(vCOMMON));
		}
		fieldsRef.a.a = (uintptr_t)Fields.v();
		fieldsRef.type = vTypes::type<vOBJECTREF>();
	}

	vNATIVEARRAY::vNATIVEARRAY(vContext* ctx, vBYTE type, vUINT size) {
		size_t arrSize = (size_t)unitSize(type) * size;
		V<vBYTE> Data = VMAKEGCARRAY(vBYTE, ctx, arrSize);
		this->type = type;
		this->size = size;
		this->cls = 0;
		memset(Data(ctx), 0, arrSize);
		dataRef.a.a = (uintptr_t)Data.v();
		dataRef.type = vTypes::type<vOBJECTREF>();
	}

}