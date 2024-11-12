#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"


// Default floating point error threshold
constexpr float kEpsilon = 0.0001f;

constexpr float kPi      = 3.141592653589793238462f;
constexpr float k1DivPi	 = 1.0f / kPi;
constexpr float k2Pi	 = kPi * 2.0f; //6.283185482025146484375f;
constexpr float k1Div2Pi = 1.0f / k2Pi;
constexpr float k3PiDiv2 = kPi * 1.5f; //4.712388992309570312500f;

constexpr float kPiDiv2  = kPi / 2.0f; //1.570796370506286621094f;
constexpr float kPiDiv3  = kPi / 3.0f; //1.047197580337524414063f;
constexpr float kPiDiv4  = kPi / 4.0f; //0.785398163397448309616f;
constexpr float kPiDiv5  = kPi / 5.0f; //0.628318548202514648438f;
constexpr float kPiDiv6  = kPi / 6.0f; //0.523598790168762207031f;
constexpr float kPiDiv8  = kPi / 8.0f; //0.392699092626571655273f;
constexpr float kPiDiv10 = kPi / 10.0f; //0.314159274101257324219f;
constexpr float kPiDiv12 = kPi / 12.0f; //0.261799395084381103516f;
constexpr float kPiDiv16 = kPi / 16.0f; //0.196349546313285827637f;


struct Float2
{
	union
	{
		struct { f32 x, y; };
		f32 v[2];
	};

	constexpr Float2() : x(0.0f), y(0.0f) {}
	constexpr Float2(f32 x, f32 y)
		: x(x), y(y) {}
};


struct Float3
{
	union
	{
		struct { f32 x, y, z; };
		f32 v[3];
		Float2 v2;
	};

	constexpr Float3() : x(0.0f), y(0.0f), z(0.0f) {}
	constexpr Float3(f32 x, f32 y, f32 z)
		: x(x), y(y), z(z) {}
};


struct Float4
{
	union
	{
		struct { f32 x, y, z, w; };
		f32 v[4];
		Float2 v2;
		Float3 v3;
	};

	constexpr Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	constexpr Float4(f32 x, f32 y, f32 z, f32 w = 0.0f)
		: x(x), y(y), z(z), w(w) {}
};


struct alignas(16) Float4A : Float4
{
	using Float4::Float4;
};


struct Float22
{
	union
	{
		Float2 r[2]{};
		f32 m[4];
		struct
		{
			f32 m0, m1,
				m2, m3;
		};
	};

	constexpr Float22() {}
	constexpr Float22(const Float2& r0, const Float2& r1)
	{
		r[0] = r0;
		r[1] = r1;
	}
};


struct Float33
{
	union
	{
		Float3 r[3]{};
		f32 m[9];
		struct
		{
			f32 m0, m1, m2,
				m3, m4, m5,
				m6, m7, m8;
		};
	};

	constexpr Float33() {}
	constexpr Float33(const Float3& r0, const Float3& r1, const Float3& r2)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
	}
};


struct Float43
{
	union
	{
		Float3 r[4]{};
		f32 m[12];
		struct
		{
			f32 m0, m1, m2,
				m3, m4, m5,
				m6, m7, m8,
				m9, m10, m11;
		};
	};

	constexpr Float43() {}
	constexpr Float43(const Float3& r0, const Float3& r1, const Float3& r2, const Float3& r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}
};


struct Float44
{
	union
	{
		Float4 r[4]{};
		f32 m[16];
		struct
		{
			f32 m0, m1, m2, m3,
				m4, m5, m6, m7,
				m8, m9, m10, m11,
				m12, m13, m14, m15;
		};
	};

	constexpr Float44() {}
	constexpr Float44(const Float4& r0, const Float4& r1, const Float4& r2, const Float4& r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}
};


struct alignas(16) Float44A : Float44
{
	using Float44::Float44;
};


#if (BV_COMPILER == BV_COMPILER_MSVC)
#define BV_VCALL __vectorcall
#else
#define BV_VCALL
#endif


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_AVX)
//#include <mmintrin.h>  // MMX
//#include <xmmintrin.h> // SSE
//#include <emmintrin.h> // SSE2
//#include <pmmintrin.h> // SSE3
//#include <tmmintrin.h> // SSSE3
//#include <smmintrin.h> // SSE4.1
//#include <nmmintrin.h> // SSE4.2
//#include <ammintrin.h> // SSE4A
//#include <wmmintrin.h> // AES
#include <immintrin.h> // AVX
//#include <zmmintrin.h> // AVX512

using vf32 = __m128;
using vi32 = __m128i;

using cvf32 = __m128;

#else
using vf32 = Float4A;
using vi32 = i32[4];

using cvf32 = const vf32&;
#endif


struct mf32
{
	mf32() = default;
	mf32(const mf32&) = default;
	mf32& operator=(const mf32&) = default;
	mf32(mf32&&) = default;
	mf32& operator=(mf32&&) = default;
	BV_INLINE mf32(cvf32 r0, cvf32 r1, cvf32 r2, cvf32 r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}

	vf32 r[4];
};


using cmf32 = const mf32&;


vf32 BV_VCALL Load1(const f32* p);
vf32 BV_VCALL Load2(const f32* p);
vf32 BV_VCALL Load3(const f32* p);
vf32 BV_VCALL Load4(const f32* p);
vf32 BV_VCALL Load4A(const f32* p);
void BV_VCALL Store1(cvf32 v, f32* p);
void BV_VCALL Store2(cvf32 v, f32* p);
void BV_VCALL Store3(cvf32 v, f32* p);
void BV_VCALL Store4(cvf32 v, f32* p);
void BV_VCALL Store4A(cvf32 v, f32* p);
mf32 BV_VCALL Load22(const f32* p);
mf32 BV_VCALL Load33(const f32* p);
mf32 BV_VCALL Load43(const f32* p);
mf32 BV_VCALL Load44(const f32* p);
mf32 BV_VCALL Load44A(const f32* p);
void BV_VCALL Store22(const mf32& m, f32* p);
void BV_VCALL Store33(const mf32& m, f32* p);
void BV_VCALL Store43(const mf32& m, f32* p);
void BV_VCALL Store44(const mf32& m, f32* p);
void BV_VCALL Store44A(const mf32& m, f32* p);

BV_INLINE vf32 BV_VCALL Load(f32 f) { return Load1(&f); }
BV_INLINE vf32 BV_VCALL Load(const Float2& f) { return Load2(f.v); }
BV_INLINE vf32 BV_VCALL Load(const Float3& f) { return Load3(f.v); }
BV_INLINE vf32 BV_VCALL Load(const Float4& f) { return Load4(f.v); }
BV_INLINE vf32 BV_VCALL Load(const Float4A& f) { return Load4A(f.v); }
BV_INLINE void BV_VCALL Store(cvf32 v, f32& f) { Store1(v, &f); }
BV_INLINE void BV_VCALL Store(cvf32 v, Float2& f) { Store2(v, f.v); }
BV_INLINE void BV_VCALL Store(cvf32 v, Float3& f) { Store3(v, f.v); }
BV_INLINE void BV_VCALL Store(cvf32 v, Float4& f) { Store4(v, f.v); }
BV_INLINE void BV_VCALL Store(cvf32 v, Float4A& f) { Store4A(v, f.v); }
BV_INLINE mf32 BV_VCALL Load(const Float22& r) { return Load22(r.m); }
BV_INLINE mf32 BV_VCALL Load(const Float33& r) { return Load33(r.m); }
BV_INLINE mf32 BV_VCALL Load(const Float43& r) { return Load43(r.m); }
BV_INLINE mf32 BV_VCALL Load(const Float44& r) { return Load44(r.m); }
BV_INLINE mf32 BV_VCALL Load(const Float44A& r) { return Load44A(r.m); }
BV_INLINE void BV_VCALL Store(cmf32 m, Float22& r) { Store22(m, r.m); }
BV_INLINE void BV_VCALL Store(cmf32 m, Float33& r) { Store33(m, r.m); }
BV_INLINE void BV_VCALL Store(cmf32 m, Float43& r) { Store43(m, r.m); }
BV_INLINE void BV_VCALL Store(cmf32 m, Float44& r) { Store44(m, r.m); }
BV_INLINE void BV_VCALL Store(cmf32 m, Float44A& r) { Store44A(m, r.m); }


f32 BV_VCALL VectorGetX(cvf32 v);
f32 BV_VCALL VectorGetY(cvf32 v);
f32 BV_VCALL VectorGetZ(cvf32 v);
f32 BV_VCALL VectorGetW(cvf32 v);
vf32 BV_VCALL VectorSetX(cvf32 v, f32 s);
vf32 BV_VCALL VectorSetY(cvf32 v, f32 s);
vf32 BV_VCALL VectorSetZ(cvf32 v, f32 s);
vf32 BV_VCALL VectorSetW(cvf32 v, f32 s);
vf32 BV_VCALL VectorSet(f32 s);
vf32 BV_VCALL VectorSet(f32 x, f32 y, f32 z, f32 w);
vf32 BV_VCALL VectorReplicateX(cvf32 v);
vf32 BV_VCALL VectorReplicateY(cvf32 v);
vf32 BV_VCALL VectorReplicateZ(cvf32 v);
vf32 BV_VCALL VectorReplicateW(cvf32 v);
vf32 BV_VCALL VectorMaskX();
vf32 BV_VCALL VectorMaskY();
vf32 BV_VCALL VectorMaskZ();
vf32 BV_VCALL VectorMaskW();
vf32 BV_VCALL VectorMaskAll();
vf32 BV_VCALL VectorMaskInvX();
vf32 BV_VCALL VectorMaskInvY();
vf32 BV_VCALL VectorMaskInvZ();
vf32 BV_VCALL VectorMaskInvW();
template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorMask();
template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorPermute(cvf32 v);
vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2, i32 mask);
vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2, cvf32 mask);
template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorMax(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorMin(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorClamp(cvf32 v, cvf32 min, cvf32 max);
vf32 BV_VCALL VectorAbs(cvf32 v);
vf32 BV_VCALL VectorFloor(cvf32 v);
vf32 BV_VCALL VectorCeil(cvf32 v);
vf32 BV_VCALL VectorRound(cvf32 v);
vf32 BV_VCALL VectorSqrt(cvf32 v);
vf32 BV_VCALL VectorRSqrt(cvf32 v);
vf32 BV_VCALL VectorRcp(cvf32 v);
vf32 BV_VCALL VectorNegate(cvf32 v);
template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorChangeSign(cvf32 v);
vf32 BV_VCALL VectorZero();
vf32 BV_VCALL VectorOne();
vf32 BV_VCALL VectorUnitX();
vf32 BV_VCALL VectorUnitY();
vf32 BV_VCALL VectorUnitZ();
vf32 BV_VCALL VectorUnitW();
vf32 BV_VCALL Vector2Equal(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector2NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
vf32 BV_VCALL Vector2Greater(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector2GreaterEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector2Less(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector2LessEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector3Equal(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector3NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
vf32 BV_VCALL Vector3Greater(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector3GreaterEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector3Less(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector3LessEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector4Equal(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector4NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
vf32 BV_VCALL Vector4Greater(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector4GreaterEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector4Less(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector4LessEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorNearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
vf32 BV_VCALL VectorGreater(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorGreaterEqual(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorLess(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorLessEqual(cvf32 v1, cvf32 v2);
bool BV_VCALL Vector2AllTrue(cvf32 v);
bool BV_VCALL Vector2AllFalse(cvf32 v);
bool BV_VCALL Vector2AnyTrue(cvf32 v);
bool BV_VCALL Vector2AnyFalse(cvf32 v);
bool BV_VCALL Vector3AllTrue(cvf32 v);
bool BV_VCALL Vector3AllFalse(cvf32 v);
bool BV_VCALL Vector3AnyTrue(cvf32 v);
bool BV_VCALL Vector3AnyFalse(cvf32 v);
bool BV_VCALL VectorAllTrue(cvf32 v);
bool BV_VCALL VectorAllFalse(cvf32 v);
bool BV_VCALL VectorAnyTrue(cvf32 v);
bool BV_VCALL VectorAnyFalse(cvf32 v);
vf32 BV_VCALL VectorOr(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorAnd(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorXor(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorNot(cvf32 v);
vf32 BV_VCALL VectorAndNot(cvf32 v1, cvf32 v2);
i32 BV_VCALL VectorGetMask(cvf32 v);
vf32 BV_VCALL VectorAdd(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorSub(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorMul(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorDiv(cvf32 v1, cvf32 v2);
vf32 BV_VCALL VectorMAdd(cvf32 v1, cvf32 v2, cvf32 v3);
vf32 BV_VCALL VectorNMAdd(cvf32 v1, cvf32 v2, cvf32 v3);
vf32 BV_VCALL VectorMSub(cvf32 v1, cvf32 v2, cvf32 v3);
vf32 BV_VCALL VectorNMSub(cvf32 v1, cvf32 v2, cvf32 v3);
vf32 BV_VCALL VectorAdd(cvf32 v, f32 val);
vf32 BV_VCALL VectorSub(cvf32 v, f32 val);
vf32 BV_VCALL VectorMul(cvf32 v, f32 val);
vf32 BV_VCALL VectorDiv(cvf32 v, f32 val);
vf32 BV_VCALL VectorAdd(f32 val, cvf32 v);
vf32 BV_VCALL VectorSub(f32 val, cvf32 v);
vf32 BV_VCALL VectorMul(f32 val, cvf32 v);
vf32 BV_VCALL VectorDiv(f32 val, cvf32 v);
vf32 BV_VCALL VectorLerp(cvf32 v1, cvf32 v2, f32 t);
vf32 BV_VCALL Vector2Dot(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector2Cross(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector2LengthSqr(cvf32 v);
vf32 BV_VCALL Vector2Length(cvf32 v);
vf32 BV_VCALL Vector2LengthRcp(cvf32 v);
vf32 BV_VCALL Vector2Normalize(cvf32 v);
vf32 BV_VCALL Vector2Rotate(cvf32 v, f32 rad);
vf32 BV_VCALL Vector2InvRotate(cvf32 v, f32 rad);
vf32 BV_VCALL Vector2TransformDir(cvf32 v, cmf32 m);
vf32 BV_VCALL Vector2TransformPoint(cvf32 v, cmf32 m);
vf32 BV_VCALL Vector3Dot(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector3Cross(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector3LengthSqr(cvf32 v);
vf32 BV_VCALL Vector3Length(cvf32 v);
vf32 BV_VCALL Vector3LengthRcp(cvf32 v);
vf32 BV_VCALL Vector3Normalize(cvf32 v);
vf32 BV_VCALL Vector3Project(cvf32 v, cvf32 p);
vf32 BV_VCALL Vector3ProjectNormal(cvf32 v, cvf32 p);
vf32 BV_VCALL Vector3Reflection(cvf32 i, cvf32 n);
vf32 BV_VCALL Vector3Refraction(cvf32 i, cvf32 n, f32 eta);
vf32 BV_VCALL Vector3Rotate(cvf32 v, cvf32 q);
vf32 BV_VCALL Vector3InvRotate(cvf32 v, cvf32 q);
vf32 BV_VCALL Vector3TransformDir(cvf32 v, cmf32 m);
vf32 BV_VCALL Vector3TransformPoint(cvf32 v, cmf32 m);
vf32 BV_VCALL Vector4Dot(cvf32 v1, cvf32 v2);
vf32 BV_VCALL Vector4LengthSqr(cvf32 v);
vf32 BV_VCALL Vector4Length(cvf32 v);
vf32 BV_VCALL Vector4LengthRcp(cvf32 v);
vf32 BV_VCALL VectorTransform(cvf32 v, cmf32 m);

vf32 BV_VCALL operator+(cvf32 v);
vf32 BV_VCALL operator-(cvf32 v);
vf32 BV_VCALL operator+(cvf32 v1, cvf32 v2);
vf32 BV_VCALL operator+(cvf32 v, f32 s);
vf32 BV_VCALL operator+(f32 s, cvf32 v);
vf32& BV_VCALL operator+=(vf32& v1, cvf32 v2);
vf32& BV_VCALL operator+=(vf32& v, f32 s);
vf32 BV_VCALL operator-(cvf32 v1, cvf32 v2);
vf32 BV_VCALL operator-(cvf32 v, f32 s);
vf32 BV_VCALL operator-(f32 s, cvf32 v);
vf32& BV_VCALL operator-=(vf32& v1, cvf32 v2);
vf32& BV_VCALL operator-=(vf32& v, f32 s);
vf32 BV_VCALL operator*(cvf32 v1, cvf32 v2);
vf32& BV_VCALL operator*=(vf32& v1, cvf32 v2);
vf32 BV_VCALL operator*(cvf32 v, f32 s);
vf32 BV_VCALL operator*(f32 s, cvf32 v);
vf32& BV_VCALL operator*=(vf32& v, f32 s);
vf32 BV_VCALL operator*(cvf32 v, cmf32 m);
vf32& BV_VCALL operator*=(vf32& v, cmf32 m);
vf32 BV_VCALL operator/(cvf32 v1, cvf32 v2);
vf32& BV_VCALL operator/=(vf32& v1, cvf32 v2);
vf32 BV_VCALL operator/(cvf32 v, f32 s);
vf32 BV_VCALL operator/(f32 s, cvf32 v);
vf32& BV_VCALL operator/=(vf32& v, f32 s);

mf32 BV_VCALL MatrixSet(cvf32 v0, cvf32 v1, cvf32 v2, cvf32 v3);
mf32 BV_VCALL MatrixAdd(cmf32 m1, cmf32 m2);
mf32 BV_VCALL MatrixAdd(cmf32 m, f32 s);
mf32 BV_VCALL MatrixSub(cmf32 m1, cmf32 m2);
mf32 BV_VCALL MatrixSub(cmf32 m, f32 s);
mf32 BV_VCALL MatrixMul(cmf32 m1, cmf32 m2);
mf32 BV_VCALL MatrixMul(cmf32 m, f32 s);
mf32 BV_VCALL MatrixDiv(cmf32 m, f32 s);
mf32 BV_VCALL MatrixIdentity();
mf32 BV_VCALL MatrixTranspose(cmf32 m);
vf32 BV_VCALL MatrixDeterminant(cmf32 m);
mf32 BV_VCALL MatrixInverse(cmf32 m, vf32& det);
mf32 BV_VCALL MatrixInverse(cmf32 m);
mf32 BV_VCALL MatrixScaling(cvf32 s);
mf32 BV_VCALL MatrixScaling(f32 sX, f32 sY, f32 sZ);
mf32 BV_VCALL MatrixTranslation(cvf32 t);
mf32 BV_VCALL MatrixTranslation(f32 tX, f32 tY, f32 tZ);
mf32 BV_VCALL MatrixRotationX(f32 angle);
mf32 BV_VCALL MatrixRotationY(f32 angle);
mf32 BV_VCALL MatrixRotationZ(f32 angle);
mf32 BV_VCALL MatrixRotationAxis(cvf32 axis, f32 angle);
mf32 BV_VCALL MatrixFromQuaternion(cvf32 q);
mf32 BV_VCALL MatrixLookAt(cvf32 eyePos, cvf32 dirVec, cvf32 upVec);
mf32 BV_VCALL MatrixLookAtLH(cvf32 eyePos, cvf32 lookPos, cvf32 upVec);
mf32 BV_VCALL MatrixLookAtRH(cvf32 eyePos, cvf32 lookPos, cvf32 upVec);
mf32 BV_VCALL MatrixPerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 BV_VCALL MatrixPerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 BV_VCALL MatrixPerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 BV_VCALL MatrixPerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 BV_VCALL MatrixPerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 BV_VCALL MatrixPerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 BV_VCALL MatrixOrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
mf32 BV_VCALL MatrixOrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
mf32 BV_VCALL MatrixOrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
mf32 BV_VCALL MatrixOrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
mf32 BV_VCALL MatrixOrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
mf32 BV_VCALL MatrixOrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);

mf32  BV_VCALL operator+(cmf32 m);
mf32  BV_VCALL operator-(cmf32 m);
mf32  BV_VCALL operator+(cmf32 m1, cmf32 m2);
mf32& BV_VCALL operator+=(mf32& m1, cmf32 m2);
mf32  BV_VCALL operator-(cmf32 m1, cmf32 m2);
mf32& BV_VCALL operator-=(mf32& m1, cmf32 m2);
mf32  BV_VCALL operator*(cmf32 m1, cmf32 m2);
mf32& BV_VCALL operator*=(mf32& m1, cmf32 m2);
mf32  BV_VCALL operator+(cmf32 m, f32 s);
mf32& BV_VCALL operator+=(mf32& m, f32 s);
mf32  BV_VCALL operator-(cmf32 m, f32 s);
mf32& BV_VCALL operator-=(mf32& m, f32 s);
mf32  BV_VCALL operator*(cmf32 m, f32 s);
mf32& BV_VCALL operator*=(mf32& m, f32 s);
mf32  BV_VCALL operator/(cmf32 m, f32 s);
mf32& BV_VCALL operator/=(mf32& m, f32 s);

vf32 BV_VCALL QuaternionIdentity();
vf32 BV_VCALL QuaternionMul(cvf32 q1, cvf32 q2);
vf32 BV_VCALL QuaternionMulKeenan(cvf32 q1, cvf32 q2);
vf32 BV_VCALL QuaternionConjugate(cvf32 q);
vf32 BV_VCALL QuaternionInverse(cvf32 q);
vf32 BV_VCALL QuaternionInverseN(cvf32 q);
vf32 BV_VCALL QuaternionNormalize(cvf32 q);
vf32 BV_VCALL QuaternionDot(cvf32 q1, cvf32 q2);
vf32 BV_VCALL QuaternionLengthSqr(cvf32 q);
vf32 BV_VCALL QuaternionLength(cvf32 q);
vf32 BV_VCALL QuaternionRotationAxis(cvf32 v, f32 angle);
vf32 BV_VCALL QuaternionQVQC(cvf32 q, cvf32 v);
vf32 BV_VCALL QuaternionQCVQ(cvf32 q, cvf32 v);
vf32 BV_VCALL QuaternionQVQCKeenan(cvf32 q, cvf32 v);
vf32 BV_VCALL QuaternionQCVQKeenan(cvf32 q, cvf32 v);
vf32 BV_VCALL QuaternionFromMatrix(cmf32 m);
vf32 BV_VCALL QuaternionSlerp(cvf32 q1, cvf32 q2, f32 t, f32 epsilon = kEpsilon);
vf32 BV_VCALL QuaternionAngle(cvf32 q);


#include "BvMathLoaders.inl"
#include "BvMathVector.inl"
#include "BvMathMatrix.inl"
#include "BvMathQuaternion.inl"


class BvQuat;
class BvMat3;
class BvMat4;


class BvBoolVec
{
public:
	BV_DEFAULTCOPYMOVE(BvBoolVec);

	BV_INLINE BvBoolVec() {}
	BV_INLINE BvBoolVec(bool s) : m_Vec(s ? VectorMaskAll() : VectorZero()) {}
	BV_INLINE explicit BvBoolVec(cvf32 v) : m_Vec(v) {}

	BV_INLINE bool AnyTrue() const { return VectorAnyTrue(m_Vec); }
	BV_INLINE bool AllTrue() const { return VectorAllTrue(m_Vec); }
	BV_INLINE bool AnyFalse() const { return VectorAnyFalse(m_Vec); }
	BV_INLINE bool AllFalse() const { return VectorAllFalse(m_Vec); }

	BV_INLINE operator bool() const { return AllTrue(); }
	BV_INLINE bool operator==(const BvBoolVec& rhs) const { return BvBoolVec(VectorEqual(m_Vec, rhs.m_Vec)); }
	BV_INLINE bool operator!=(const BvBoolVec& rhs) const { return !(*this == rhs); }
	
	BV_INLINE operator vf32() const { return m_Vec; }

private:
	vf32 m_Vec;
};


class BvSVec
{
public:
	BV_DEFAULTCOPYMOVE(BvSVec);

	BV_INLINE BvSVec() {}
	BV_INLINE BvSVec(f32 s)	: m_Vec(VectorSet(s)) {}
	BV_INLINE explicit BvSVec(cvf32 v) : m_Vec(v) {}

	BV_INLINE BvSVec operator+() const { return BvSVec(m_Vec); }
	BV_INLINE BvSVec operator-() const { return BvSVec(VectorNegate(m_Vec)); }

	BV_INLINE BvSVec operator+(BvSVec v) const { return BvSVec(m_Vec + v.m_Vec); }
	BV_INLINE BvSVec operator-(BvSVec v) const { return BvSVec(m_Vec - v.m_Vec); }
	BV_INLINE BvSVec operator*(BvSVec v) const { return BvSVec(m_Vec * v.m_Vec); }
	BV_INLINE BvSVec operator/(BvSVec v) const { return BvSVec(m_Vec / v.m_Vec); }

	BV_INLINE BvSVec operator+(f32 s) const { return BvSVec(m_Vec + s); }
	BV_INLINE BvSVec operator-(f32 s) const { return BvSVec(m_Vec - s); }
	BV_INLINE BvSVec operator*(f32 s) const { return BvSVec(m_Vec * s); }
	BV_INLINE BvSVec operator/(f32 s) const { return BvSVec(m_Vec / s); }

	BV_INLINE friend BvSVec operator+(f32 s, BvSVec v) { return BvSVec(s + v.m_Vec); }
	BV_INLINE friend BvSVec operator-(f32 s, BvSVec v) { return BvSVec(s - v.m_Vec); }
	BV_INLINE friend BvSVec operator*(f32 s, BvSVec v) { return BvSVec(s * v.m_Vec); }
	BV_INLINE friend BvSVec operator/(f32 s, BvSVec v) { return BvSVec(s / v.m_Vec); }

	BV_INLINE BvSVec& operator+=(BvSVec v) { m_Vec += v.m_Vec; return *this; }
	BV_INLINE BvSVec& operator-=(BvSVec v) { m_Vec -= v.m_Vec; return *this; }
	BV_INLINE BvSVec& operator*=(BvSVec v) { m_Vec *= v.m_Vec; return *this; }
	BV_INLINE BvSVec& operator/=(BvSVec v) { m_Vec /= v.m_Vec; return *this; }

	BV_INLINE BvSVec& operator+=(f32 s) { m_Vec += s; return *this; }
	BV_INLINE BvSVec& operator-=(f32 s) { m_Vec -= s; return *this; }
	BV_INLINE BvSVec& operator*=(f32 s) { m_Vec *= s; return *this; }
	BV_INLINE BvSVec& operator/=(f32 s) { m_Vec /= s; return *this; }
	
	BV_INLINE BvBoolVec IsNearlyEqual(BvSVec v, f32 epsilon = kEpsilon) const { return BvBoolVec(VectorNearlyEqual(m_Vec, v.m_Vec, epsilon)); }
	BV_INLINE BvBoolVec operator==(BvSVec v) const { return BvBoolVec(VectorEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>(BvSVec v) const { return BvBoolVec(VectorGreater(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(BvSVec v) const { return BvBoolVec(VectorGreaterEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<(BvSVec v) const { return BvBoolVec(VectorLess(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(BvSVec v) const { return BvBoolVec(VectorLessEqual(m_Vec, v.m_Vec)); }

	BV_INLINE operator vf32() const { return m_Vec; }
	BV_INLINE operator f32() const { return VectorGetX(m_Vec); }

private:
	vf32 m_Vec;
};


class BvVec2
{
public:
	BV_DEFAULTCOPYMOVE(BvVec2);

	BV_INLINE BvVec2() {}
	BV_INLINE BvVec2(const Float2& v) : m_Vec(Load(v)) {}
	BV_INLINE explicit BvVec2(cvf32 v) : m_Vec(v) {}

	BV_INLINE BvSVec GetX() const { return BvSVec(VectorReplicateX(m_Vec)); }
	BV_INLINE BvSVec GetY() const { return BvSVec(VectorReplicateY(m_Vec)); }

	BV_INLINE void SetX(f32 x) { m_Vec = VectorSetX(m_Vec, x); }
	BV_INLINE void SetY(f32 y) { m_Vec = VectorSetY(m_Vec, y); }

	BV_INLINE BvVec2 operator+() const { return BvVec2(m_Vec); }
	BV_INLINE BvVec2 operator-() const { return BvVec2(VectorChangeSign<1, 1, 0, 0>(m_Vec)); }

	BV_INLINE BvVec2 operator+(BvVec2 v) const { return BvVec2(m_Vec + v.m_Vec); }
	BV_INLINE BvVec2 operator-(BvVec2 v) const { return BvVec2(m_Vec - v.m_Vec); }
	BV_INLINE BvVec2 operator*(BvVec2 v) const { return BvVec2(m_Vec * v.m_Vec); }
	BV_INLINE BvVec2 operator/(BvVec2 v) const { return BvVec2(m_Vec / v.m_Vec); }

	BV_INLINE BvVec2 operator+(BvSVec v) const { return BvVec2(m_Vec + (vf32)v); }
	BV_INLINE BvVec2 operator-(BvSVec v) const { return BvVec2(m_Vec - (vf32)v); }
	BV_INLINE BvVec2 operator*(BvSVec v) const { return BvVec2(m_Vec * (vf32)v); }
	BV_INLINE BvVec2 operator/(BvSVec v) const { return BvVec2(m_Vec / (vf32)v); }

	BV_INLINE BvVec2 operator+(f32 s) const { return BvVec2(m_Vec + s); }
	BV_INLINE BvVec2 operator-(f32 s) const { return BvVec2(m_Vec - s); }
	BV_INLINE BvVec2 operator*(f32 s) const { return BvVec2(m_Vec * s); }
	BV_INLINE BvVec2 operator/(f32 s) const { return BvVec2(m_Vec / s); }

	BV_INLINE BvVec2& operator+=(BvVec2 v) { m_Vec += v.m_Vec; return *this; }
	BV_INLINE BvVec2& operator-=(BvVec2 v) { m_Vec -= v.m_Vec; return *this; }
	BV_INLINE BvVec2& operator*=(BvVec2 v) { m_Vec *= v.m_Vec; return *this; }
	BV_INLINE BvVec2& operator/=(BvVec2 v) { m_Vec /= v.m_Vec; return *this; }

	BV_INLINE BvVec2& operator+=(BvSVec v) { m_Vec += (vf32)v; return *this; }
	BV_INLINE BvVec2& operator-=(BvSVec v) { m_Vec -= (vf32)v; return *this; }
	BV_INLINE BvVec2& operator*=(BvSVec v) { m_Vec *= (vf32)v; return *this; }
	BV_INLINE BvVec2& operator/=(BvSVec v) { m_Vec /= (vf32)v; return *this; }

	BV_INLINE BvVec2& operator+=(f32 s) { m_Vec += s; return *this; }
	BV_INLINE BvVec2& operator-=(f32 s) { m_Vec -= s; return *this; }
	BV_INLINE BvVec2& operator*=(f32 s) { m_Vec *= s; return *this; }
	BV_INLINE BvVec2& operator/=(f32 s) { m_Vec /= s; return *this; }

	BV_INLINE BvSVec Dot(BvVec2 v) const { return BvSVec(Vector2Dot(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec Length() const { return BvSVec(Vector2Length(m_Vec)); }
	BV_INLINE BvSVec LengthSqr() const { return BvSVec(Vector2LengthSqr(m_Vec)); }
	BV_INLINE BvVec2 Normalize() const { return BvVec2(Vector2Normalize(m_Vec)); }
	BV_INLINE BvSVec Cross(BvVec2 v) const { return BvSVec(Vector2Cross(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec2 Lerp(BvVec2 v, f32 t) const { return BvVec2(VectorLerp(m_Vec, v.m_Vec, t)); }
	BV_INLINE BvVec2 Rotate(f32 rad) const { return BvVec2(Vector2Rotate(m_Vec, rad)); }
	BV_INLINE BvVec2 InvRotate(f32 rad) const { return BvVec2(Vector2InvRotate(m_Vec, rad)); }

	BV_INLINE BvBoolVec IsNearlyEqual(BvVec2 v, f32 epsilon = kEpsilon) const { return BvBoolVec(Vector2NearlyEqual(m_Vec, v.m_Vec, epsilon)); }
	BV_INLINE BvBoolVec operator==(BvVec2 v) const { return BvBoolVec(Vector2Equal(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>(BvVec2 v) const { return BvBoolVec(Vector2Greater(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(BvVec2 v) const { return BvBoolVec(Vector2GreaterEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<(BvVec2 v) const { return BvBoolVec(Vector2Less(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(BvVec2 v) const { return BvBoolVec(Vector2LessEqual(m_Vec, v.m_Vec)); }

	BV_INLINE operator vf32() const { return m_Vec; }

private:
	vf32 m_Vec;
};


class BvVec3
{
public:
	BV_DEFAULTCOPYMOVE(BvVec3);

	BV_INLINE BvVec3() {}
	BV_INLINE BvVec3(const Float3& v) : m_Vec(Load(v)) {}
	BV_INLINE explicit BvVec3(cvf32 v) : m_Vec(v) {}

	BV_INLINE BvSVec GetX() const { return BvSVec(VectorReplicateX(m_Vec)); }
	BV_INLINE BvSVec GetY() const { return BvSVec(VectorReplicateY(m_Vec)); }
	BV_INLINE BvSVec GetZ() const { return BvSVec(VectorReplicateZ(m_Vec)); }

	BV_INLINE void SetX(f32 x) { m_Vec = VectorSetX(m_Vec, x); }
	BV_INLINE void SetY(f32 y) { m_Vec = VectorSetY(m_Vec, y); }
	BV_INLINE void SetZ(f32 z) { m_Vec = VectorSetZ(m_Vec, z); }

	BV_INLINE BvVec3 operator+() const { return BvVec3(m_Vec); }
	BV_INLINE BvVec3 operator-() const { return BvVec3(VectorChangeSign<1, 1, 1, 0>(m_Vec)); }

	BV_INLINE BvVec3 operator+(BvVec3 v) const { return BvVec3(m_Vec + v.m_Vec); }
	BV_INLINE BvVec3 operator-(BvVec3 v) const { return BvVec3(m_Vec - v.m_Vec); }
	BV_INLINE BvVec3 operator*(BvVec3 v) const { return BvVec3(m_Vec * v.m_Vec); }
	BV_INLINE BvVec3 operator/(BvVec3 v) const { return BvVec3(m_Vec / v.m_Vec); }

	BV_INLINE BvVec3 operator+(BvSVec v) const { return BvVec3(m_Vec + (vf32)v); }
	BV_INLINE BvVec3 operator-(BvSVec v) const { return BvVec3(m_Vec - (vf32)v); }
	BV_INLINE BvVec3 operator*(BvSVec v) const { return BvVec3(m_Vec * (vf32)v); }
	BV_INLINE BvVec3 operator/(BvSVec v) const { return BvVec3(m_Vec / (vf32)v); }

	BV_INLINE BvVec3 operator+(f32 s) const { return BvVec3(m_Vec + s); }
	BV_INLINE BvVec3 operator-(f32 s) const { return BvVec3(m_Vec - s); }
	BV_INLINE BvVec3 operator*(f32 s) const { return BvVec3(m_Vec * s); }
	BV_INLINE BvVec3 operator/(f32 s) const { return BvVec3(m_Vec / s); }

	BV_INLINE BvVec3& operator+=(BvVec3 v) { m_Vec += v.m_Vec; return *this; }
	BV_INLINE BvVec3& operator-=(BvVec3 v) { m_Vec -= v.m_Vec; return *this; }
	BV_INLINE BvVec3& operator*=(BvVec3 v) { m_Vec *= v.m_Vec; return *this; }
	BV_INLINE BvVec3& operator/=(BvVec3 v) { m_Vec /= v.m_Vec; return *this; }

	BV_INLINE BvVec3& operator+=(BvSVec v) { m_Vec += (vf32)v; return *this; }
	BV_INLINE BvVec3& operator-=(BvSVec v) { m_Vec -= (vf32)v; return *this; }
	BV_INLINE BvVec3& operator*=(BvSVec v) { m_Vec *= (vf32)v; return *this; }
	BV_INLINE BvVec3& operator/=(BvSVec v) { m_Vec /= (vf32)v; return *this; }

	BV_INLINE BvVec3& operator+=(f32 s) { m_Vec += s; return *this; }
	BV_INLINE BvVec3& operator-=(f32 s) { m_Vec -= s; return *this; }
	BV_INLINE BvVec3& operator*=(f32 s) { m_Vec *= s; return *this; }
	BV_INLINE BvVec3& operator/=(f32 s) { m_Vec /= s; return *this; }

	BV_INLINE BvSVec Dot(BvVec3 v) const { return BvSVec(Vector3Dot(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec Length() const { return BvSVec(Vector3Length(m_Vec)); }
	BV_INLINE BvSVec LengthSqr() const { return BvSVec(Vector3LengthSqr(m_Vec)); }
	BV_INLINE BvVec3 Normalize() const { return BvVec3(Vector3Normalize(m_Vec)); }
	BV_INLINE BvVec3 Cross(BvVec3 v) const { return BvVec3(Vector3Cross(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec3 Lerp(BvVec3 v, f32 t) const { return BvVec3(VectorLerp(m_Vec, v.m_Vec, t)); }
	BV_INLINE BvVec3 Rotate(BvQuat q) const; // { return BvVec3(Vector3Rotate(m_Vec, q.m_Vec)); }
	BV_INLINE BvVec3 InvRotate(BvQuat q) const; // { return BvVec3(Vector3InvRotate(m_Vec, q.m_Vec)); }

	BV_INLINE BvBoolVec IsNearlyEqual(BvVec3 v, f32 epsilon = kEpsilon) const { return BvBoolVec(Vector3NearlyEqual(m_Vec, v.m_Vec, epsilon)); }
	BV_INLINE BvBoolVec operator==(BvVec3 v) const { return BvBoolVec(Vector3Equal(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>(BvVec3 v) const { return BvBoolVec(Vector3Greater(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(BvVec3 v) const { return BvBoolVec(Vector3GreaterEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<(BvVec3 v) const { return BvBoolVec(Vector3Less(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(BvVec3 v) const { return BvBoolVec(Vector3LessEqual(m_Vec, v.m_Vec)); }

	BV_INLINE operator vf32() const { return m_Vec; }

private:
	vf32 m_Vec;
};