#pragma once


#include "../../BvFloatTypes.h"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_FPU)


// ==================================
// Vector operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Access
// ======================

float VectorGetX(const BvFastVec & v);
float VectorGetY(const BvFastVec & v);
float VectorGetZ(const BvFastVec & v);
float VectorGetW(const BvFastVec & v);

BvFastVec VectorSet(const float x, const float y, const float z, const float w = 0.0f);

BvFastVec VectorReplicate(const float s);
BvFastVec VectorReplicateX(const BvFastVec & v);
BvFastVec VectorReplicateY(const BvFastVec & v);
BvFastVec VectorReplicateZ(const BvFastVec & v);
BvFastVec VectorReplicateW(const BvFastVec & v);
template<unsigned int x, unsigned int y, unsigned int z, unsigned int w> BvFastVec VectorPermute(BvFastVec v);

BvFastVec VectorMax(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorMin(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorAbs(const BvFastVec & v);
BvFastVec VectorFloor(const BvFastVec &v);
BvFastVec VectorCeil(const BvFastVec & v);
BvFastVec VectorRound(const BvFastVec & v);

BvFastVec VectorNegate(const BvFastVec & v);
BvFastVec VectorZero();

BvFastVec VectorIsZeroV(const BvFastVec & v, const float epsilon = BvEpsilon);
bool VectorIsZero(const BvFastVec & v, const float epsilon = BvEpsilon);
BvFastVec VectorIsEqualV(const BvFastVec & v1, const BvFastVec & v2, const float epsilon = BvEpsilon);
bool VectorIsEqual(const BvFastVec & v1, const BvFastVec & v2, const float epsilon = BvEpsilon);
BvFastVec VectorIsExactlyEqualV(const BvFastVec & v1, const BvFastVec & v2);
bool VectorIsExactlyEqual(const BvFastVec & v1, const BvFastVec & v2);

BvFastVec VectorGreater(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorGreaterEq(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorLess(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorLessEq(const BvFastVec & v1, const BvFastVec & v2);

unsigned int VectorGetMask(const BvFastVec & v);

// ======================
// Basic Operations
// ======================

BvFastVec VectorAdd(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorSub(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorMul(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorDiv(const BvFastVec & v1, const BvFastVec & v2);

BvFastVec VectorAdd(const BvFastVec & v, const float val);
BvFastVec VectorSub(const BvFastVec & v, const float val);
BvFastVec VectorMul(const BvFastVec & v, const float val);
BvFastVec VectorDiv(const BvFastVec & v, const float val);

BvFastVec VectorMul(const BvFastVec & v, const BvFastMat & m); // Row-major
BvFastVec VectorMul(const BvFastMat & m, const BvFastVec & v); // Col-major

float VectorDot(const BvFastVec & v1, const BvFastVec & v2);
BvFastVec VectorDotV(const BvFastVec & v1, const BvFastVec & v2);

BvFastVec VectorCross(const BvFastVec & v1, const BvFastVec & v2);

float VectorLengthSqr(const BvFastVec & v);
BvFastVec VectorLengthSqrV(const BvFastVec & v);

float VectorLength(const BvFastVec & v);
BvFastVec VectorLengthV(const BvFastVec & v);

BvFastVec VectorNormalize(const BvFastVec & v);

BvFastVec VectorProject(const BvFastVec & v1, const BvFastVec & v2);

BvFastVec VectorLerp(const BvFastVec & v1, const BvFastVec & v2, float t);

BvFastVec VectorReflection(const BvFastVec & i, const BvFastVec & n);

// ======================
// Operators
// ======================

BvFastVec operator + (const BvFastVec & v);
BvFastVec operator - (const BvFastVec & v);

BvFastVec operator + (const BvFastVec & v1, const BvFastVec & v2);
BvFastVec operator + (const BvFastVec & v, const float s);
BvFastVec operator + (const float s, const BvFastVec & v);
BvFastVec & operator += (BvFastVec & v1, const BvFastVec & v2);

BvFastVec operator - (const BvFastVec & v1, const BvFastVec & v2);
BvFastVec operator - (const BvFastVec & v, const float s);
BvFastVec operator - (const float s, const BvFastVec & v);
BvFastVec & operator -= (BvFastVec & v1, const BvFastVec & v2);

BvFastVec operator * (const BvFastVec & v1, const BvFastVec & v2);
BvFastVec & operator *= (BvFastVec & v1, const BvFastVec & v2);

BvFastVec operator * (const BvFastVec & v, const float s);
BvFastVec operator * (const float s, const BvFastVec & v);
BvFastVec & operator *= (BvFastVec & v, const float s);

BvFastVec operator * (const BvFastVec & v, const BvFastMat & m);
BvFastVec operator * (const BvFastMat & m, const BvFastVec & v);
BvFastVec & operator *= (BvFastVec & v, const BvFastMat & m);

BvFastVec operator / (const BvFastVec & v1, const BvFastVec & v2);
BvFastVec & operator /= (BvFastVec & v1, const BvFastVec & v2);

BvFastVec operator / (const BvFastVec & v, const float s);
BvFastVec & operator /= (BvFastVec & v, const float s);

// ======================
// Specialized Operations
// ======================

BvFastVec VectorTransformDir(const BvFastVec & v, const BvFastMat & m);
BvFastVec VectorTransformPoint(const BvFastVec & v, const BvFastMat & m);

// =================
// Definitions
// =================

// ======================
// Access
// ======================

inline float VectorGetX(const BvFastVec & v)
{
	return v.x;
}

inline float VectorGetY(const BvFastVec & v)
{
	return v.y;
}

inline float VectorGetZ(const BvFastVec & v)
{
	return v.z;
}

inline float VectorGetW(const BvFastVec & v)
{
	return v.w;
}

inline BvFastVec VectorSet(const float x, const float y, const float z, const float w)
{
	return BvFastVec(x, y, z, w);
}

inline BvFastVec VectorReplicate(const float s)
{
	return BvFastVec(s, s, s, s);
}

inline BvFastVec VectorReplicateX(const BvFastVec & v)
{
	return BvFastVec(v.x, v.x, v.x, v.x);
}

inline BvFastVec VectorReplicateY(const BvFastVec & v)
{
	return BvFastVec(v.y, v.y, v.y, v.y);
}

inline BvFastVec VectorReplicateZ(const BvFastVec & v)
{
	return BvFastVec(v.z, v.z, v.z, v.z);
}

inline BvFastVec VectorReplicateW(const BvFastVec & v)
{
	return BvFastVec(v.w, v.w, v.w, v.w);
}

template<unsigned int x, unsigned int y, unsigned int z, unsigned int w>
inline BvFastVec VectorPermute(BvFastVec v)
{
	return BvFastVec(v.v[x], v.v[y], v.v[z], v.v[w]);
}

inline BvFastVec VectorMax(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(v1.x > v2.x ? v1.x : v2.x,
		v1.y > v2.y ? v1.y : v2.y,
		v1.z > v2.z ? v1.z : v2.z,
		v1.w > v2.w ? v1.w : v2.w);
}

inline BvFastVec VectorMin(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(v1.x < v2.x ? v1.x : v2.x,
		v1.y < v2.y ? v1.y : v2.y,
		v1.z < v2.z ? v1.z : v2.z,
		v1.w < v2.w ? v1.w : v2.w);
}

inline BvFastVec VectorAbs(const BvFastVec & v)
{
	return BvFastVec(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w));
}

inline BvFastVec VectorFloor(const BvFastVec & v)
{
	return BvFastVec(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

inline BvFastVec VectorCeil(const BvFastVec & v)
{
	return BvFastVec(ceilf(v.x), ceilf(v.y), ceilf(v.z), ceilf(v.w));
}

inline BvFastVec VectorRound(const BvFastVec & v)
{
	return BvFastVec(roundf(v.x), roundf(v.y), roundf(v.z), roundf(v.w));
}

inline BvFastVec VectorNegate(const BvFastVec & v)
{
	return BvFastVec(-v.x, -v.y, -v.z, -v.w);
}

inline BvFastVec VectorZero()
{
	return BvFastVec();
}

inline BvFastVec VectorIsZeroV(const BvFastVec & v, const float epsilon)
{
	return BvFastVec(static_cast<float>(v.x <= epsilon && v.x >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<float>(v.y <= epsilon && v.y >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<float>(v.z <= epsilon && v.z >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<float>(v.w <= epsilon && v.w >= -epsilon ? 0xFFFFFFFF : 0x0));
}

inline bool VectorIsZero(const BvFastVec & v, const float epsilon)
{
	return (v.x <= epsilon && v.x >= -epsilon
		&& v.y <= epsilon && v.y >= -epsilon
		&& v.z <= epsilon && v.z >= -epsilon
		&& v.w <= epsilon && v.w >= -epsilon);
}

inline BvFastVec VectorIsEqualV(const BvFastVec & v1, const BvFastVec & v2, const float epsilon)
{
	return VectorIsZeroV(VectorSub(v1, v2), epsilon);
}

inline bool VectorIsEqual(const BvFastVec & v1, const BvFastVec & v2, const float epsilon)
{
	return VectorIsZero(VectorSub(v1, v2), epsilon);
}

inline BvFastVec VectorIsExactlyEqualV(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(static_cast<float>(v1.x == v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y == v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z == v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w == v2.w ? 0xFFFFFFFF : 0));
}

inline bool VectorIsExactlyEqual(const BvFastVec & v1, const BvFastVec & v2)
{
	return (v1.x == v2.x
		&& v1.y == v2.y
		&& v1.z == v2.z
		&& v1.w == v2.w);
}

inline BvFastVec VectorGreater(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(static_cast<float>(v1.x > v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y > v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z > v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w > v2.w ? 0xFFFFFFFF : 0));
}

inline BvFastVec VectorGreaterEq(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(static_cast<float>(v1.x >= v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y >= v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z >= v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w >= v2.w ? 0xFFFFFFFF : 0));
}

inline BvFastVec VectorLess(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(static_cast<float>(v1.x < v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y < v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z < v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w < v2.w ? 0xFFFFFFFF : 0));
}

inline BvFastVec VectorLessEq(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(static_cast<float>(v1.x <= v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y <= v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z <= v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w <= v2.w ? 0xFFFFFFFF : 0));
}

inline unsigned int VectorGetMask(const BvFastVec & v)
{
	return (v.x == 0.0f ? 1 : 0)
		| (v.y == 0.0f ? 1 : 0) << 1
		| (v.z == 0.0f ? 1 : 0) << 2
		| (v.w == 0.0f ? 1 : 0) << 3;
}

// ======================
// Basic Operations
// ======================

inline BvFastVec VectorAdd(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

inline BvFastVec VectorSub(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

inline BvFastVec VectorMul(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

inline BvFastVec VectorDiv(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

inline BvFastVec VectorAdd(const BvFastVec & v, const float val)
{
	return BvFastVec(v.x + val, v.y + val, v.z + val, v.w + val);
}

inline BvFastVec VectorSub(const BvFastVec & v, const float val)
{
	return BvFastVec(v.x - val, v.y - val, v.z - val, v.w - val);
}

inline BvFastVec VectorMul(const BvFastVec & v, const float val)
{
	return BvFastVec(v.x * val, v.y * val, v.z * val, v.w * val);
}

inline BvFastVec VectorDiv(const BvFastVec & v, const float val)
{
	return BvFastVec(v.x / val, v.y / val, v.z / val, v.w / val);
}

inline BvFastVec VectorMul(const BvFastVec & v, const BvFastMat & m)
{
	return BvFastVec(v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x + v.w * m.r[3].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y + v.w * m.r[3].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z + v.w * m.r[3].z,
		v.x * m.r[0].w + v.y * m.r[1].w + v.z * m.r[2].w + v.w * m.r[3].w
		);
}

inline BvFastVec VectorMul(const BvFastMat & m, const BvFastVec & v)
{
	return BvFastVec(v.x * m.r[0].x + v.y * m.r[0].y + v.z * m.r[0].z + v.w * m.r[0].w,
		v.x * m.r[1].x + v.y * m.r[1].y + v.z * m.r[1].z + v.w * m.r[1].w,
		v.x * m.r[2].x + v.y * m.r[2].y + v.z * m.r[2].z + v.w * m.r[2].w,
		v.x * m.r[3].x + v.y * m.r[3].y + v.z * m.r[3].z + v.w * m.r[3].w
	);
}

inline float VectorDot(const BvFastVec & v1, const BvFastVec & v2)
{
	return VectorDotV(v1, v2).x;
}

inline BvFastVec VectorDotV(const BvFastVec & v1, const BvFastVec & v2)
{
	float dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return BvFastVec(dot, dot, dot, dot);
}

inline BvFastVec VectorCross(const BvFastVec & v1, const BvFastVec & v2)
{
	return BvFastVec(v1.y * v2.z - v2.y * v1.z,
		v1.z * v2.x - v2.z * v1.x,
		v1.x * v2.y - v2.x * v1.y);
}

inline float VectorLengthSqr(const BvFastVec & v)
{
	return VectorLengthSqrV(v).x;
}

inline BvFastVec VectorLengthSqrV(const BvFastVec & v)
{
	return VectorDotV(v, v);
}

inline float VectorLength(const BvFastVec & v)
{
	return VectorLengthV(v).x;
}

inline BvFastVec VectorLengthV(const BvFastVec & v)
{
	BvFastVec r = VectorLengthSqrV(v);
	r.x = sqrtf(r.x);
	r.y = r.x;
	r.z = r.x;
	r.w = r.x;
	return r;
}

inline BvFastVec VectorNormalize(const BvFastVec & v)
{
	return VectorMul(v, 1.0f / VectorLengthSqr(v));
}

inline BvFastVec VectorProject(const BvFastVec & v, const BvFastVec & p)
{
	// To project on Vector onto another
	// get the dot product between the Vectors
	// and multiply the value by the unit Vector
	// of the Vector to be projected on
	// Projection = (V.P)P
	BvFastVec vDotP = VectorDotV(v, p);
	BvFastVec pN = VectorNormalize(p);
	return VectorMul(vDotP, pN);
}

inline BvFastVec VectorLerp(const BvFastVec & v1, const BvFastVec & v2, float t)
{
	// Lerp = V0 + t * (V1 - V0)
	BvFastVec vT = VectorReplicate(t);
	BvFastVec v2MinusV1 = VectorSub(v2, v1);

	return VectorAdd(v1, VectorMul(vT, v2MinusV1));
}

inline BvFastVec VectorReflection(const BvFastVec & i, const BvFastVec & n)
{
	// Reflection = I - 2(I.N)N
	BvFastVec twoTimesProj = VectorProject(i, n);
	twoTimesProj = VectorMul(twoTimesProj, 2.0f);

	return VectorSub(i, twoTimesProj);
}

// ======================
// Operators
// ======================

inline BvFastVec operator + (const BvFastVec & v)
{
	return v;
}

inline BvFastVec operator - (const BvFastVec & v)
{
	return VectorNegate(v);
}

inline BvFastVec operator + (const BvFastVec & v1, const BvFastVec & v2)
{
	return VectorAdd(v1, v2);
}

inline BvFastVec operator + (const BvFastVec & v, const float s)
{
	return VectorAdd(v, s);
}

inline BvFastVec operator + (const float s, const BvFastVec & v)
{
	return VectorAdd(v, s);
}

inline BvFastVec & operator += (BvFastVec & v1, const BvFastVec & v2)
{
	v1 = VectorAdd(v1, v2);
	return v1;
}

inline BvFastVec operator - (const BvFastVec & v1, const BvFastVec & v2)
{
	return VectorSub(v1, v2);
}

inline BvFastVec operator - (const BvFastVec & v, const float s)
{
	return VectorSub(v, s);
}

inline BvFastVec operator - (const float s, const BvFastVec & v)
{
	return VectorSub(BvFastVec(s, s, s, s), v);
}

inline BvFastVec & operator -= (BvFastVec & v1, const BvFastVec & v2)
{
	v1 = VectorSub(v1, v2);
	return v1;
}

inline BvFastVec operator * (const BvFastVec & v1, const BvFastVec & v2)
{
	return VectorMul(v1, v2);
}

inline BvFastVec & operator *= (BvFastVec & v1, const BvFastVec & v2)
{
	v1 = VectorMul(v1, v2);
	return v1;
}

inline BvFastVec operator * (const BvFastVec & v, const float s)
{
	return VectorMul(v, s);
}

inline BvFastVec operator * (const float s, const BvFastVec & v)
{
	return VectorMul(v, s);
}

inline BvFastVec & operator *= (BvFastVec & v, const float s)
{
	v = VectorMul(v, s);
	return v;
}

inline BvFastVec operator * (const BvFastVec & v, const BvFastMat & m)
{
	return VectorMul(v, m);
}

inline BvFastVec operator*(const BvFastMat & m, const BvFastVec & v)
{
	return VectorMul(m, v);
}

inline BvFastVec operator / (const BvFastVec & v1, const BvFastVec & v2)
{
	return VectorDiv(v1, v2);
}

inline BvFastVec & operator /= (BvFastVec & v1, const BvFastVec & v2)
{
	v1 = VectorDiv(v1, v2);
	return v1;
}

inline BvFastVec operator / (const BvFastVec & v, const float s)
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

inline BvFastVec VectorTransformDir(const BvFastVec & v, const BvFastMat & m)
{
	return BvFastVec(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z,
		0.0f
	);
}

inline BvFastVec VectorTransformPoint(const BvFastVec & v, const BvFastMat & m)
{
	return BvFastVec(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x + m.r[3].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y + m.r[3].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z + m.r[3].z,
		1.0f
	);
}


#endif