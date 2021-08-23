#pragma once
#include <string.h>
#include "Pointer.h"

typedef int8_t		vCHAR;
typedef wchar_t		vJCHAR;
typedef int16_t		vSHORT;
typedef int32_t		vINT;
typedef int64_t		vLONG;
typedef uint8_t		vBYTE;
typedef uint16_t	vUSHORT;
typedef uint32_t	vUINT;
typedef uint64_t	vULONG;
typedef uint32_t	vCAT1;
typedef uint64_t	vCAT2;
typedef float vFLOAT;
typedef double vDOUBLE;

class vClass;

enum vClassTag {
	vCT_UTF8 = 1,
	vCT_INT = 3,
	vCT_DOUBLE = 6,
	vCT_CLASS = 7,
	vCT_STRING = 8,
	vCT_FIELDREF = 9,
	vCT_METHODREF = 10,
	vCT_NAMEANDTYPE = 12
};

struct vREF {
	vULONG a;
};

struct vCLASS {
	vUSHORT clsIndex;
};

struct vMETHODREF {
	vUSHORT clsIndex;
	vUSHORT nameIndex;
};

struct vNAMEANDTYPE {
	vUSHORT nameIndex;
	vUSHORT descIndex;
};

struct vUTF8BODY {
	vUSHORT len;
	V<vBYTE> s;
};

struct vOBJECTREF {
	vREF r;
};

struct vSTRING {
	vUSHORT strIndex;
};

struct vUTF8 {
	vREF r;
};

struct vCONST {
	vREF ref;
};

class vTypes {
public:
	template<class T> static vBYTE type() {
		return 0;
	}
	template<> static vBYTE type<vJCHAR>() {
		return 4;
	}
	template<> static vBYTE type<vFLOAT>() {
		return 6;
	}
	template<> static vBYTE type<vDOUBLE>() {
		return 7;
	}
	template<> static vBYTE type<vBYTE>() {
		return 8;
	}
	template<> static vBYTE type<vSHORT>() {
		return 9;
	}
	template<> static vBYTE type<vUSHORT>() {
		return 9;
	}
	template<> static vBYTE type<vINT>() {
		return 10;
	}
	template<> static vBYTE type<vUINT>() {
		return 10;
	}
	template<> static vBYTE type<vLONG>() {
		return 11;
	}
	template<> static vBYTE type<vULONG>() {
		return 11;
	}
	template<> static vBYTE type<vREF>() {
		return 12;
	}
	template<> static vBYTE type<vCONST>() {
		return 13;
	}
	template<> static vBYTE type<vUTF8>() {
		return 14;
	}
	template<> static vBYTE type<vMETHODREF>() {
		return 15;
	}
	template<> static vBYTE type<vCLASS>() {
		return 16;
	}
	template<> static vBYTE type<vSTRING>() {
		return 17;
	}
	template<> static vBYTE type<vNAMEANDTYPE>() {
		return 18;
	}
	template<> static vBYTE type<vOBJECTREF>() {
		return 12;
	}
};

struct vCOMMON {
	union {
		vCHAR c;
		vBYTE b;
		vSHORT si;
		vUSHORT usi;
		vJCHAR jc;
		vINT i;
		vUINT u;
		vLONG l;
		vULONG ul;
		vREF a;
		vUTF8 s;
		vCLASS cls;
		vSTRING str;
		vUTF8 utf8;
		vMETHODREF mr;
		vNAMEANDTYPE nt;
		vOBJECTREF objref;
		double d;
		float f;
	};
	vBYTE type;

	template<class T> static vCOMMON create(T value) {
		vCOMMON var;
		memset(&var, 0, sizeof(vCOMMON));
		memcpy(&var, &value, sizeof(T));
		var.type = vTypes::type<T>();
		return var;
	}
};

struct vATTRIBUTE {
	vUSHORT name;
	vUINT length;
	V<vBYTE> info;
};

struct vFIELD {
	vUSHORT access;
	vUSHORT name;
	vUSHORT desc;
	vUSHORT attributeCount;
	V<vATTRIBUTE> attributes;
	vCOMMON value;
};

typedef vFIELD vMETHOD;

struct vOBJECT {
	V<vCOMMON> fields;
	V<vClass> cls;
	vOBJECT(const V<vClass>& klass);
};