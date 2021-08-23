#include "Types.h"
#include "Class.h"

vOBJECT::vOBJECT(const V<vClass>& klass) : cls(klass) {
	fields = VMAKEARRAY(vCOMMON, cls->_fieldCount);
	for (vUSHORT i = 0; i < cls->_fieldCount; i++) {
		memset(&fields[i], 0, sizeof(vCOMMON));
	}
}