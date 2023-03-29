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

float VectorGetX(vf32 v);
float VectorGetY(vf32 v);
float VectorGetZ(vf32 v);
float VectorGetW(vf32 v);

vf32 VectorSet(const float x, const float y, const float z, const float w = 0.0f);

vf32 VectorReplicate(const float s);
vf32 VectorReplicateX(vf32 v);
vf32 VectorReplicateY(vf32 v);
vf32 VectorReplicateZ(vf32 v);
vf32 VectorReplicateW(vf32 v);
template<unsigned int x, unsigned int y, unsigned int z, unsigned int w> vf32 VectorPermute(vf32 v);

vf32 VectorMax(vf32 v1, vf32 v2);
vf32 VectorMin(vf32 v1, vf32 v2);
vf32 VectorAbs(vf32 v);
vf32 VectorFloor(vf32 v);
vf32 VectorCeil(vf32 v);
vf32 VectorRound(vf32 v);
vf32 VectorSqrt(vf32 v);
vf32 VectorRSqrt(vf32 v);
vf32 VectorRcp(vf32 v);

vf32 VectorNegate(vf32 v);
vf32 VectorZero();

vf32 VectorEqual(vf32 v1, vf32 v2);
vf32 VectorNearlyEqual(vf32 v1, vf32 v2, const float epsilon = kEpsilon);
vf32 VectorGreater(vf32 v1, vf32 v2);
vf32 VectorGreaterEqual(vf32 v1, vf32 v2);
vf32 VectorLess(vf32 v1, vf32 v2);
vf32 VectorLessEqual(vf32 v1, vf32 v2);
vf32 VectorOr(vf32 v1, vf32 v2);
vf32 VectorAnd(vf32 v1, vf32 v2);
vf32 VectorXor(vf32 v1, vf32 v2);
vf32 VectorAndNot(vf32 v1, vf32 v2);

unsigned int VectorGetMask(vf32 v);

// ======================
// Basic Operations
// ======================

vf32 VectorAdd(vf32 v1, vf32 v2);
vf32 VectorSub(vf32 v1, vf32 v2);
vf32 VectorMul(vf32 v1, vf32 v2);
vf32 VectorDiv(vf32 v1, vf32 v2);

vf32 VectorMAdd(vf32 v1, vf32 v2, vf32 v3);
vf32 VectorNMAdd(vf32 v1, vf32 v2, vf32 v3);

vf32 VectorAdd(vf32 v, const float val);
vf32 VectorSub(vf32 v, const float val);
vf32 VectorMul(vf32 v, const float val);
vf32 VectorDiv(vf32 v, const float val);

vf32 VectorMul(vf32 v, const mf32 & m); // Row-major
vf32 VectorMul(const mf32 & m, vf32 v); // Col-major

float VectorDot(vf32 v1, vf32 v2);
vf32 VectorDotV(vf32 v1, vf32 v2);

vf32 VectorCross(vf32 v1, vf32 v2);

float VectorLengthSqr(vf32 v);
vf32 VectorLengthSqrV(vf32 v);

float VectorLength(vf32 v);
vf32 VectorLengthV(vf32 v);

vf32 VectorNormalize(vf32 v);

vf32 VectorProject(vf32 v1, vf32 v2);
vf32 VectorProjectNormal(vf32 v1, vf32 v2);

vf32 VectorLerp(vf32 v1, vf32 v2, float t);

vf32 VectorReflection(vf32 i, vf32 n);
vf32 VectorRefraction(vf32 i, vf32 n, float eta);

// ======================
// Operators
// ======================

vf32 operator + (vf32 v);
vf32 operator - (vf32 v);

vf32 operator + (vf32 v1, vf32 v2);
vf32 operator + (vf32 v, const float s);
vf32 operator + (const float s, vf32 v);
vf32 & operator += (vf32 & v1, vf32 v2);

vf32 operator - (vf32 v1, vf32 v2);
vf32 operator - (vf32 v, const float s);
vf32 operator - (const float s, vf32 v);
vf32 & operator -= (vf32 & v1, vf32 v2);

vf32 operator * (vf32 v1, vf32 v2);
vf32 & operator *= (vf32 & v1, vf32 v2);

vf32 operator * (vf32 v, const float s);
vf32 operator * (const float s, vf32 v);
vf32 & operator *= (vf32 & v, const float s);

vf32 operator * (vf32 v, const mf32 & m);
vf32 operator * (const mf32 & m, vf32 v);
vf32 & operator *= (vf32 & v, const mf32 & m);

vf32 operator / (vf32 v1, vf32 v2);
vf32 & operator /= (vf32 & v1, vf32 v2);

vf32 operator / (vf32 v, const float s);
vf32 & operator /= (vf32 & v, const float s);

// ======================
// Specialized Operations
// ======================
		
vf32 VectorTransformDir(vf32 v, const mf32 & m);
vf32 VectorTransformPoint(vf32 v, const mf32 & m);

// =================
// Definitions
// =================

// ======================
// Access
// ======================

inline float VectorGetX(vf32 v)
{
	return _mm_cvtss_f32(v);
}

inline float VectorGetY(vf32 v)
{
	vf32 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_cvtss_f32(y);
}

inline float VectorGetZ(vf32 v)
{
	vf32 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	return _mm_cvtss_f32(z);
}

inline float VectorGetW(vf32 v)
{
	vf32 w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
	return _mm_cvtss_f32(w);
}

inline vf32 VectorSet(const float x, const float y, const float z, const float w)
{
	return _mm_set_ps(w, z, y, x);
}

inline vf32 VectorReplicate(const float s)
{
	return _mm_set1_ps(s);
}

inline vf32 VectorReplicateX(vf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}

inline vf32 VectorReplicateY(vf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}

inline vf32 VectorReplicateZ(vf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}

inline vf32 VectorReplicateW(vf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

template<unsigned int x, unsigned int y, unsigned int z, unsigned int w>
inline vf32 VectorPermute(vf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(w, z, y, x));
}

inline vf32 VectorMax(vf32 v1, vf32 v2)
{
	return _mm_max_ps(v1, v2);
}

inline vf32 VectorMin(vf32 v1, vf32 v2)
{
	return _mm_min_ps(v1, v2);
}

inline vf32 VectorAbs(vf32 v)
{
	vf32 signMask = _mm_set1_ps(-0.0f);
	return _mm_andnot_ps(signMask, v);
}

inline vf32 VectorFloor(vf32 v)
{
	return _mm_floor_ps(v);
}

inline vf32 VectorCeil(vf32 v)
{
	return _mm_ceil_ps(v);
}

inline vf32 VectorRound(vf32 v)
{
	return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT);
}

inline vf32 VectorSqrt(vf32 v)
{
	return _mm_sqrt_ps(v);
}

inline vf32 VectorRSqrt(vf32 v)
{
	return _mm_rsqrt_ps(v);
}

inline vf32 VectorRcp(vf32 v)
{
	return _mm_rcp_ps(v);
}

inline vf32 VectorNegate(vf32 v)
{
	return _mm_xor_ps(v, _mm_set1_ps(-0.0f));
}

inline vf32 VectorZero()
{
	return _mm_setzero_ps();
}

inline vf32 VectorEqual(vf32 v1, vf32 v2)
{
	return _mm_cmpeq_ps(v1, v2);
}

inline vf32 VectorNearlyEqual(vf32 v1, vf32 v2, const float epsilon)
{
	vf32 vDiff = VectorAbs(VectorSub(v1, v2));
	vf32 vE = VectorReplicate(epsilon);

	return VectorLessEqual(vDiff, vE);
}

inline vf32 VectorGreater(vf32 v1, vf32 v2)
{
	return _mm_cmpgt_ps(v1, v2);
}

inline vf32 VectorGreaterEqual(vf32 v1, vf32 v2)
{
	return _mm_cmpge_ps(v1, v2);
}

inline vf32 VectorLess(vf32 v1, vf32 v2)
{
	return _mm_cmplt_ps(v1, v2);
}

inline vf32 VectorLessEqual(vf32 v1, vf32 v2)
{
	return _mm_cmple_ps(v1, v2);
}

inline vf32 VectorOr(vf32 v1, vf32 v2)
{
	return _mm_or_ps(v1, v2);
}

inline vf32 VectorAnd(vf32 v1, vf32 v2)
{
	return _mm_and_ps(v1, v2);
}

inline vf32 VectorXor(vf32 v1, vf32 v2)
{
	return _mm_xor_ps(v1, v2);
}

inline vf32 VectorAndNot(vf32 v1, vf32 v2)
{
	return _mm_andnot_ps(v1, v2);
}

inline unsigned int VectorGetMask(vf32 v)
{
	return _mm_movemask_ps(v);
}

// ======================
// Basic Operations
// ======================

inline vf32 VectorAdd(vf32 v1, vf32 v2)
{
	return _mm_add_ps(v1, v2);
}

inline vf32 VectorSub(vf32 v1, vf32 v2)
{
	return _mm_sub_ps(v1, v2);
}

inline vf32 VectorMul(vf32 v1, vf32 v2)
{
	return _mm_mul_ps(v1, v2);
}

inline vf32 VectorDiv(vf32 v1, vf32 v2)
{
	return _mm_div_ps(v1, v2);
}

inline vf32 VectorMAdd(vf32 v1, vf32 v2, vf32 v3)
{
	return _mm_fmadd_ps(v1, v2, v3);
}

inline vf32 VectorNMAdd(vf32 v1, vf32 v2, vf32 v3)
{
	return _mm_fnmadd_ps(v1, v2, v3);
}

inline vf32 VectorAdd(vf32 v, const float val)
{
	return _mm_add_ps(v, _mm_set_ps1(val));
}

inline vf32 VectorSub(vf32 v, const float val)
{
	return _mm_sub_ps(v, _mm_set_ps1(val));
}

inline vf32 VectorMul(vf32 v, const float val)
{
	return _mm_mul_ps(v, _mm_set_ps1(val));
}

inline vf32 VectorDiv(vf32 v, const float val)
{
	return _mm_div_ps(v, _mm_set_ps1(val));
}

inline vf32 VectorMul(vf32 v, const mf32 & m)
{
	vf32 tmp0 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m.r[0]);

	vf32 tmp1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m.r[1]);

	vf32 tmp2 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m.r[2]);

	vf32 tmp3 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m.r[3]);

	vf32 r = _mm_add_ps(tmp0, tmp1);
	r = _mm_add_ps(r, _mm_add_ps(tmp2, tmp3));

	return r;
}

inline vf32 VectorMul(const mf32 & m, vf32 v)
{
	vf32 tmp0 = _mm_mul_ps(m.r[0], v);
	vf32 tmp1 = _mm_mul_ps(m.r[1], v);
	vf32 tmp2 = _mm_mul_ps(m.r[2], v);
	vf32 tmp3 = _mm_mul_ps(m.r[3], v);

	// top left 2x2 matrix
	vf32 upperBlock = _mm_shuffle_ps(tmp0, tmp1, _MM_SHUFFLE(1, 0, 1, 0));
	// bottom left 2x2 matrix
	vf32 lowerBlock = _mm_shuffle_ps(tmp2, tmp3, _MM_SHUFFLE(1, 0, 1, 0));

	mf32 mT;
	mT.r[0] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[1] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	// top right 2x2 matrix
	upperBlock = _mm_shuffle_ps(tmp0, tmp1, _MM_SHUFFLE(3, 2, 3, 2));
	// bottom right 2x2 matrix
	lowerBlock = _mm_shuffle_ps(tmp2, tmp3, _MM_SHUFFLE(3, 2, 3, 2));

	mT.r[2] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[3] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	vf32 r = _mm_add_ps(mT.r[0], mT.r[1]);
	r = _mm_add_ps(r, _mm_add_ps(mT.r[2], mT.r[3]));

	return r;
}

inline float VectorDot(vf32 v1, vf32 v2)
{
	// _mm_cvtss_f32 gets the value of the lowest float, in this case, X
	// _mm_store_ss could also be used, but I think it might be slower
	return _mm_cvtss_f32(VectorDotV(v1, v2));
}

inline vf32 VectorDotV(vf32 v1, vf32 v2)
{
	// _mm_dp_ps will use a 8 bit mask where:
	// bits 7, 6, 5 and 4 describe which elements to multiply
	// bits 3, 2, 1 and 0 describe where elements will be stored

	// For this implementation, 0111 1111 will be used, so x, y and z will get multiplied
	// and the value is stored across all elements
	return _mm_dp_ps(v1, v2, 0x7F);
}

inline vf32 VectorCross(vf32 v1, vf32 v2)
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

inline float VectorLengthSqr(vf32 v)
{
	return _mm_cvtss_f32(VectorLengthSqrV(v));
}

inline vf32 VectorLengthSqrV(vf32 v)
{
	return VectorDotV(v, v);
}

inline float VectorLength(vf32 v)
{
	return _mm_cvtss_f32(VectorLengthV(v));
}

inline vf32 VectorLengthV(vf32 v)
{
	return _mm_sqrt_ps(VectorLengthSqrV(v));
}

inline vf32 VectorNormalize(vf32 v)
{
	return _mm_mul_ps(v, _mm_rsqrt_ps(VectorLengthSqrV(v)));
}

inline vf32 VectorProject(vf32 v, vf32 p)
{
	// Projection = (V.P) * (P / ||P||)
	vf32 vDotP = VectorDotV(v, p);
	vf32 pN = VectorNormalize(p);
	return VectorMul(vDotP, pN);
}

inline vf32 VectorProjectNormal(vf32 v, vf32 p)
{
	// Projection = (V.P) * (P / ||P||)
	vf32 vDotP = VectorDotV(v, p);
	return VectorMul(vDotP, p);
}

inline vf32 VectorLerp(vf32 v1, vf32 v2, float t)
{
	// Lerp = V0 + t * (V1 - V0)
	vf32 vT = _mm_set1_ps(t);

	// (t * v2) + (-(t * v1) + v1)
	return VectorMAdd(vT, v2, VectorNMAdd(vT, v1, v1));
}

inline vf32 VectorReflection(vf32 i, vf32 n)
{
	// Reflection = I - 2(I.N)N
	vf32 twoTimesProj = VectorMul(VectorDotV(i, n), n);
	twoTimesProj = VectorAdd(twoTimesProj, twoTimesProj);

	return VectorSub(i, twoTimesProj);
}

inline vf32 VectorRefraction(vf32 i, vf32 n, float eta)
{
	vf32 v1 = VectorReplicate(1.0f);
	vf32 iDotN = VectorDotV(i, n);
	vf32 iDotN2 = VectorMul(iDotN, iDotN);
	vf32 vEta = VectorReplicate(eta);
	vf32 vEta2 = VectorMul(vEta, vEta);

	// 1.0f - (-(iDotN^2 * eta^2) + eta^2) = 1.0f - eta^2 * (1.0f - iDotN^2)
	vf32 vK = VectorSub(v1, _mm_fnmadd_ps(iDotN2, vEta2, vEta2));

	vf32 c0 = VectorLess(vK, VectorZero());

	// eta * i - (eta * iDotN + sqrtf(k)) * n;
	auto r0 = VectorMul(n, iDotN); // n * iDotN
	auto r1 = VectorMul(n, _mm_sqrt_ps(vK)); // n * sqrtf(k)
	r1 = VectorMAdd(vEta, r0, r1); // eta * n * iDotN + n * sqrtf(k)
	r0 = VectorMul(vEta, i); // eta * i
	r0 = VectorSub(r0, r1); // eta * i - eta * n * iDotN - n * sqrtf(k)

	return _mm_andnot_ps(c0, r0);
}


// ======================
// Operators
// ======================

inline vf32 operator + (vf32 v)
{
	return v;
}

inline vf32 operator - (vf32 v)
{
	return VectorNegate(v);
}

inline vf32 operator + (vf32 v1, vf32 v2)
{
	return VectorAdd(v1, v2);
}

inline vf32 operator+(vf32 v, const float s)
{
	return VectorAdd(v, s);
}

inline vf32 operator+(const float s, vf32 v)
{
	return VectorAdd(v, s);
}

inline vf32 & operator += (vf32 & v1, vf32 v2)
{
	v1 = VectorAdd(v1, v2);
	return v1;
}

inline vf32 operator - (vf32 v1, vf32 v2)
{
	return VectorSub(v1, v2);
}

inline vf32 operator-(vf32 v, const float s)
{
	return VectorSub(v, s);
}

inline vf32 operator-(const float s, vf32 v)
{
	return VectorSub(VectorReplicate(s), v);
}

inline vf32 & operator -= (vf32 & v1, vf32 v2)
{
	v1 = VectorSub(v1, v2);
	return v1;
}

inline vf32 operator * (vf32 v1, vf32 v2)
{
	return VectorMul(v1, v2);
}

inline vf32 & operator *= (vf32 & v1, vf32 v2)
{
	v1 = VectorMul(v1, v2);
	return v1;
}

inline vf32 operator * (vf32 v, const float s)
{
	return VectorMul(v, s);
}

inline vf32 operator * (const float s, vf32 v)
{
	return VectorMul(v, s);
}

inline vf32 & operator *= (vf32 & v, const float s)
{
	v = VectorMul(v, s);
	return v;
}

inline vf32 operator * (vf32 v, const mf32 & m)
{
	return VectorMul(v, m);
}

inline vf32 operator*(const mf32 & m, vf32 v)
{
	return VectorMul(m, v);
}

inline vf32 & operator *= (vf32 & v, const mf32 & m)
{
	v = VectorMul(v, m);
	return v;
}

inline vf32 operator / (vf32 v1, vf32 v2)
{
	return VectorDiv(v1, v2);
}

inline vf32 & operator /= (vf32 & v1, vf32 v2)
{
	v1 = VectorDiv(v1, v2);
	return v1;
}

inline vf32 operator / (vf32 v, const float s)
{
	return VectorDiv(v, s);
}

inline vf32 & operator /= (vf32 & v, const float s)
{
	v = VectorDiv(v, s);
	return v;
}

// ======================
// Specialized Operations
// ======================


inline vf32 VectorTransformDir(vf32 v, const mf32 & m)
{
	vf32 c = VectorReplicateX(v);
	vf32 r = _mm_mul_ps(c, m.r[0]);

	c = VectorReplicateY(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[1]));

	c = VectorReplicateZ(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[2]));

	return r;
}

inline vf32 VectorTransformPoint(vf32 v, const mf32 & m)
{
	vf32 c = VectorReplicateX(v);
	vf32 r = _mm_mul_ps(c, m.r[0]);

	c = VectorReplicateY(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[1]));

	c = VectorReplicateZ(v);
	r = _mm_add_ps(r, _mm_mul_ps(c, m.r[2]));

	return _mm_add_ps(r, m.r[3]);
}

#endif