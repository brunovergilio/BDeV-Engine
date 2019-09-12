#pragma once


#include "../../BvFloatTypes.h"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_SIMD)


// ==================================
// Vector operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Access
// ======================

float VectorGetX(BvFastVec v);
float VectorGetY(BvFastVec v);
float VectorGetZ(BvFastVec v);
float VectorGetW(BvFastVec v);

BvFastVec VectorSet(const float x, const float y, const float z, const float w = 0.0f);

BvFastVec VectorReplicate(const float s);
BvFastVec VectorReplicateX(BvFastVec v);
BvFastVec VectorReplicateY(BvFastVec v);
BvFastVec VectorReplicateZ(BvFastVec v);
BvFastVec VectorReplicateW(BvFastVec v);
template<unsigned int x, unsigned int y, unsigned int z, unsigned int w> BvFastVec VectorPermute(BvFastVec v);

BvFastVec VectorMax(BvFastVec v1, BvFastVec v2);
BvFastVec VectorMin(BvFastVec v1, BvFastVec v2);
BvFastVec VectorAbs(BvFastVec v);
BvFastVec VectorFloor(BvFastVec v);
BvFastVec VectorCeil(BvFastVec v);
BvFastVec VectorRound(BvFastVec v);

BvFastVec VectorNegate(BvFastVec v);
BvFastVec VectorZero();

BvFastVec VectorIsZeroV(BvFastVec v, const float epsilon = BvEpsilon);
bool VectorIsZero(BvFastVec v, const float epsilon = BvEpsilon);
BvFastVec VectorIsEqualV(BvFastVec v1, BvFastVec v2, const float epsilon = BvEpsilon);
bool VectorIsEqual(BvFastVec v1, BvFastVec v2, const float epsilon = BvEpsilon);
BvFastVec VectorIsExactlyEqualV(BvFastVec v1, BvFastVec v2);
bool VectorIsExactlyEqual(BvFastVec v1, BvFastVec v2);

BvFastVec VectorGreater(BvFastVec v1, BvFastVec v2);
BvFastVec VectorGreaterEq(BvFastVec v1, BvFastVec v2);
BvFastVec VectorLess(BvFastVec v1, BvFastVec v2);
BvFastVec VectorLessEq(BvFastVec v1, BvFastVec v2);

unsigned int VectorGetMask(BvFastVec v);

// ======================
// Basic Operations
// ======================

BvFastVec VectorAdd(BvFastVec v1, BvFastVec v2);
BvFastVec VectorSub(BvFastVec v1, BvFastVec v2);
BvFastVec VectorMul(BvFastVec v1, BvFastVec v2);
BvFastVec VectorDiv(BvFastVec v1, BvFastVec v2);

BvFastVec VectorAdd(BvFastVec v, const float val);
BvFastVec VectorSub(BvFastVec v, const float val);
BvFastVec VectorMul(BvFastVec v, const float val);
BvFastVec VectorDiv(BvFastVec v, const float val);

BvFastVec VectorMul(BvFastVec v, const BvFastMat & m); // Row-major
BvFastVec VectorMul(const BvFastMat & m, BvFastVec v); // Col-major

float VectorDot(BvFastVec v1, BvFastVec v2);
BvFastVec VectorDotV(BvFastVec v1, BvFastVec v2);

BvFastVec VectorCross(BvFastVec v1, BvFastVec v2);

float VectorLengthSqr(BvFastVec v);
BvFastVec VectorLengthSqrV(BvFastVec v);

float VectorLength(BvFastVec v);
BvFastVec VectorLengthV(BvFastVec v);

BvFastVec VectorNormalize(BvFastVec v);

BvFastVec VectorProject(BvFastVec v1, BvFastVec v2);

BvFastVec VectorLerp(BvFastVec v1, BvFastVec v2, float t);

BvFastVec VectorReflection(BvFastVec i, BvFastVec n);

// ======================
// Operators
// ======================

BvFastVec operator + (BvFastVec v);
BvFastVec operator - (BvFastVec v);

BvFastVec operator + (BvFastVec v1, BvFastVec v2);
BvFastVec operator + (BvFastVec v, const float s);
BvFastVec operator + (const float s, BvFastVec v);
BvFastVec & operator += (BvFastVec & v1, BvFastVec v2);

BvFastVec operator - (BvFastVec v1, BvFastVec v2);
BvFastVec operator - (BvFastVec v, const float s);
BvFastVec operator - (const float s, BvFastVec v);
BvFastVec & operator -= (BvFastVec & v1, BvFastVec v2);

BvFastVec operator * (BvFastVec v1, BvFastVec v2);
BvFastVec & operator *= (BvFastVec & v1, BvFastVec v2);

BvFastVec operator * (BvFastVec v, const float s);
BvFastVec operator * (const float s, BvFastVec v);
BvFastVec & operator *= (BvFastVec & v, const float s);

BvFastVec operator * (BvFastVec v, const BvFastMat & m);
BvFastVec operator * (const BvFastMat & m, BvFastVec v);
BvFastVec & operator *= (BvFastVec & v, const BvFastMat & m);

BvFastVec operator / (BvFastVec v1, BvFastVec v2);
BvFastVec & operator /= (BvFastVec & v1, BvFastVec v2);

BvFastVec operator / (BvFastVec v, const float s);
BvFastVec & operator /= (BvFastVec & v, const float s);

// ======================
// Specialized Operations
// ======================
		
BvFastVec VectorTransformDir(BvFastVec v, const BvFastMat & m);
BvFastVec VectorTransformPoint(BvFastVec v, const BvFastMat & m);

// =================
// Definitions
// =================

// ======================
// Access
// ======================

inline float VectorGetX(BvFastVec v)
{
	return _mm_cvtss_f32(v);
}

inline float VectorGetY(BvFastVec v)
{
	BvFastVec y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_cvtss_f32(y);
}

inline float VectorGetZ(BvFastVec v)
{
	BvFastVec z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	return _mm_cvtss_f32(z);
}

inline float VectorGetW(BvFastVec v)
{
	BvFastVec w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
	return _mm_cvtss_f32(w);
}

inline BvFastVec VectorSet(const float x, const float y, const float z, const float w)
{
	return _mm_set_ps(w, z, y, x);
}

inline BvFastVec VectorReplicate(const float s)
{
	return _mm_set1_ps(s);
}

inline BvFastVec VectorReplicateX(BvFastVec v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}

inline BvFastVec VectorReplicateY(BvFastVec v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}

inline BvFastVec VectorReplicateZ(BvFastVec v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}

inline BvFastVec VectorReplicateW(BvFastVec v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

template<unsigned int x, unsigned int y, unsigned int z, unsigned int w>
inline BvFastVec VectorPermute(BvFastVec v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(w, z, y, x));
}

inline BvFastVec VectorMax(BvFastVec v1, BvFastVec v2)
{
	return _mm_max_ps(v1, v2);
}

inline BvFastVec VectorMin(BvFastVec v1, BvFastVec v2)
{
	return _mm_min_ps(v1, v2);
}

inline BvFastVec VectorAbs(BvFastVec v)
{
	BvFastVec signMask = _mm_set1_ps(-0.0f);
	return _mm_andnot_ps(signMask, v);
}

inline BvFastVec VectorFloor(BvFastVec v)
{
	return _mm_floor_ps(v);
}

inline BvFastVec VectorCeil(BvFastVec v)
{
	return _mm_ceil_ps(v);
}

inline BvFastVec VectorRound(BvFastVec v)
{
	return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT);
}

inline BvFastVec VectorNegate(BvFastVec v)
{
	return _mm_xor_ps(v, _mm_set1_ps(-0.0f));
}

inline BvFastVec VectorZero()
{
	return _mm_setzero_ps();
}

inline BvFastVec VectorIsZeroV(BvFastVec v, const float epsilon)
{
	BvFastVec e = _mm_set1_ps(epsilon);
	BvFastVec z = _mm_setzero_ps();
	z = _mm_sub_ps(z, v);
	z = _mm_max_ps(z, v);
	// cmple (compare less or equal) will check the condition for every element
	// the result will be either 0xffffffff if true or 0 if false (per element)
	return _mm_cmple_ps(z, e);
}

inline bool VectorIsZero(BvFastVec v, const float epsilon)
{
	BvFastVec r = VectorIsZeroV(v, epsilon);

	// movemask will get the most significant bit of every element and shift them
	// x << 0 | y << 1 | z << 2 | w << 3
	// so if every element matches, the result should be 0xf (1111, first four bits set)
	return _mm_movemask_ps(r) == 0xf;
}

inline BvFastVec VectorIsEqualV(BvFastVec v1, BvFastVec v2, const float epsilon)
{
	return VectorIsZeroV(_mm_sub_ps(v1, v2), epsilon);
}

inline bool VectorIsEqual(BvFastVec v1, BvFastVec v2, const float epsilon)
{
	return VectorIsZero(_mm_sub_ps(v1, v2), epsilon);
}

inline BvFastVec VectorIsExactlyEqualV(BvFastVec v1, BvFastVec v2)
{
	return _mm_cmpeq_ps(v1, v2);
}

inline bool VectorIsExactlyEqual(BvFastVec v1, BvFastVec v2)
{
	BvFastVec c = VectorIsExactlyEqualV(v1, v2);

	return _mm_movemask_ps(c) == 0xf;
}

inline BvFastVec VectorGreater(BvFastVec v1, BvFastVec v2)
{
	return _mm_cmpgt_ps(v1, v2);
}

inline BvFastVec VectorGreaterEq(BvFastVec v1, BvFastVec v2)
{
	return _mm_cmpge_ps(v1, v2);
}

inline BvFastVec VectorLess(BvFastVec v1, BvFastVec v2)
{
	return _mm_cmplt_ps(v1, v2);
}

inline BvFastVec VectorLessEq(BvFastVec v1, BvFastVec v2)
{
	return _mm_cmple_ps(v1, v2);
}

inline unsigned int VectorGetMask(BvFastVec v)
{
	return _mm_movemask_ps(v);
}

// ======================
// Basic Operations
// ======================

inline BvFastVec VectorAdd(BvFastVec v1, BvFastVec v2)
{
	return _mm_add_ps(v1, v2);
}

inline BvFastVec VectorSub(BvFastVec v1, BvFastVec v2)
{
	return _mm_sub_ps(v1, v2);
}

inline BvFastVec VectorMul(BvFastVec v1, BvFastVec v2)
{
	return _mm_mul_ps(v1, v2);
}

inline BvFastVec VectorDiv(BvFastVec v1, BvFastVec v2)
{
	return _mm_div_ps(v1, v2);
}

inline BvFastVec VectorAdd(BvFastVec v, const float val)
{
	return _mm_add_ps(v, _mm_set_ps1(val));
}

inline BvFastVec VectorSub(BvFastVec v, const float val)
{
	return _mm_sub_ps(v, _mm_set_ps1(val));
}

inline BvFastVec VectorMul(BvFastVec v, const float val)
{
	return _mm_mul_ps(v, _mm_set_ps1(val));
}

inline BvFastVec VectorDiv(BvFastVec v, const float val)
{
	return _mm_div_ps(v, _mm_set_ps1(val));
}

inline BvFastVec VectorMul(BvFastVec v, const BvFastMat & m)
{
	BvFastVec tmp0 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m.r[0]);

	BvFastVec tmp1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m.r[1]);

	BvFastVec tmp2 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m.r[2]);

	BvFastVec tmp3 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m.r[3]);

	BvFastVec r = _mm_add_ps(tmp0, tmp1);
	r = _mm_add_ps(r, _mm_add_ps(tmp2, tmp3));

	return r;
}

inline BvFastVec VectorMul(const BvFastMat & m, BvFastVec v)
{
	BvFastVec tmp0 = _mm_mul_ps(m.r[0], v);
	BvFastVec tmp1 = _mm_mul_ps(m.r[1], v);
	BvFastVec tmp2 = _mm_mul_ps(m.r[2], v);
	BvFastVec tmp3 = _mm_mul_ps(m.r[3], v);

	// top left 2x2 matrix
	BvFastVec upperBlock = _mm_shuffle_ps(tmp0, tmp1, _MM_SHUFFLE(1, 0, 1, 0));
	// bottom left 2x2 matrix
	BvFastVec lowerBlock = _mm_shuffle_ps(tmp2, tmp3, _MM_SHUFFLE(1, 0, 1, 0));

	BvFastMat mT;
	mT.r[0] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[1] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	// top right 2x2 matrix
	upperBlock = _mm_shuffle_ps(tmp0, tmp1, _MM_SHUFFLE(3, 2, 3, 2));
	// bottom right 2x2 matrix
	lowerBlock = _mm_shuffle_ps(tmp2, tmp3, _MM_SHUFFLE(3, 2, 3, 2));

	mT.r[2] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[3] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	BvFastVec r = _mm_add_ps(mT.r[0], mT.r[1]);
	r = _mm_add_ps(r, _mm_add_ps(mT.r[2], mT.r[3]));

	return r;
}

inline float VectorDot(BvFastVec v1, BvFastVec v2)
{
	// _mm_cvtss_f32 gets the value of the lowest float, in this case, X
	// _mm_store_ss could also be used, but I think it might be slower
	return _mm_cvtss_f32(VectorDotV(v1, v2));
}

inline BvFastVec VectorDotV(BvFastVec v1, BvFastVec v2)
{
	// _mm_dp_ps will use a 8 bit mask where:
	// bits 7, 6, 5 and 4 describe which elements to multiply
	// bits 3, 2, 1 and 0 describe where elements will be stored

	// For this implementation, 0111 1111 will be used, so x, y and z will get multiplied
	// and the value is stored across all elements
	return _mm_dp_ps(v1, v2, 0x7F);
}

inline BvFastVec VectorCross(BvFastVec v1, BvFastVec v2)
{
	// Using _MM_SHUFFLE macro to change the element places (reverse order)
	// A(3, 2, 1, 0)
	// B(3, 2, 1, 0)
	// SHUFFLE AB (0, 1, 2, 3) = (B0, B1, A2, A3) (reverse)
	// Result => (A.w, A.z, B.y, B.x)

	// (v1.y, v1.z, v1.x, v1.w) * (v2.z, v2.x, v2.y, v2.w)
	// -
	// (v1.z, v1.x, v1.y, v1.w) * (v2.y, v2.z, v2.x, v2.w)
	return _mm_sub_ps
	(
		_mm_mul_ps(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 1, 0, 2))),
		_mm_mul_ps(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 0, 2, 1)))
	);

	//__m128i w0 = _mm_set_epi32(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	//return _mm_and_ps(r, _mm_castsi128_ps(w0));
}

inline float VectorLengthSqr(BvFastVec v)
{
	return _mm_cvtss_f32(VectorLengthSqrV(v));
}

inline BvFastVec VectorLengthSqrV(BvFastVec v)
{
	return VectorDotV(v, v);
}

inline float VectorLength(BvFastVec v)
{
	return _mm_cvtss_f32(VectorLengthV(v));
}

inline BvFastVec VectorLengthV(BvFastVec v)
{
	return _mm_sqrt_ps(VectorLengthSqrV(v));
}

inline BvFastVec VectorNormalize(BvFastVec v)
{
	return _mm_mul_ps(v, _mm_rsqrt_ps(VectorLengthSqrV(v)));
}

inline BvFastVec VectorProject(BvFastVec v, BvFastVec p)
{
	// Projection = (V.P) * (P / ||P||)
	BvFastVec vDotP = VectorDotV(v, p);
	BvFastVec pN = VectorNormalize(p);
	return VectorMul(vDotP, pN);
}

inline BvFastVec VectorLerp(BvFastVec v1, BvFastVec v2, float t)
{
	// Lerp = V0 + t * (V1 - V0)
	BvFastVec vT = _mm_set1_ps(t);
	BvFastVec v2MinusV1 = _mm_sub_ps(v2, v1);

	return _mm_add_ps(v1, _mm_mul_ps(vT, v2MinusV1));
}

inline BvFastVec VectorReflection(BvFastVec i, BvFastVec n)
{
	// Reflection = I - 2(I.N)N
	BvFastVec twoTimesProj = VectorProject(i, n);
	twoTimesProj = VectorAdd(twoTimesProj, twoTimesProj);

	return VectorSub(i, twoTimesProj);
}


// ======================
// Operators
// ======================

inline BvFastVec operator + (BvFastVec v)
{
	return v;
}

inline BvFastVec operator - (BvFastVec v)
{
	return VectorNegate(v);
}

inline BvFastVec operator + (BvFastVec v1, BvFastVec v2)
{
	return VectorAdd(v1, v2);
}

inline BvFastVec operator+(BvFastVec v, const float s)
{
	return VectorAdd(v, s);
}

inline BvFastVec operator+(const float s, BvFastVec v)
{
	return VectorAdd(v, s);
}

inline BvFastVec & operator += (BvFastVec & v1, BvFastVec v2)
{
	v1 = VectorAdd(v1, v2);
	return v1;
}

inline BvFastVec operator - (BvFastVec v1, BvFastVec v2)
{
	return VectorSub(v1, v2);
}

inline BvFastVec operator-(BvFastVec v, const float s)
{
	return VectorSub(v, s);
}

inline BvFastVec operator-(const float s, BvFastVec v)
{
	return VectorSub(VectorReplicate(s), v);
}

inline BvFastVec & operator -= (BvFastVec & v1, BvFastVec v2)
{
	v1 = VectorSub(v1, v2);
	return v1;
}

inline BvFastVec operator * (BvFastVec v1, BvFastVec v2)
{
	return VectorMul(v1, v2);
}

inline BvFastVec & operator *= (BvFastVec & v1, BvFastVec v2)
{
	v1 = VectorMul(v1, v2);
	return v1;
}

inline BvFastVec operator * (BvFastVec v, const float s)
{
	return VectorMul(v, s);
}

inline BvFastVec operator * (const float s, BvFastVec v)
{
	return VectorMul(v, s);
}

inline BvFastVec & operator *= (BvFastVec & v, const float s)
{
	v = VectorMul(v, s);
	return v;
}

inline BvFastVec operator * (BvFastVec v, const BvFastMat & m)
{
	return VectorMul(v, m);
}

inline BvFastVec operator*(const BvFastMat & m, BvFastVec v)
{
	return VectorMul(m, v);
}

inline BvFastVec & operator *= (BvFastVec & v, const BvFastMat & m)
{
	v = VectorMul(v, m);
	return v;
}

inline BvFastVec operator / (BvFastVec v1, BvFastVec v2)
{
	return VectorDiv(v1, v2);
}

inline BvFastVec & operator /= (BvFastVec & v1, BvFastVec v2)
{
	v1 = VectorDiv(v1, v2);
	return v1;
}

inline BvFastVec operator / (BvFastVec v, const float s)
{
	return VectorDiv(v, s);
}

inline BvFastVec & operator /= (BvFastVec & v, const float s)
{
	v = VectorDiv(v, s);
	return v;
}

// ======================
// Specialized Operations
// ======================


inline BvFastVec VectorTransformDir(BvFastVec v, const BvFastMat & m)
{
	BvFastVec c = VectorReplicateX(v);
	BvFastVec r = _mm_mul_ps(c, m.r[0]);

	c = VectorReplicateY(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[1]));

	c = VectorReplicateZ(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[2]));

	return r;
}

inline BvFastVec VectorTransformPoint(BvFastVec v, const BvFastMat & m)
{
	BvFastVec c = VectorReplicateX(v);
	BvFastVec r = _mm_mul_ps(c, m.r[0]);

	c = VectorReplicateY(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[1]));

	c = VectorReplicateZ(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[2]));

	return _mm_add_ps(r, m.r[3]);
}

#endif