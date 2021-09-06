#pragma once
#include <string.h>
#include <assert.h>
#include <exception>
#include <stdexcept>
#include "Pointer.h"

namespace jaether {

	template<class T> class JArray;
	class JObject;
	class JString;

#define JAETHER_OBJ_TAG 0x33010133
#define JAETHER_ARR_TAG 0x33020233

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
		template<> vBYTE type<vBYTE>() {
			return 1;
		}
		template<> vBYTE type<vSHORT>() {
			return 2;
		}
		template<> vBYTE type<vUSHORT>() {
			return 2;
		}
		template<> vBYTE type<vINT>() {
			return 3;
		}
		template<> vBYTE type<vUINT>() {
			return 3;
		}
		template<> vBYTE type<vLONG>() {
			return 4;
		}
		template<> vBYTE type<vULONG>() {
			return 4;
		}
		template<> vBYTE type<vJCHAR>() {
			return 5;
		}
		template<> vBYTE type<vFLOAT>() {
			return 6;
		}
		template<> vBYTE type<vDOUBLE>() {
			return 7;
		}
		template<> vBYTE type<vCONST>() {
			return 9;
		}
		template<> vBYTE type<vUTF8>() {
			return 10;
		}
		template<> vBYTE type<vMETHODREF>() {
			return 11;
		}
		template<> vBYTE type<vCLASS>() {
			return 12;
		}
		template<> vBYTE type<vSTRING>() {
			return 13;
		}
		template<> vBYTE type<vNAMEANDTYPE>() {
			return 14;
		}
		template<> vBYTE type<vREF>() {
			return 15;
		}
		template<> vBYTE type<vOBJECTREF>() {
			return 15;
		}
		template<> vBYTE type<vARRAYREF>() {
			return 15;
		}
		template<> vBYTE type<vMETHODHANDLE>() {
			return 16;
		}
		template<> vBYTE type<vINVOKEDYNAMIC>() {
			return 17;
		}
		template<> vBYTE type<vMETHODTYPE>() {
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

		template<class T> static vCOMMON create(const T& value) {
			vCOMMON var;
			memset(&var, 0, sizeof(vCOMMON));
			memcpy(&var, &value, sizeof(T));
			var.type = vTypes::type<T>();
			return var;
		}

		template<class T> vCOMMON& set(const T& value) {
			vCOMMON& var = *this;
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
		vClass* cls;
		vCOMMON value;
	};

	typedef vFIELD vMETHOD;

	struct vNATIVEARRAY {
		template<typename T> friend class JArray;
		int TAG = JAETHER_ARR_TAG;
		V<vBYTE> dataObj;
		V<vClass> cls;
		vCOMMON x;

		vUINT size;
		vBYTE type;

		vNATIVEARRAY(vContext* ctx, vBYTE type, vUINT size);
		static vUINT unitSize(vBYTE type) {
			switch (type) {
			case 1: // V_REF
				return sizeof(vCOMMON);
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
			if (index >= size) {
				throw std::runtime_error("out of bounds");
			}
			vBYTE& base = data()(ctx, scaledIndex);
			*(T*)&base = value;
			if (type == 1) {
				vCOMMON* c = (vCOMMON*)&base;
				c->type = vTypes::type<vOBJECTREF>();
			}
		}

		template<typename T> T& get(vContext* ctx, size_t index) {
			size_t scaledIndex = index * unitSize(type);
			if (index >= size) {
				throw std::runtime_error("out of bounds");
			}
			vBYTE* base = data()(ctx) + scaledIndex;
			return *(T*)base;
		}

		size_t release(vContext* ctx) {
			//printf("Release arr\n");
			size_t rel = data().release(ctx, true);
			return rel;
		}

		size_t release(Allocator* ctx) {
			//printf("Release arr\n");
			size_t rel = data().release(ctx, true);
			return rel;
		}

		V<vBYTE> data() const {
			return dataObj;
		}
	};

	struct vOBJECT {
		friend class JObject;
		friend class JString;
		int TAG = JAETHER_OBJ_TAG;
		V<vCOMMON> fieldsObj;
		V<vClass> cls;
		vCOMMON x;
		vOBJECT(vContext* ctx, const V<vClass>& klass);
		size_t release(vContext* ctx) {
			//printf("Release object\n");
			size_t rel = fields().release(ctx, true);
			return rel;
		}
		size_t release(Allocator* ctx) {
			//printf("Release object\n");
			size_t rel = fields().release(ctx, true);
			return rel;
		}
		V<vCOMMON> fields() const {
			return fieldsObj;
		}
	};

}