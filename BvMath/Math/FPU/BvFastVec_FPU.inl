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

float VectorGetX(const vf32 & v);
float VectorGetY(const vf32 & v);
float VectorGetZ(const vf32 & v);
float VectorGetW(const vf32 & v);

vf32 VectorSet(const float x, const float y, const float z, const float w = 0.0f);

vf32 VectorReplicate(const float s);
vf32 VectorReplicateX(const vf32 & v);
vf32 VectorReplicateY(const vf32 & v);
vf32 VectorReplicateZ(const vf32 & v);
vf32 VectorReplicateW(const vf32 & v);
template<unsigned int x, unsigned int y, unsigned int z, unsigned int w> vf32 VectorPermute(vf32 v);

vf32 VectorMax(const vf32 & v1, const vf32 & v2);
vf32 VectorMin(const vf32 & v1, const vf32 & v2);
vf32 VectorAbs(const vf32 & v);
vf32 VectorFloor(const vf32 &v);
vf32 VectorCeil(const vf32 & v);
vf32 VectorRound(const vf32 & v);

vf32 VectorNegate(const vf32 & v);
vf32 VectorZero();

vf32 VectorIsZeroV(const vf32 & v, const float epsilon = kEpsilon);
bool VectorIsZero(const vf32 & v, const float epsilon = kEpsilon);
vf32 VectorIsEqualV(const vf32 & v1, const vf32 & v2, const float epsilon = kEpsilon);
bool VectorIsEqual(const vf32 & v1, const vf32 & v2, const float epsilon = kEpsilon);
vf32 VectorIsExactlyEqualV(const vf32 & v1, const vf32 & v2);
bool VectorIsExactlyEqual(const vf32 & v1, const vf32 & v2);

vf32 VectorGreater(const vf32 & v1, const vf32 & v2);
vf32 VectorGreaterEqual(const vf32 & v1, const vf32 & v2);
vf32 VectorLess(const vf32 & v1, const vf32 & v2);
vf32 VectorLessEqual(const vf32 & v1, const vf32 & v2);

unsigned int VectorGetMask(const vf32 & v);

// ======================
// Basic Operations
// ======================

vf32 VectorAdd(const vf32 & v1, const vf32 & v2);
vf32 VectorSub(const vf32 & v1, const vf32 & v2);
vf32 VectorMul(const vf32 & v1, const vf32 & v2);
vf32 VectorDiv(const vf32 & v1, const vf32 & v2);

vf32 VectorAdd(const vf32 & v, const float val);
vf32 VectorSub(const vf32 & v, const float val);
vf32 VectorMul(const vf32 & v, const float val);
vf32 VectorDiv(const vf32 & v, const float val);

vf32 VectorMul(const vf32 & v, const mf32 & m); // Row-major
vf32 VectorMul(const mf32 & m, const vf32 & v); // Col-major

float VectorDot(const vf32 & v1, const vf32 & v2);
vf32 VectorDotV(const vf32 & v1, const vf32 & v2);

vf32 VectorCross(const vf32 & v1, const vf32 & v2);

float VectorLengthSqr(const vf32 & v);
vf32 VectorLengthSqrV(const vf32 & v);

float VectorLength(const vf32 & v);
vf32 VectorLengthV(const vf32 & v);

vf32 VectorNormalize(const vf32 & v);

vf32 VectorProject(const vf32 & v1, const vf32 & v2);

vf32 VectorLerp(const vf32 & v1, const vf32 & v2, float t);

vf32 VectorReflection(const vf32 & i, const vf32 & n);

// ======================
// Operators
// ======================

vf32 operator + (const vf32 & v);
vf32 operator - (const vf32 & v);

vf32 operator + (const vf32 & v1, const vf32 & v2);
vf32 operator + (const vf32 & v, const float s);
vf32 operator + (const float s, const vf32 & v);
vf32 & operator += (vf32 & v1, const vf32 & v2);

vf32 operator - (const vf32 & v1, const vf32 & v2);
vf32 operator - (const vf32 & v, const float s);
vf32 operator - (const float s, const vf32 & v);
vf32 & operator -= (vf32 & v1, const vf32 & v2);

vf32 operator * (const vf32 & v1, const vf32 & v2);
vf32 & operator *= (vf32 & v1, const vf32 & v2);

vf32 operator * (const vf32 & v, const float s);
vf32 operator * (const float s, const vf32 & v);
vf32 & operator *= (vf32 & v, const float s);

vf32 operator * (const vf32 & v, const mf32 & m);
vf32 operator * (const mf32 & m, const vf32 & v);
vf32 & operator *= (vf32 & v, const mf32 & m);

vf32 operator / (const vf32 & v1, const vf32 & v2);
vf32 & operator /= (vf32 & v1, const vf32 & v2);

vf32 operator / (const vf32 & v, const float s);
vf32 & operator /= (vf32 & v, const float s);

// ======================
// Specialized Operations
// ======================

vf32 VectorTransformDir(const vf32 & v, const mf32 & m);
vf32 VectorTransformPoint(const vf32 & v, const mf32 & m);

// =================
// Definitions
// =================

// ======================
// Access
// ======================

inline float VectorGetX(const vf32 & v)
{
	return v.x;
}

inline float VectorGetY(const vf32 & v)
{
	return v.y;
}

inline float VectorGetZ(const vf32 & v)
{
	return v.z;
}

inline float VectorGetW(const vf32 & v)
{
	return v.w;
}

inline vf32 VectorSet(const float x, const float y, const float z, const float w)
{
	return vf32(x, y, z, w);
}

inline vf32 VectorReplicate(const float s)
{
	return vf32(s, s, s, s);
}

inline vf32 VectorReplicateX(const vf32 & v)
{
	return vf32(v.x, v.x, v.x, v.x);
}

inline vf32 VectorReplicateY(const vf32 & v)
{
	return vf32(v.y, v.y, v.y, v.y);
}

inline vf32 VectorReplicateZ(const vf32 & v)
{
	return vf32(v.z, v.z, v.z, v.z);
}

inline vf32 VectorReplicateW(const vf32 & v)
{
	return vf32(v.w, v.w, v.w, v.w);
}

template<unsigned int x, unsigned int y, unsigned int z, unsigned int w>
inline vf32 VectorPermute(vf32 v)
{
	return vf32(v.v[x], v.v[y], v.v[z], v.v[w]);
}

inline vf32 VectorMax(const vf32 & v1, const vf32 & v2)
{
	return vf32(v1.x > v2.x ? v1.x : v2.x,
		v1.y > v2.y ? v1.y : v2.y,
		v1.z > v2.z ? v1.z : v2.z,
		v1.w > v2.w ? v1.w : v2.w);
}

inline vf32 VectorMin(const vf32 & v1, const vf32 & v2)
{
	return vf32(v1.x < v2.x ? v1.x : v2.x,
		v1.y < v2.y ? v1.y : v2.y,
		v1.z < v2.z ? v1.z : v2.z,
		v1.w < v2.w ? v1.w : v2.w);
}

inline vf32 VectorAbs(const vf32 & v)
{
	return vf32(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w));
}

inline vf32 VectorFloor(const vf32 & v)
{
	return vf32(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

inline vf32 VectorCeil(const vf32 & v)
{
	return vf32(ceilf(v.x), ceilf(v.y), ceilf(v.z), ceilf(v.w));
}

inline vf32 VectorRound(const vf32 & v)
{
	return vf32(roundf(v.x), roundf(v.y), roundf(v.z), roundf(v.w));
}

inline vf32 VectorNegate(const vf32 & v)
{
	return vf32(-v.x, -v.y, -v.z, -v.w);
}

inline vf32 VectorZero()
{
	return vf32();
}

inline vf32 VectorIsZeroV(const vf32 & v, const float epsilon)
{
	return vf32(static_cast<float>(v.x <= epsilon && v.x >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<float>(v.y <= epsilon && v.y >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<float>(v.z <= epsilon && v.z >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<float>(v.w <= epsilon && v.w >= -epsilon ? 0xFFFFFFFF : 0x0));
}

inline bool VectorIsZero(const vf32 & v, const float epsilon)
{
	return (v.x <= epsilon && v.x >= -epsilon
		&& v.y <= epsilon && v.y >= -epsilon
		&& v.z <= epsilon && v.z >= -epsilon
		&& v.w <= epsilon && v.w >= -epsilon);
}

inline vf32 VectorIsEqualV(const vf32 & v1, const vf32 & v2, const float epsilon)
{
	return VectorIsZeroV(VectorSub(v1, v2), epsilon);
}

inline bool VectorIsEqual(const vf32 & v1, const vf32 & v2, const float epsilon)
{
	return VectorIsZero(VectorSub(v1, v2), epsilon);
}

inline vf32 VectorIsExactlyEqualV(const vf32 & v1, const vf32 & v2)
{
	return vf32(static_cast<float>(v1.x == v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y == v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z == v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w == v2.w ? 0xFFFFFFFF : 0));
}

inline bool VectorIsExactlyEqual(const vf32 & v1, const vf32 & v2)
{
	return (v1.x == v2.x
		&& v1.y == v2.y
		&& v1.z == v2.z
		&& v1.w == v2.w);
}

inline vf32 VectorGreater(const vf32 & v1, const vf32 & v2)
{
	return vf32(static_cast<float>(v1.x > v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y > v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z > v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w > v2.w ? 0xFFFFFFFF : 0));
}

inline vf32 VectorGreaterEqual(const vf32 & v1, const vf32 & v2)
{
	return vf32(static_cast<float>(v1.x >= v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y >= v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z >= v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w >= v2.w ? 0xFFFFFFFF : 0));
}

inline vf32 VectorLess(const vf32 & v1, const vf32 & v2)
{
	return vf32(static_cast<float>(v1.x < v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y < v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z < v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w < v2.w ? 0xFFFFFFFF : 0));
}

inline vf32 VectorLessEqual(const vf32 & v1, const vf32 & v2)
{
	return vf32(static_cast<float>(v1.x <= v2.x ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.y <= v2.y ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.z <= v2.z ? 0xFFFFFFFF : 0),
		static_cast<float>(v1.w <= v2.w ? 0xFFFFFFFF : 0));
}

inline unsigned int VectorGetMask(const vf32 & v)
{
	return (v.x == 0.0f ? 1 : 0)
		| (v.y == 0.0f ? 1 : 0) << 1
		| (v.z == 0.0f ? 1 : 0) << 2
		| (v.w == 0.0f ? 1 : 0) << 3;
}

// ======================
// Basic Operations
// ======================

inline vf32 VectorAdd(const vf32 & v1, const vf32 & v2)
{
	return vf32(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

inline vf32 VectorSub(const vf32 & v1, const vf32 & v2)
{
	return vf32(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

inline vf32 VectorMul(const vf32 & v1, const vf32 & v2)
{
	return vf32(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

inline vf32 VectorDiv(const vf32 & v1, const vf32 & v2)
{
	return vf32(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

inline vf32 VectorAdd(const vf32 & v, const float val)
{
	return vf32(v.x + val, v.y + val, v.z + val, v.w + val);
}

inline vf32 VectorSub(const vf32 & v, const float val)
{
	return vf32(v.x - val, v.y - val, v.z - val, v.w - val);
}

inline vf32 VectorMul(const vf32 & v, const float val)
{
	return vf32(v.x * val, v.y * val, v.z * val, v.w * val);
}

inline vf32 VectorDiv(const vf32 & v, const float val)
{
	return vf32(v.x / val, v.y / val, v.z / val, v.w / val);
}

inline vf32 VectorMul(const vf32 & v, const mf32 & m)
{
	return vf32(v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x + v.w * m.r[3].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y + v.w * m.r[3].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z + v.w * m.r[3].z,
		v.x * m.r[0].w + v.y * m.r[1].w + v.z * m.r[2].w + v.w * m.r[3].w
		);
}

inline vf32 VectorMul(const mf32 & m, const vf32 & v)
{
	return vf32(v.x * m.r[0].x + v.y * m.r[0].y + v.z * m.r[0].z + v.w * m.r[0].w,
		v.x * m.r[1].x + v.y * m.r[1].y + v.z * m.r[1].z + v.w * m.r[1].w,
		v.x * m.r[2].x + v.y * m.r[2].y + v.z * m.r[2].z + v.w * m.r[2].w,
		v.x * m.r[3].x + v.y * m.r[3].y + v.z * m.r[3].z + v.w * m.r[3].w
	);
}

inline float VectorDot(const vf32 & v1, const vf32 & v2)
{
	return VectorDotV(v1, v2).x;
}

inline vf32 VectorDotV(const vf32 & v1, const vf32 & v2)
{
	float dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return vf32(dot, dot, dot, dot);
}

inline vf32 VectorCross(const vf32 & v1, const vf32 & v2)
{
	return vf32(v1.y * v2.z - v2.y * v1.z,
		v1.z * v2.x - v2.z * v1.x,
		v1.x * v2.y - v2.x * v1.y);
}

inline float VectorLengthSqr(const vf32 & v)
{
	return VectorLengthSqrV(v).x;
}

inline vf32 VectorLengthSqrV(const vf32 & v)
{
	return VectorDotV(v, v);
}

inline float VectorLength(const vf32 & v)
{
	return VectorLengthV(v).x;
}

inline vf32 VectorLengthV(const vf32 & v)
{
	vf32 r = VectorLengthSqrV(v);
	r.x = sqrtf(r.x);
	r.y = r.x;
	r.z = r.x;
	r.w = r.x;
	return r;
}

inline vf32 VectorNormalize(const vf32 & v)
{
	return VectorMul(v, 1.0f / VectorLengthSqr(v));
}

inline vf32 VectorProject(const vf32 & v, const vf32 & p)
{
	// To project on Vector onto another
	// get the dot product between the Vectors
	// and multiply the value by the unit Vector
	// of the Vector to be projected on
	// Projection = (V.P)P
	vf32 vDotP = VectorDotV(v, p);
	vf32 pN = VectorNormalize(p);
	return VectorMul(vDotP, pN);
}

inline vf32 VectorLerp(const vf32 & v1, const vf32 & v2, float t)
{
	// Lerp = V0 + t * (V1 - V0)
	vf32 vT = VectorReplicate(t);
	vf32 v2MinusV1 = VectorSub(v2, v1);

	return VectorAdd(v1, VectorMul(vT, v2MinusV1));
}

inline vf32 VectorReflection(const vf32 & i, const vf32 & n)
{
	// Reflection = I - 2(I.N)N
	vf32 twoTimesProj = VectorProject(i, n);
	twoTimesProj = VectorMul(twoTimesProj, 2.0f);

	return VectorSub(i, twoTimesProj);
}

// ======================
// Operators
// ======================

inline vf32 operator + (const vf32 & v)
{
	return v;
}

inline vf32 operator - (const vf32 & v)
{
	return VectorNegate(v);
}

inline vf32 operator + (const vf32 & v1, const vf32 & v2)
{
	return VectorAdd(v1, v2);
}

inline vf32 operator + (const vf32 & v, const float s)
{
	return VectorAdd(v, s);
}

inline vf32 operator + (const float s, const vf32 & v)
{
	return VectorAdd(v, s);
}

inline vf32 & operator += (vf32 & v1, const vf32 & v2)
{
	v1 = VectorAdd(v1, v2);
	return v1;
}

inline vf32 operator - (const vf32 & v1, const vf32 & v2)
{
	return VectorSub(v1, v2);
}

inline vf32 operator - (const vf32 & v, const float s)
{
	return VectorSub(v, s);
}

inline vf32 operator - (const float s, const vf32 & v)
{
	return VectorSub(vf32(s, s, s, s), v);
}

inline vf32 & operator -= (vf32 & v1, const vf32 & v2)
{
	v1 = VectorSub(v1, v2);
	return v1;
}

inline vf32 operator * (const vf32 & v1, const vf32 & v2)
{
	return VectorMul(v1, v2);
}

inline vf32 & operator *= (vf32 & v1, const vf32 & v2)
{
	v1 = VectorMul(v1, v2);
	return v1;
}

inline vf32 operator * (const vf32 & v, const float s)
{
	return VectorMul(v, s);
}

inline vf32 operator * (const float s, const vf32 & v)
{
	return VectorMul(v, s);
}

inline vf32 & operator *= (vf32 & v, const float s)
{
	v = VectorMul(v, s);
	return v;
}

inline vf32 operator * (const vf32 & v, const mf32 & m)
{
	return VectorMul(v, m);
}

inline vf32 operator*(const mf32 & m, const vf32 & v)
{
	return VectorMul(m, v);
}

inline vf32 operator / (const vf32 & v1, const vf32 & v2)
{
	return VectorDiv(v1, v2);
}

inline vf32 & operator /= (vf32 & v1, const vf32 & v2)
{
	v1 = VectorDiv(v1, v2);
	return v1;
}

inline vf32 operator / (const vf32 & v, const float s)
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

inline vf32 VectorTransformDir(const vf32 & v, const mf32 & m)
{
	return vf32(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z,
		0.0f
	);
}

inline vf32 VectorTransformPoint(const vf32 & v, const mf32 & m)
{
	return vf32(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x + m.r[3].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y + m.r[3].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z + m.r[3].z,
		1.0f
	);
}


#endif