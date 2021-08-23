#include "Types.h"
#include "Class.h"

vOBJECT::vOBJECT(V<vClass> cls) {
	fields = VMAKEARRAY(vCOMMON, cls->_fieldCount);
}