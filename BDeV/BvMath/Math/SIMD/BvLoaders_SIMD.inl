#pragma once


#include "../../BvFloatTypes.h"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_SIMD)


BvFastVec Load(const float * const p);
BvFastVec Load2(const float * const p);
BvFastVec Load3(const float * const p);
BvFastVec Load4(const float * const p);
BvFastVec Load4A(const float * const p);

void Store(BvFastVec v, float * const p);
void Store2(BvFastVec v, float * const p);
void Store3(BvFastVec v, float * const p);
void Store4(BvFastVec v, float * const p);
void Store4A(BvFastVec v, float * const p);

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
	return _mm_load_ss(p);
}

inline BvFastVec Load2(const float * const p)
{
	BvFastVec x = _mm_load_ss(p);
	BvFastVec y = _mm_load_ss(p + 1);
	BvFastVec xy = _mm_movelh_ps(x, y);
	return _mm_shuffle_ps(xy, xy, _MM_SHUFFLE(3, 1, 2, 0));
}

inline BvFastVec Load3(const float * const p)
{
	BvFastVec x = _mm_load_ss(p);
	BvFastVec y = _mm_load_ss(p + 1);
	BvFastVec z = _mm_load_ss(p + 2);
	BvFastVec xy = _mm_movelh_ps(x, y);
	return _mm_shuffle_ps(xy, z, _MM_SHUFFLE(2, 0, 2, 0));
}

inline BvFastVec Load4(const float * const p)
{
	return _mm_loadu_ps(p);
}

inline BvFastVec Load4A(const float * const p)
{
	return _mm_load_ps(p);
}

inline void Store(BvFastVec v, float * const p)
{
	_mm_store_ss(p, v);
}

inline void Store2(BvFastVec v, float * const p)
{
	_mm_store_ss(p, v);
	_mm_store_ss(p + 1, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)));
}

inline void Store3(BvFastVec v, float * const p)
{
	_mm_store_ss(p, v);
	_mm_store_ss(p + 1, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)));
	_mm_store_ss(p + 2, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)));
}

inline void Store4(BvFastVec v, float * const p)
{
	_mm_storeu_ps(p, v);
}

inline void Store4A(BvFastVec v, float * const p)
{
	_mm_store_ps(p, v);
}

inline BvFastMat Load22(const float * const p)
{
	BvFastMat r;
	r.r[0] = Load2(p);
	r.r[1] = Load2(p + 2);
	r.r[2] = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline BvFastMat Load33(const float * const p)
{
	BvFastMat r;
	r.r[0] = Load3(p);
	r.r[1] = Load3(p + 3);
	r.r[2] = Load3(p + 6);
	r.r[3] = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);

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