#pragma once


#include "BDeV/Core/Math/BvFloatTypes.h"


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_FPU)


// ==================================
// Vector operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Access
// ======================

f32 VectorGetX(crvf32 v);
f32 VectorGetY(crvf32 v);
f32 VectorGetZ(crvf32 v);
f32 VectorGetW(crvf32 v);

vf32 VectorSet(f32 x, f32 y, f32 z, f32 w = 0.0f);

vf32 VectorReplicate(f32 s);
vf32 VectorReplicateX(crvf32 v);
vf32 VectorReplicateY(crvf32 v);
vf32 VectorReplicateZ(crvf32 v);
vf32 VectorReplicateW(crvf32 v);
template<u32 x, u32 y, u32 z, u32 w> vf32 VectorPermute(vf32 v);

vf32 VectorMax(crvf32 v1, crvf32 v2);
vf32 VectorMin(crvf32 v1, crvf32 v2);
vf32 VectorAbs(crvf32 v);
vf32 VectorFloor(crvf32v);
vf32 VectorCeil(crvf32 v);
vf32 VectorRound(crvf32 v);

vf32 VectorNegate(crvf32 v);
vf32 VectorZero();

vf32 VectorIsZeroV(crvf32 v, f32 epsilon = kEpsilon);
bool VectorIsZero(crvf32 v, f32 epsilon = kEpsilon);
vf32 VectorIsEqualV(crvf32 v1, crvf32 v2, f32 epsilon = kEpsilon);
bool VectorIsEqual(crvf32 v1, crvf32 v2, f32 epsilon = kEpsilon);
vf32 VectorIsExactlyEqualV(crvf32 v1, crvf32 v2);
bool VectorIsExactlyEqual(crvf32 v1, crvf32 v2);

vf32 VectorGreater(crvf32 v1, crvf32 v2);
vf32 VectorGreaterEqual(crvf32 v1, crvf32 v2);
vf32 VectorLess(crvf32 v1, crvf32 v2);
vf32 VectorLessEqual(crvf32 v1, crvf32 v2);

u32 VectorGetMask(crvf32 v);

// ======================
// Basic Operations
// ======================

vf32 VectorAdd(crvf32 v1, crvf32 v2);
vf32 VectorSub(crvf32 v1, crvf32 v2);
vf32 VectorMul(crvf32 v1, crvf32 v2);
vf32 VectorDiv(crvf32 v1, crvf32 v2);

vf32 VectorAdd(crvf32 v, f32 val);
vf32 VectorSub(crvf32 v, f32 val);
vf32 VectorMul(crvf32 v, f32 val);
vf32 VectorDiv(crvf32 v, f32 val);

vf32 VectorMul(crvf32 v, const mf32 & m); // Row-major
vf32 VectorMul(const mf32 & m, crvf32 v); // Col-major

f32 VectorDot(crvf32 v1, crvf32 v2);
vf32 VectorDotV(crvf32 v1, crvf32 v2);

vf32 VectorCross(crvf32 v1, crvf32 v2);

f32 VectorLengthSqr(crvf32 v);
vf32 VectorLengthSqrV(crvf32 v);

f32 VectorLength(crvf32 v);
vf32 VectorLengthV(crvf32 v);

vf32 VectorNormalize(crvf32 v);

vf32 VectorProject(crvf32 v1, crvf32 v2);

vf32 VectorLerp(crvf32 v1, crvf32 v2, f32 t);

vf32 VectorReflection(crvf32 i, crvf32 n);

// ======================
// Operators
// ======================

vf32 operator + (crvf32 v);
vf32 operator - (crvf32 v);

vf32 operator + (crvf32 v1, crvf32 v2);
vf32 operator + (crvf32 v, f32 s);
vf32 operator + (f32 s, crvf32 v);
vf32 & operator += (vf32 & v1, crvf32 v2);

vf32 operator - (crvf32 v1, crvf32 v2);
vf32 operator - (crvf32 v, f32 s);
vf32 operator - (f32 s, crvf32 v);
vf32 & operator -= (vf32 & v1, crvf32 v2);

vf32 operator * (crvf32 v1, crvf32 v2);
vf32 & operator *= (vf32 & v1, crvf32 v2);

vf32 operator * (crvf32 v, f32 s);
vf32 operator * (f32 s, crvf32 v);
vf32 & operator *= (vf32 & v, f32 s);

vf32 operator * (crvf32 v, const mf32 & m);
vf32 operator * (const mf32 & m, crvf32 v);
vf32 & operator *= (vf32 & v, const mf32 & m);

vf32 operator / (crvf32 v1, crvf32 v2);
vf32 & operator /= (vf32 & v1, crvf32 v2);

vf32 operator / (crvf32 v, f32 s);
vf32 & operator /= (vf32 & v, f32 s);

// ======================
// Specialized Operations
// ======================

vf32 VectorTransformDir(crvf32 v, const mf32 & m);
vf32 VectorTransformPoint(crvf32 v, const mf32 & m);

// =================
// Definitions
// =================

// ======================
// Access
// ======================

inline f32 VectorGetX(crvf32 v)
{
	return v.x;
}

inline f32 VectorGetY(crvf32 v)
{
	return v.y;
}

inline f32 VectorGetZ(crvf32 v)
{
	return v.z;
}

inline f32 VectorGetW(crvf32 v)
{
	return v.w;
}

inline vf32 VectorSet(f32 x, f32 y, f32 z, f32 w)
{
	return vf32(x, y, z, w);
}

inline vf32 VectorReplicate(f32 s)
{
	return vf32(s, s, s, s);
}

inline vf32 VectorReplicateX(crvf32 v)
{
	return vf32(v.x, v.x, v.x, v.x);
}

inline vf32 VectorReplicateY(crvf32 v)
{
	return vf32(v.y, v.y, v.y, v.y);
}

inline vf32 VectorReplicateZ(crvf32 v)
{
	return vf32(v.z, v.z, v.z, v.z);
}

inline vf32 VectorReplicateW(crvf32 v)
{
	return vf32(v.w, v.w, v.w, v.w);
}

template<u32 x, u32 y, u32 z, u32 w>
inline vf32 VectorPermute(vf32 v)
{
	return vf32(v.v[x], v.v[y], v.v[z], v.v[w]);
}

inline vf32 VectorMax(crvf32 v1, crvf32 v2)
{
	return vf32(v1.x > v2.x ? v1.x : v2.x,
		v1.y > v2.y ? v1.y : v2.y,
		v1.z > v2.z ? v1.z : v2.z,
		v1.w > v2.w ? v1.w : v2.w);
}

inline vf32 VectorMin(crvf32 v1, crvf32 v2)
{
	return vf32(v1.x < v2.x ? v1.x : v2.x,
		v1.y < v2.y ? v1.y : v2.y,
		v1.z < v2.z ? v1.z : v2.z,
		v1.w < v2.w ? v1.w : v2.w);
}

inline vf32 VectorAbs(crvf32 v)
{
	return vf32(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w));
}

inline vf32 VectorFloor(crvf32 v)
{
	return vf32(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}

inline vf32 VectorCeil(crvf32 v)
{
	return vf32(ceilf(v.x), ceilf(v.y), ceilf(v.z), ceilf(v.w));
}

inline vf32 VectorRound(crvf32 v)
{
	return vf32(roundf(v.x), roundf(v.y), roundf(v.z), roundf(v.w));
}

inline vf32 VectorNegate(crvf32 v)
{
	return vf32(-v.x, -v.y, -v.z, -v.w);
}

inline vf32 VectorZero()
{
	return vf32();
}

inline vf32 VectorIsZeroV(crvf32 v, f32 epsilon)
{
	return vf32(static_cast<f32>(v.x <= epsilon && v.x >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<f32>(v.y <= epsilon && v.y >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<f32>(v.z <= epsilon && v.z >= -epsilon ? 0xFFFFFFFF : 0x0),
		static_cast<f32>(v.w <= epsilon && v.w >= -epsilon ? 0xFFFFFFFF : 0x0));
}

inline bool VectorIsZero(crvf32 v, f32 epsilon)
{
	return (v.x <= epsilon && v.x >= -epsilon
		&& v.y <= epsilon && v.y >= -epsilon
		&& v.z <= epsilon && v.z >= -epsilon
		&& v.w <= epsilon && v.w >= -epsilon);
}

inline vf32 VectorIsEqualV(crvf32 v1, crvf32 v2, f32 epsilon)
{
	return VectorIsZeroV(VectorSub(v1, v2), epsilon);
}

inline bool VectorIsEqual(crvf32 v1, crvf32 v2, f32 epsilon)
{
	return VectorIsZero(VectorSub(v1, v2), epsilon);
}

inline vf32 VectorIsExactlyEqualV(crvf32 v1, crvf32 v2)
{
	return vf32(static_cast<f32>(v1.x == v2.x ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.y == v2.y ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.z == v2.z ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.w == v2.w ? 0xFFFFFFFF : 0));
}

inline bool VectorIsExactlyEqual(crvf32 v1, crvf32 v2)
{
	return (v1.x == v2.x
		&& v1.y == v2.y
		&& v1.z == v2.z
		&& v1.w == v2.w);
}

inline vf32 VectorGreater(crvf32 v1, crvf32 v2)
{
	return vf32(static_cast<f32>(v1.x > v2.x ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.y > v2.y ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.z > v2.z ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.w > v2.w ? 0xFFFFFFFF : 0));
}

inline vf32 VectorGreaterEqual(crvf32 v1, crvf32 v2)
{
	return vf32(static_cast<f32>(v1.x >= v2.x ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.y >= v2.y ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.z >= v2.z ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.w >= v2.w ? 0xFFFFFFFF : 0));
}

inline vf32 VectorLess(crvf32 v1, crvf32 v2)
{
	return vf32(static_cast<f32>(v1.x < v2.x ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.y < v2.y ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.z < v2.z ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.w < v2.w ? 0xFFFFFFFF : 0));
}

inline vf32 VectorLessEqual(crvf32 v1, crvf32 v2)
{
	return vf32(static_cast<f32>(v1.x <= v2.x ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.y <= v2.y ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.z <= v2.z ? 0xFFFFFFFF : 0),
		static_cast<f32>(v1.w <= v2.w ? 0xFFFFFFFF : 0));
}

inline u32 VectorGetMask(crvf32 v)
{
	return (v.x == 0.0f ? 1 : 0)
		| (v.y == 0.0f ? 1 : 0) << 1
		| (v.z == 0.0f ? 1 : 0) << 2
		| (v.w == 0.0f ? 1 : 0) << 3;
}

// ======================
// Basic Operations
// ======================

inline vf32 VectorAdd(crvf32 v1, crvf32 v2)
{
	return vf32(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

inline vf32 VectorSub(crvf32 v1, crvf32 v2)
{
	return vf32(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

inline vf32 VectorMul(crvf32 v1, crvf32 v2)
{
	return vf32(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

inline vf32 VectorDiv(crvf32 v1, crvf32 v2)
{
	return vf32(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

inline vf32 VectorAdd(crvf32 v, f32 val)
{
	return vf32(v.x + val, v.y + val, v.z + val, v.w + val);
}

inline vf32 VectorSub(crvf32 v, f32 val)
{
	return vf32(v.x - val, v.y - val, v.z - val, v.w - val);
}

inline vf32 VectorMul(crvf32 v, f32 val)
{
	return vf32(v.x * val, v.y * val, v.z * val, v.w * val);
}

inline vf32 VectorDiv(crvf32 v, f32 val)
{
	return vf32(v.x / val, v.y / val, v.z / val, v.w / val);
}

inline vf32 VectorMul(crvf32 v, const mf32 & m)
{
	return vf32(v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x + v.w * m.r[3].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y + v.w * m.r[3].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z + v.w * m.r[3].z,
		v.x * m.r[0].w + v.y * m.r[1].w + v.z * m.r[2].w + v.w * m.r[3].w
		);
}

inline vf32 VectorMul(const mf32 & m, crvf32 v)
{
	return vf32(v.x * m.r[0].x + v.y * m.r[0].y + v.z * m.r[0].z + v.w * m.r[0].w,
		v.x * m.r[1].x + v.y * m.r[1].y + v.z * m.r[1].z + v.w * m.r[1].w,
		v.x * m.r[2].x + v.y * m.r[2].y + v.z * m.r[2].z + v.w * m.r[2].w,
		v.x * m.r[3].x + v.y * m.r[3].y + v.z * m.r[3].z + v.w * m.r[3].w
	);
}

inline f32 VectorDot(crvf32 v1, crvf32 v2)
{
	return VectorDotV(v1, v2).x;
}

inline vf32 VectorDotV(crvf32 v1, crvf32 v2)
{
	f32 dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return vf32(dot, dot, dot, dot);
}

inline vf32 VectorCross(crvf32 v1, crvf32 v2)
{
	return vf32(v1.y * v2.z - v2.y * v1.z,
		v1.z * v2.x - v2.z * v1.x,
		v1.x * v2.y - v2.x * v1.y);
}

inline f32 VectorLengthSqr(crvf32 v)
{
	return VectorLengthSqrV(v).x;
}

inline vf32 VectorLengthSqrV(crvf32 v)
{
	return VectorDotV(v, v);
}

inline f32 VectorLength(crvf32 v)
{
	return VectorLengthV(v).x;
}

inline vf32 VectorLengthV(crvf32 v)
{
	vf32 r = VectorLengthSqrV(v);
	r.x = sqrtf(r.x);
	r.y = r.x;
	r.z = r.x;
	r.w = r.x;
	return r;
}

inline vf32 VectorNormalize(crvf32 v)
{
	return VectorMul(v, 1.0f / VectorLengthSqr(v));
}

inline vf32 VectorProject(crvf32 v, crvf32 p)
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

inline vf32 VectorLerp(crvf32 v1, crvf32 v2, f32 t)
{
	// Lerp = V0 + t * (V1 - V0)
	vf32 vT = VectorReplicate(t);
	vf32 v2MinusV1 = VectorSub(v2, v1);

	return VectorAdd(v1, VectorMul(vT, v2MinusV1));
}

inline vf32 VectorReflection(crvf32 i, crvf32 n)
{
	// Reflection = I - 2(I.N)N
	vf32 twoTimesProj = VectorProject(i, n);
	twoTimesProj = VectorMul(twoTimesProj, 2.0f);

	return VectorSub(i, twoTimesProj);
}

// ======================
// Operators
// ======================

inline vf32 operator + (crvf32 v)
{
	return v;
}

inline vf32 operator - (crvf32 v)
{
	return VectorNegate(v);
}

inline vf32 operator + (crvf32 v1, crvf32 v2)
{
	return VectorAdd(v1, v2);
}

inline vf32 operator + (crvf32 v, f32 s)
{
	return VectorAdd(v, s);
}

inline vf32 operator + (f32 s, crvf32 v)
{
	return VectorAdd(v, s);
}

inline vf32 & operator += (vf32 & v1, crvf32 v2)
{
	v1 = VectorAdd(v1, v2);
	return v1;
}

inline vf32 operator - (crvf32 v1, crvf32 v2)
{
	return VectorSub(v1, v2);
}

inline vf32 operator - (crvf32 v, f32 s)
{
	return VectorSub(v, s);
}

inline vf32 operator - (f32 s, crvf32 v)
{
	return VectorSub(vf32(s, s, s, s), v);
}

inline vf32 & operator -= (vf32 & v1, crvf32 v2)
{
	v1 = VectorSub(v1, v2);
	return v1;
}

inline vf32 operator * (crvf32 v1, crvf32 v2)
{
	return VectorMul(v1, v2);
}

inline vf32 & operator *= (vf32 & v1, crvf32 v2)
{
	v1 = VectorMul(v1, v2);
	return v1;
}

inline vf32 operator * (crvf32 v, f32 s)
{
	return VectorMul(v, s);
}

inline vf32 operator * (f32 s, crvf32 v)
{
	return VectorMul(v, s);
}

inline vf32 & operator *= (vf32 & v, f32 s)
{
	v = VectorMul(v, s);
	return v;
}

inline vf32 operator * (crvf32 v, const mf32 & m)
{
	return VectorMul(v, m);
}

inline vf32 operator*(const mf32 & m, crvf32 v)
{
	return VectorMul(m, v);
}

inline vf32 operator / (crvf32 v1, crvf32 v2)
{
	return VectorDiv(v1, v2);
}

inline vf32 & operator /= (vf32 & v1, crvf32 v2)
{
	v1 = VectorDiv(v1, v2);
	return v1;
}

inline vf32 operator / (crvf32 v, f32 s)
{
	return VectorDiv(v, s);
}

inline vf32 & operator /= (vf32 & v, f32 s)
{
	v = VectorDiv(v, s);
	return v;
}

// ======================
// Specialized Operations
// ======================

inline vf32 VectorTransformDir(crvf32 v, const mf32 & m)
{
	return vf32(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z,
		0.0f
	);
}

inline vf32 VectorTransformPoint(crvf32 v, const mf32 & m)
{
	return vf32(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x + m.r[3].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y + m.r[3].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z + m.r[3].z,
		1.0f
	);
}


#endif