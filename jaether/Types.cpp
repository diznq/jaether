#include "Types.h"
#include "Class.h"

vOBJECT::vOBJECT(vContext* ctx, const V<vClass>& klass) : cls(klass) {
	fields = VMAKEARRAY(vCOMMON, ctx, cls.Ptr(ctx)->_fieldCount);
	for (vUSHORT i = 0; i < cls.Ptr(ctx)->_fieldCount; i++) {
		memset(&fields[VCtxIdx{ ctx, i }], 0, sizeof(vCOMMON));
	}
}