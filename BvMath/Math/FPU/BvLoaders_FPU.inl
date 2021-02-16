#pragma once


#include "../../BvFloatTypes.h"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_FPU)


BvFastVec Load(const float * const p);
BvFastVec Load2(const float * const p);
BvFastVec Load3(const float * const p);
BvFastVec Load4(const float * const p);
BvFastVec Load4A(const float * const p);

void Store(const BvFastVec & v, float * const p);
void Store2(const BvFastVec & v, float * const p);
void Store3(const BvFastVec & v, float * const p);
void Store4(const BvFastVec & v, float * const p);
void Store4A(const BvFastVec & v, float * const p);

BvFastMat Load22(const float * const p);
BvFastMat Load33(const float * const p);
BvFastMat Load43(const float * const p);
BvFastMat Load44(const float * const p);
BvFastMat Load44A(const float * const p);

void Store22(const BvFastMat & m, float * const p);
void Store33(const BvFastMat & m, float * const p);
void Store43(const BvFastMat & m, float * const p);
void Store44(const BvFastMat & m, float * const p);
void Store44A(const BvFastMat & m, float * const p);


inline BvFastVec Load(const float * const p)
{
	return BvFastVec(p[0], 0.0f, 0.0f, 0.0f);
}

inline BvFastVec Load2(const float * const p)
{
	return BvFastVec(p[0], p[1], 0.0f, 0.0f);
}

inline BvFastVec Load3(const float * const p)
{
	return BvFastVec(p[0], p[1], p[2], 0.0f);
}

inline BvFastVec Load4(const float * const p)
{
	return BvFastVec(p[0], p[1], p[2], p[3]);
}

inline BvFastVec Load4A(const float * const p)
{
	return BvFastVec(p[0], p[1], p[2], p[3]);
}

inline void Store(const BvFastVec & v, float * const p)
{
	p[0] = v.x;
}

inline void Store2(const BvFastVec & v, float * const p)
{
	p[0] = v.x;
	p[1] = v.y;
}

inline void Store3(const BvFastVec & v, float * const p)
{
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
}

inline void Store4(const BvFastVec & v, float * const p)
{
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
	p[3] = v.w;
}

inline void Store4A(const BvFastVec & v, float * const p)
{
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
	p[3] = v.w;
}

inline BvFastMat Load22(const float * const p)
{
	BvFastMat r;
	r.r[0] = Load2(p);
	r.r[1] = Load2(p + 2);
	r.r[2].Set(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3].Set(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline BvFastMat Load33(const float * const p)
{
	BvFastMat r;
	r.r[0] = Load3(p);
	r.r[1] = Load3(p + 3);
	r.r[2] = Load3(p + 6);
	r.r[3].Set(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline BvFastMat Load43(const float * const p)
{
	BvFastMat r;
	r.r[0] = Load3(p);
	r.r[1] = Load3(p + 3);
	r.r[2] = Load3(p + 6);
	r.r[3] = Load3(p + 9);

	return r;
}

inline BvFastMat Load44(const float * const p)
{
	BvFastMat r;
	r.r[0] = Load4(p);
	r.r[1] = Load4(p + 4);
	r.r[2] = Load4(p + 8);
	r.r[3] = Load4(p + 12);

	return r;
}

inline BvFastMat Load44A(const float * const p)
{
	BvFastMat r;
	r.r[0] = Load4A(p);
	r.r[1] = Load4A(p + 4);
	r.r[2] = Load4A(p + 8);
	r.r[3] = Load4A(p + 12);

	return r;
}

inline void Store22(const BvFastMat & m, float * const p)
{
	Store2(m.r[0], p);
	Store2(m.r[1], p + 2);
}

inline void Store33(const BvFastMat & m, float * const p)
{
	Store3(m.r[0], p);
	Store3(m.r[1], p + 3);
	Store3(m.r[2], p + 6);
}

inline void Store43(const BvFastMat & m, float * const p)
{
	Store3(m.r[0], p);
	Store3(m.r[1], p + 3);
	Store3(m.r[2], p + 6);
	Store3(m.r[3], p + 9);
}

inline void Store44(const BvFastMat & m, float * const p)
{
	Store4(m.r[0], p);
	Store4(m.r[1], p + 4);
	Store4(m.r[2], p + 8);
	Store4(m.r[3], p + 12);
}

inline void Store44A(const BvFastMat & m, float * const p)
{
	Store4A(m.r[0], p);
	Store4A(m.r[1], p + 4);
	Store4A(m.r[2], p + 8);
	Store4A(m.r[3], p + 12);
}


#endif