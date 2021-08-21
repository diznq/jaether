#pragma once

typedef int8_t		vCHAR;
typedef int16_t		vSHORT;
typedef int32_t		vINT;
typedef int64_t		vLONG;
typedef uint8_t		vBYTE;
typedef uint16_t	vUSHORT;
typedef uint16_t	vJCHAR;
typedef uint32_t	vUINT;
typedef uint64_t	vULONG;
typedef uint64_t	vREF;
typedef uint32_t	vCAT1;
typedef uint64_t	vCAT2;
typedef float vFLOAT;
typedef double vDOUBLE;

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
		double d;
		float f;
	};
	vBYTE type;
};

class vTypes {
public:
	template<class T> static vBYTE type() {
		return 0;
	}
	template<> static vBYTE type<vCHAR>() {
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
};