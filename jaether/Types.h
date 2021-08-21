#pragma once

typedef int8_t		vCHAR;
typedef int16_t		vSHORT;
typedef int32_t		vINT;
typedef int64_t		vLONG;
typedef uint8_t		vBYTE;
typedef uint16_t	vUSHORT;
typedef uint32_t	vUINT;
typedef uint64_t	vULONG;
typedef uint64_t	vREF;
typedef uint32_t	vCAT1;
typedef uint64_t	vCAT2;
typedef float vFLOAT;
typedef double vDOUBLE;

union vCOMMON {
	vCHAR c;
	vBYTE b;
	vSHORT si;
	vUSHORT usi;
	vINT i;
	vUINT u;
	vLONG l;
	vULONG ul;
	double d;
	float f;
};
