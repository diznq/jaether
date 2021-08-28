#pragma once
#include <string.h>
#include <assert.h>
#include "Pointer.h"

namespace jaether {

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
		vCT_FLOAT = 4,
		vCT_LONG = 5,
		vCT_DOUBLE = 6,
		vCT_CLASS = 7,
		vCT_STRING = 8,
		vCT_FIELDREF = 9,
		vCT_METHODREF = 10,
		vCT_IFACEMETHODREF = 11,
		vCT_NAMEANDTYPE = 12,
		vCT_METHODHANDLE = 15,
		vCT_METHODTYPE = 16,
		vCT_INVOKEDYNAMIC = 18
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

	struct vARRAYREF {
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

	struct vINVOKEDYNAMIC {
		vUSHORT bootstrapMethodAttrIndex;
		vUSHORT nameIndex;
	};

	struct vMETHODHANDLE {
		vBYTE kind;
		vUSHORT index;
	};

	struct vMETHODTYPE {
		vUSHORT descIndex;
	};

	namespace vTypes {

		template<class T> static vBYTE type() {
			return 0;
		}
		template<> static vBYTE type<vBYTE>() {
			return 1;
		}
		template<> static vBYTE type<vSHORT>() {
			return 2;
		}
		template<> static vBYTE type<vUSHORT>() {
			return 2;
		}
		template<> static vBYTE type<vINT>() {
			return 3;
		}
		template<> static vBYTE type<vUINT>() {
			return 3;
		}
		template<> static vBYTE type<vLONG>() {
			return 4;
		}
		template<> static vBYTE type<vULONG>() {
			return 4;
		}
		template<> static vBYTE type<vJCHAR>() {
			return 5;
		}
		template<> static vBYTE type<vFLOAT>() {
			return 6;
		}
		template<> static vBYTE type<vDOUBLE>() {
			return 7;
		}
		template<> static vBYTE type<vREF>() {
			return 8;
		}
		template<> static vBYTE type<vCONST>() {
			return 9;
		}
		template<> static vBYTE type<vUTF8>() {
			return 10;
		}
		template<> static vBYTE type<vMETHODREF>() {
			return 11;
		}
		template<> static vBYTE type<vCLASS>() {
			return 12;
		}
		template<> static vBYTE type<vSTRING>() {
			return 13;
		}
		template<> static vBYTE type<vNAMEANDTYPE>() {
			return 14;
		}
		template<> static vBYTE type<vOBJECTREF>() {
			return 15;
		}
		template<> static vBYTE type<vARRAYREF>() {
			return 15;
		}
		template<> static vBYTE type<vMETHODHANDLE>() {
			return 16;
		}
		template<> static vBYTE type<vINVOKEDYNAMIC>() {
			return 17;
		}
		template<> static vBYTE type<vMETHODTYPE>() {
			return 18;
		}
	}

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
			vINVOKEDYNAMIC idyn;
			vMETHODHANDLE mh;
			vMETHODTYPE mt;
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

		vCOMMON() {
			memset(this, 0, sizeof(vCOMMON));
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

	struct vNATIVEARRAY {
		V<vBYTE> data;
		vUINT size;
		vBYTE type;
		vUSHORT cls;
		vNATIVEARRAY(vContext* ctx, vBYTE type, vUINT size);
		static vUINT unitSize(vBYTE type) {
			switch (type) {
			case 1: // V_REF
				return sizeof(vOBJECTREF);
			case 4:	// T_BOOLEAN
				return sizeof(vBYTE);
			case 5: // T_CHAR
				return sizeof(vJCHAR);
			case 6: // T_FLOAT
				return sizeof(vFLOAT);
			case 7: // T_DOUBLE
				return sizeof(vDOUBLE);
			case 8: // T_BYTE
				return sizeof(vBYTE);
			case 9: // T_SHORT
				return sizeof(vSHORT);
			case 10: // T_INT
				return sizeof(vINT);
			case 11: // T_LONG
				return sizeof(vLONG);
			default:
				return 0;
			}
		}

		template<typename T> void set(vContext* ctx, size_t index, const T& value) {
			size_t scaledIndex = index * unitSize(type);
			assert(index < size);
			vBYTE* base = data.Ptr(ctx) + scaledIndex;
			*(T*)base = value;
		}

		template<typename T> T& get(vContext* ctx, size_t index) {
			size_t scaledIndex = index * unitSize(type);
			assert(index < size);
			vBYTE* base = data.Ptr(ctx) + scaledIndex;
			return *(T*)base;
		}
	};

	struct vOBJECT {
		V<vCOMMON> fields;
		V<vClass> cls;
		vOBJECT(vContext* ctx, const V<vClass>& klass);
	};

}