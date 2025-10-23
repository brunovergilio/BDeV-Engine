#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"


// Default floating point error threshold
constexpr f32 kEpsilon = 0.0001f;

constexpr f32 kPi      = 3.141592653589793238462f;
constexpr f32 k1DivPi	 = 1.0f / kPi;
constexpr f32 k2Pi	 = kPi * 2.0f; //6.283185482025146484375f;
constexpr f32 k1Div2Pi = 1.0f / k2Pi;
constexpr f32 k3PiDiv2 = kPi * 1.5f; //4.712388992309570312500f;

constexpr f32 kPiDiv2  = kPi / 2.0f; //1.570796370506286621094f;
constexpr f32 kPiDiv3  = kPi / 3.0f; //1.047197580337524414063f;
constexpr f32 kPiDiv4  = kPi / 4.0f; //0.785398163397448309616f;
constexpr f32 kPiDiv5  = kPi / 5.0f; //0.628318548202514648438f;
constexpr f32 kPiDiv6  = kPi / 6.0f; //0.523598790168762207031f;
constexpr f32 kPiDiv8  = kPi / 8.0f; //0.392699092626571655273f;
constexpr f32 kPiDiv10 = kPi / 10.0f; //0.314159274101257324219f;
constexpr f32 kPiDiv12 = kPi / 12.0f; //0.261799395084381103516f;
constexpr f32 kPiDiv16 = kPi / 16.0f; //0.196349546313285827637f;


//struct Float2
//{
//	union
//	{
//		struct { f32 x, y; };
//		f32 v[2];
//	};
//
//	constexpr Float2() : x(0.0f), y(0.0f) {}
//	constexpr Float2(f32 x, f32 y)
//		: x(x), y(y) {}
//};
//
//
//struct Float3
//{
//	union
//	{
//		struct { f32 x, y, z; };
//		f32 v[3];
//		Float2 v2;
//	};
//
//	constexpr Float3() : x(0.0f), y(0.0f), z(0.0f) {}
//	constexpr Float3(f32 x, f32 y, f32 z)
//		: x(x), y(y), z(z) {}
//};
//
//
//struct Float4
//{
//	union
//	{
//		struct { f32 x, y, z, w; };
//		f32 v[4];
//		Float2 v2;
//		Float3 v3;
//	};
//
//	constexpr Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
//	constexpr Float4(f32 x, f32 y, f32 z, f32 w = 0.0f)
//		: x(x), y(y), z(z), w(w) {}
//};
//
//
//struct alignas(16) Float4A : Float4
//{
//	using Float4::Float4;
//};
//
//
//struct Float22
//{
//	union
//	{
//		Float2 r[2]{};
//		f32 m[4];
//		struct
//		{
//			f32 m0, m1,
//				m2, m3;
//		};
//	};
//
//	constexpr Float22() {}
//	constexpr Float22(const Float2& r0, const Float2& r1)
//	{
//		r[0] = r0;
//		r[1] = r1;
//	}
//};
//
//
//struct Float33
//{
//	union
//	{
//		Float3 r[3]{};
//		f32 m[9];
//		struct
//		{
//			f32 m0, m1, m2,
//				m3, m4, m5,
//				m6, m7, m8;
//		};
//	};
//
//	constexpr Float33() {}
//	constexpr Float33(const Float3& r0, const Float3& r1, const Float3& r2)
//	{
//		r[0] = r0;
//		r[1] = r1;
//		r[2] = r2;
//	}
//};
//
//
//struct Float43
//{
//	union
//	{
//		Float3 r[4]{};
//		f32 m[12];
//		struct
//		{
//			f32 m0, m1, m2,
//				m3, m4, m5,
//				m6, m7, m8,
//				m9, m10, m11;
//		};
//	};
//
//	constexpr Float43() {}
//	constexpr Float43(const Float3& r0, const Float3& r1, const Float3& r2, const Float3& r3)
//	{
//		r[0] = r0;
//		r[1] = r1;
//		r[2] = r2;
//		r[3] = r3;
//	}
//};
//
//
//struct Float34
//{
//	union
//	{
//		Float4 r[3]{};
//		f32 m[12];
//		struct
//		{
//			f32 m0, m1, m2, m3,
//				m4, m5, m6, m7,
//				m8, m9, m10, m11;
//		};
//	};
//
//	constexpr Float34() {}
//	constexpr Float34(const Float4& r0, const Float4& r1, const Float4& r2)
//	{
//		r[0] = r0;
//		r[1] = r1;
//		r[2] = r2;
//	}
//};
//
//
//struct Float44
//{
//	union
//	{
//		Float4 r[4]{};
//		f32 m[16];
//		struct
//		{
//			f32 m0, m1, m2, m3,
//				m4, m5, m6, m7,
//				m8, m9, m10, m11,
//				m12, m13, m14, m15;
//		};
//	};
//
//	constexpr Float44() {}
//	constexpr Float44(const Float4& r0, const Float4& r1, const Float4& r2, const Float4& r3)
//	{
//		r[0] = r0;
//		r[1] = r1;
//		r[2] = r2;
//		r[3] = r3;
//	}
//};
//
//
//struct alignas(16) Float44A : Float44
//{
//	using Float44::Float44;
//};
//
//
//#if (BV_COMPILER == BV_COMPILER_MSVC)
//#define BV_VCALL __vectorcall
//#else
//#define BV_VCALL
//#endif
//
//
//#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_AVX)
////#include <mmintrin.h>  // MMX
////#include <xmmintrin.h> // SSE
////#include <emmintrin.h> // SSE2
////#include <pmmintrin.h> // SSE3
////#include <tmmintrin.h> // SSSE3
////#include <smmintrin.h> // SSE4.1
////#include <nmmintrin.h> // SSE4.2
////#include <ammintrin.h> // SSE4A
////#include <wmmintrin.h> // AES
//#include <immintrin.h> // AVX
////#include <zmmintrin.h> // AVX512
//
//using vf32 = __m128;
//using vi32 = __m128i;
//
//using cvf32 = __m128;
//
//#else
//using vf32 = Float4A;
//using vi32 = i32[4];
//
//using cvf32 = const vf32&;
//#endif
//
//
//struct mf32
//{
//	mf32() = default;
//	mf32(const mf32&) = default;
//	mf32& operator=(const mf32&) = default;
//	mf32(mf32&&) = default;
//	mf32& operator=(mf32&&) = default;
//	BV_INLINE mf32(cvf32 r0, cvf32 r1, cvf32 r2, cvf32 r3)
//	{
//		r[0] = r0;
//		r[1] = r1;
//		r[2] = r2;
//		r[3] = r3;
//	}
//
//	vf32 r[4];
//};
//
//
//using cmf32 = const mf32&;
//
//
//BV_INLINE vf32 BV_VCALL Load1(const f32* p);
//BV_INLINE vf32 BV_VCALL Load2(const f32* p);
//BV_INLINE vf32 BV_VCALL Load3(const f32* p);
//BV_INLINE vf32 BV_VCALL Load4(const f32* p);
//BV_INLINE vf32 BV_VCALL Load4A(const f32* p);
//BV_INLINE void BV_VCALL Store1(cvf32 v, f32* p);
//BV_INLINE void BV_VCALL Store2(cvf32 v, f32* p);
//BV_INLINE void BV_VCALL Store3(cvf32 v, f32* p);
//BV_INLINE void BV_VCALL Store4(cvf32 v, f32* p);
//BV_INLINE void BV_VCALL Store4A(cvf32 v, f32* p);
//BV_INLINE mf32 BV_VCALL Load22(const f32* p);
//BV_INLINE mf32 BV_VCALL Load33(const f32* p);
//BV_INLINE mf32 BV_VCALL Load43(const f32* p);
//BV_INLINE mf32 BV_VCALL Load34(const f32* p);
//BV_INLINE mf32 BV_VCALL Load44(const f32* p);
//BV_INLINE mf32 BV_VCALL Load44A(const f32* p);
//BV_INLINE void BV_VCALL Store22(const mf32& m, f32* p);
//BV_INLINE void BV_VCALL Store33(const mf32& m, f32* p);
//BV_INLINE void BV_VCALL Store43(const mf32& m, f32* p);
//BV_INLINE void BV_VCALL Store34(const mf32& m, f32* p);
//BV_INLINE void BV_VCALL Store44(const mf32& m, f32* p);
//BV_INLINE void BV_VCALL Store44A(const mf32& m, f32* p);
//
//BV_INLINE vf32 BV_VCALL Load(f32 f) { return Load1(&f); }
//BV_INLINE vf32 BV_VCALL Load(const Float2& f) { return Load2(f.v); }
//BV_INLINE vf32 BV_VCALL Load(const Float3& f) { return Load3(f.v); }
//BV_INLINE vf32 BV_VCALL Load(const Float4& f) { return Load4(f.v); }
//BV_INLINE vf32 BV_VCALL Load(const Float4A& f) { return Load4A(f.v); }
//BV_INLINE void BV_VCALL Store(cvf32 v, f32& f) { Store1(v, &f); }
//BV_INLINE void BV_VCALL Store(cvf32 v, Float2& f) { Store2(v, f.v); }
//BV_INLINE void BV_VCALL Store(cvf32 v, Float3& f) { Store3(v, f.v); }
//BV_INLINE void BV_VCALL Store(cvf32 v, Float4& f) { Store4(v, f.v); }
//BV_INLINE void BV_VCALL Store(cvf32 v, Float4A& f) { Store4A(v, f.v); }
//BV_INLINE mf32 BV_VCALL Load(const Float22& r) { return Load22(r.m); }
//BV_INLINE mf32 BV_VCALL Load(const Float33& r) { return Load33(r.m); }
//BV_INLINE mf32 BV_VCALL Load(const Float43& r) { return Load43(r.m); }
//BV_INLINE mf32 BV_VCALL Load(const Float34& r) { return Load43(r.m); }
//BV_INLINE mf32 BV_VCALL Load(const Float44& r) { return Load44(r.m); }
//BV_INLINE mf32 BV_VCALL Load(const Float44A& r) { return Load44A(r.m); }
//BV_INLINE void BV_VCALL Store(cmf32 m, Float22& r) { Store22(m, r.m); }
//BV_INLINE void BV_VCALL Store(cmf32 m, Float33& r) { Store33(m, r.m); }
//BV_INLINE void BV_VCALL Store(cmf32 m, Float43& r) { Store43(m, r.m); }
//BV_INLINE void BV_VCALL Store(cmf32 m, Float34& r) { Store34(m, r.m); }
//BV_INLINE void BV_VCALL Store(cmf32 m, Float44& r) { Store44(m, r.m); }
//BV_INLINE void BV_VCALL Store(cmf32 m, Float44A& r) { Store44A(m, r.m); }
//
//
//f32 BV_VCALL VectorGetX(cvf32 v);
//f32 BV_VCALL VectorGetY(cvf32 v);
//f32 BV_VCALL VectorGetZ(cvf32 v);
//f32 BV_VCALL VectorGetW(cvf32 v);
//vf32 BV_VCALL VectorSetX(cvf32 v, f32 s);
//vf32 BV_VCALL VectorSetY(cvf32 v, f32 s);
//vf32 BV_VCALL VectorSetZ(cvf32 v, f32 s);
//vf32 BV_VCALL VectorSetW(cvf32 v, f32 s);
//vf32 BV_VCALL VectorSet(f32 s);
//vf32 BV_VCALL VectorSet(f32 x, f32 y, f32 z, f32 w);
//vf32 BV_VCALL VectorReplicateX(cvf32 v);
//vf32 BV_VCALL VectorReplicateY(cvf32 v);
//vf32 BV_VCALL VectorReplicateZ(cvf32 v);
//vf32 BV_VCALL VectorReplicateW(cvf32 v);
//vf32 BV_VCALL VectorMaskX();
//vf32 BV_VCALL VectorMaskY();
//vf32 BV_VCALL VectorMaskZ();
//vf32 BV_VCALL VectorMaskW();
//vf32 BV_VCALL VectorMaskAll();
//vf32 BV_VCALL VectorMaskInvX();
//vf32 BV_VCALL VectorMaskInvY();
//vf32 BV_VCALL VectorMaskInvZ();
//vf32 BV_VCALL VectorMaskInvW();
//template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorMask();
//template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorPermute(cvf32 v);
//vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2, i32 mask);
//vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2, cvf32 mask);
//template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorMax(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorMin(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorClamp(cvf32 v, cvf32 min, cvf32 max);
//vf32 BV_VCALL VectorAbs(cvf32 v);
//vf32 BV_VCALL VectorFloor(cvf32 v);
//vf32 BV_VCALL VectorCeil(cvf32 v);
//vf32 BV_VCALL VectorRound(cvf32 v);
//vf32 BV_VCALL VectorSqrt(cvf32 v);
//vf32 BV_VCALL VectorRSqrt(cvf32 v);
//vf32 BV_VCALL VectorRcp(cvf32 v);
//vf32 BV_VCALL VectorNegate(cvf32 v);
//vf32 BV_VCALL VectorNegate2(cvf32 v);
//vf32 BV_VCALL VectorNegate3(cvf32 v);
//template<u32 X, u32 Y, u32 Z, u32 W> vf32 BV_VCALL VectorChangeSign(cvf32 v);
//vf32 BV_VCALL VectorZero();
//vf32 BV_VCALL VectorOne();
//vf32 BV_VCALL VectorUnitX();
//vf32 BV_VCALL VectorUnitY();
//vf32 BV_VCALL VectorUnitZ();
//vf32 BV_VCALL VectorUnitW();
//vf32 BV_VCALL Vector2Equal(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector2NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
//vf32 BV_VCALL Vector2Greater(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector2GreaterEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector2Less(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector2LessEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector3Equal(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector3NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
//vf32 BV_VCALL Vector3Greater(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector3GreaterEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector3Less(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector3LessEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector4Equal(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector4NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
//vf32 BV_VCALL Vector4Greater(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector4GreaterEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector4Less(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector4LessEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorNearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon);
//vf32 BV_VCALL VectorGreater(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorGreaterEqual(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorLess(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorLessEqual(cvf32 v1, cvf32 v2);
//bool BV_VCALL Vector2AllTrue(cvf32 v);
//bool BV_VCALL Vector2AllFalse(cvf32 v);
//bool BV_VCALL Vector2AnyTrue(cvf32 v);
//bool BV_VCALL Vector2AnyFalse(cvf32 v);
//bool BV_VCALL Vector3AllTrue(cvf32 v);
//bool BV_VCALL Vector3AllFalse(cvf32 v);
//bool BV_VCALL Vector3AnyTrue(cvf32 v);
//bool BV_VCALL Vector3AnyFalse(cvf32 v);
//bool BV_VCALL VectorAllTrue(cvf32 v);
//bool BV_VCALL VectorAllFalse(cvf32 v);
//bool BV_VCALL VectorAnyTrue(cvf32 v);
//bool BV_VCALL VectorAnyFalse(cvf32 v);
//vf32 BV_VCALL VectorOr(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorAnd(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorXor(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorNot(cvf32 v);
//vf32 BV_VCALL VectorAndNot(cvf32 v1, cvf32 v2);
//i32 BV_VCALL VectorGetMask(cvf32 v);
//vf32 BV_VCALL VectorAdd(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorSub(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorMul(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorDiv(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL VectorMAdd(cvf32 v1, cvf32 v2, cvf32 v3);
//vf32 BV_VCALL VectorNMAdd(cvf32 v1, cvf32 v2, cvf32 v3);
//vf32 BV_VCALL VectorMSub(cvf32 v1, cvf32 v2, cvf32 v3);
//vf32 BV_VCALL VectorNMSub(cvf32 v1, cvf32 v2, cvf32 v3);
//vf32 BV_VCALL VectorAdd(cvf32 v, f32 val);
//vf32 BV_VCALL VectorSub(cvf32 v, f32 val);
//vf32 BV_VCALL VectorMul(cvf32 v, f32 val);
//vf32 BV_VCALL VectorDiv(cvf32 v, f32 val);
//vf32 BV_VCALL VectorAdd(f32 val, cvf32 v);
//vf32 BV_VCALL VectorSub(f32 val, cvf32 v);
//vf32 BV_VCALL VectorMul(f32 val, cvf32 v);
//vf32 BV_VCALL VectorDiv(f32 val, cvf32 v);
//vf32 BV_VCALL VectorLerp(cvf32 v1, cvf32 v2, f32 t);
//vf32 BV_VCALL Vector2Dot(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector2Cross(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector2LengthSqr(cvf32 v);
//vf32 BV_VCALL Vector2Length(cvf32 v);
//vf32 BV_VCALL Vector2LengthRcp(cvf32 v);
//vf32 BV_VCALL Vector2Normalize(cvf32 v);
//vf32 BV_VCALL Vector2Rotate(cvf32 v, f32 rad);
//vf32 BV_VCALL Vector2InvRotate(cvf32 v, f32 rad);
//vf32 BV_VCALL Vector2TransformDir(cvf32 v, cmf32 m);
//vf32 BV_VCALL Vector2TransformPoint(cvf32 v, cmf32 m);
//bool BV_VCALL Vector2IsUnit(cvf32 v, f32 epsilon = kEpsilon);
//vf32 BV_VCALL Vector3Dot(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector3Cross(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector3LengthSqr(cvf32 v);
//vf32 BV_VCALL Vector3Length(cvf32 v);
//vf32 BV_VCALL Vector3LengthRcp(cvf32 v);
//vf32 BV_VCALL Vector3Normalize(cvf32 v);
//vf32 BV_VCALL Vector3Project(cvf32 v, cvf32 p);
//vf32 BV_VCALL Vector3ProjectNormal(cvf32 v, cvf32 p);
//vf32 BV_VCALL Vector3Reflection(cvf32 i, cvf32 n);
//vf32 BV_VCALL Vector3Refraction(cvf32 i, cvf32 n, f32 eta);
//vf32 BV_VCALL Vector3Rotate(cvf32 v, cvf32 q);
//vf32 BV_VCALL Vector3InvRotate(cvf32 v, cvf32 q);
//vf32 BV_VCALL Vector3TransformDir(cvf32 v, cmf32 m);
//vf32 BV_VCALL Vector3TransformPoint(cvf32 v, cmf32 m);
//bool BV_VCALL Vector3IsUnit(cvf32 v, f32 epsilon = kEpsilon);
//vf32 BV_VCALL Vector4Dot(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL Vector4LengthSqr(cvf32 v);
//vf32 BV_VCALL Vector4Length(cvf32 v);
//vf32 BV_VCALL Vector4LengthRcp(cvf32 v);
//vf32 BV_VCALL VectorTransform(cvf32 v, cmf32 m);
//
//vf32 BV_VCALL operator+(cvf32 v);
//vf32 BV_VCALL operator-(cvf32 v);
//vf32 BV_VCALL operator+(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL operator+(cvf32 v, f32 s);
//vf32 BV_VCALL operator+(f32 s, cvf32 v);
//vf32& BV_VCALL operator+=(vf32& v1, cvf32 v2);
//vf32& BV_VCALL operator+=(vf32& v, f32 s);
//vf32 BV_VCALL operator-(cvf32 v1, cvf32 v2);
//vf32 BV_VCALL operator-(cvf32 v, f32 s);
//vf32 BV_VCALL operator-(f32 s, cvf32 v);
//vf32& BV_VCALL operator-=(vf32& v1, cvf32 v2);
//vf32& BV_VCALL operator-=(vf32& v, f32 s);
//vf32 BV_VCALL operator*(cvf32 v1, cvf32 v2);
//vf32& BV_VCALL operator*=(vf32& v1, cvf32 v2);
//vf32 BV_VCALL operator*(cvf32 v, f32 s);
//vf32 BV_VCALL operator*(f32 s, cvf32 v);
//vf32& BV_VCALL operator*=(vf32& v, f32 s);
//vf32 BV_VCALL operator*(cvf32 v, cmf32 m);
//vf32& BV_VCALL operator*=(vf32& v, cmf32 m);
//vf32 BV_VCALL operator/(cvf32 v1, cvf32 v2);
//vf32& BV_VCALL operator/=(vf32& v1, cvf32 v2);
//vf32 BV_VCALL operator/(cvf32 v, f32 s);
//vf32 BV_VCALL operator/(f32 s, cvf32 v);
//vf32& BV_VCALL operator/=(vf32& v, f32 s);
//
//mf32 BV_VCALL MatrixSet(cvf32 v0, cvf32 v1, cvf32 v2, cvf32 v3);
//mf32 BV_VCALL MatrixAdd(cmf32 m1, cmf32 m2);
//mf32 BV_VCALL MatrixAdd(cmf32 m, f32 s);
//mf32 BV_VCALL MatrixSub(cmf32 m1, cmf32 m2);
//mf32 BV_VCALL MatrixSub(cmf32 m, f32 s);
//mf32 BV_VCALL MatrixMul(cmf32 m1, cmf32 m2);
//mf32 BV_VCALL MatrixMul(cmf32 m, f32 s);
//mf32 BV_VCALL MatrixDiv(cmf32 m, f32 s);
//mf32 BV_VCALL MatrixIdentity();
//mf32 BV_VCALL MatrixTranspose(cmf32 m);
//vf32 BV_VCALL MatrixDeterminant(cmf32 m);
//mf32 BV_VCALL MatrixInverse(cmf32 m, vf32& det);
//mf32 BV_VCALL MatrixInverse(cmf32 m);
//mf32 BV_VCALL MatrixScaling(cvf32 s);
//mf32 BV_VCALL MatrixScaling(f32 sX, f32 sY, f32 sZ);
//mf32 BV_VCALL MatrixTranslation(cvf32 t);
//mf32 BV_VCALL MatrixTranslation(f32 tX, f32 tY, f32 tZ);
//mf32 BV_VCALL MatrixRotationX(f32 angle);
//mf32 BV_VCALL MatrixRotationY(f32 angle);
//mf32 BV_VCALL MatrixRotationZ(f32 angle);
//mf32 BV_VCALL MatrixRotationAxis(cvf32 axis, f32 angle);
//mf32 BV_VCALL MatrixFromQuaternion(cvf32 q);
//mf32 BV_VCALL MatrixLookAt(cvf32 eyePos, cvf32 dirVec, cvf32 upVec);
//mf32 BV_VCALL MatrixLookAtLH(cvf32 eyePos, cvf32 lookPos, cvf32 upVec);
//mf32 BV_VCALL MatrixLookAtRH(cvf32 eyePos, cvf32 lookPos, cvf32 upVec);
//mf32 BV_VCALL MatrixPerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
//mf32 BV_VCALL MatrixPerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
//mf32 BV_VCALL MatrixPerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
//mf32 BV_VCALL MatrixPerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
//mf32 BV_VCALL MatrixPerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
//mf32 BV_VCALL MatrixPerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
//mf32 BV_VCALL MatrixOrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
//mf32 BV_VCALL MatrixOrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
//mf32 BV_VCALL MatrixOrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
//mf32 BV_VCALL MatrixOrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
//mf32 BV_VCALL MatrixOrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
//mf32 BV_VCALL MatrixOrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ);
//
//mf32  BV_VCALL operator+(cmf32 m);
//mf32  BV_VCALL operator-(cmf32 m);
//mf32  BV_VCALL operator+(cmf32 m1, cmf32 m2);
//mf32& BV_VCALL operator+=(mf32& m1, cmf32 m2);
//mf32  BV_VCALL operator-(cmf32 m1, cmf32 m2);
//mf32& BV_VCALL operator-=(mf32& m1, cmf32 m2);
//mf32  BV_VCALL operator*(cmf32 m1, cmf32 m2);
//mf32& BV_VCALL operator*=(mf32& m1, cmf32 m2);
//mf32  BV_VCALL operator+(cmf32 m, f32 s);
//mf32& BV_VCALL operator+=(mf32& m, f32 s);
//mf32  BV_VCALL operator-(cmf32 m, f32 s);
//mf32& BV_VCALL operator-=(mf32& m, f32 s);
//mf32  BV_VCALL operator*(cmf32 m, f32 s);
//mf32& BV_VCALL operator*=(mf32& m, f32 s);
//mf32  BV_VCALL operator/(cmf32 m, f32 s);
//mf32& BV_VCALL operator/=(mf32& m, f32 s);
//
//vf32 BV_VCALL QuaternionIdentity();
//vf32 BV_VCALL QuaternionMul(cvf32 q1, cvf32 q2);
//vf32 BV_VCALL QuaternionMulKeenan(cvf32 q1, cvf32 q2);
//vf32 BV_VCALL QuaternionConjugate(cvf32 q);
//vf32 BV_VCALL QuaternionInverse(cvf32 q);
//vf32 BV_VCALL QuaternionInverseN(cvf32 q);
//vf32 BV_VCALL QuaternionNormalize(cvf32 q);
//vf32 BV_VCALL QuaternionDot(cvf32 q1, cvf32 q2);
//vf32 BV_VCALL QuaternionLengthSqr(cvf32 q);
//vf32 BV_VCALL QuaternionLength(cvf32 q);
//vf32 BV_VCALL QuaternionRotationAxis(cvf32 v, f32 angle);
//vf32 BV_VCALL QuaternionQVQC(cvf32 q, cvf32 v);
//vf32 BV_VCALL QuaternionQCVQ(cvf32 q, cvf32 v);
//vf32 BV_VCALL QuaternionQVQCKeenan(cvf32 q, cvf32 v);
//vf32 BV_VCALL QuaternionQCVQKeenan(cvf32 q, cvf32 v);
//vf32 BV_VCALL QuaternionFromMatrix(cmf32 m);
//vf32 BV_VCALL QuaternionSlerp(cvf32 q1, cvf32 q2, f32 t, f32 epsilon = kEpsilon);
//vf32 BV_VCALL QuaternionAngle(cvf32 q);
//bool BV_VCALL QuaternionIsUnit(cvf32 q, f32 epsilon = kEpsilon);
//
//
//#include "BvMathLoaders.inl"
//#include "BvMathVector.inl"
//#include "BvMathMatrix.inl"
//#include "BvMathQuaternion.inl"


#if 0
#pragma warning(push, 0)
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#pragma warning(pop)
#else
#pragma warning(push, 0)
#include "DirectXMath/DirectXMath.h"
#include "DirectXMath/DirectXCollision.h"
#include "DirectXMath/DirectXPackedVector.h"
#include "DirectXMath/DirectXColors.h"
#pragma warning(pop)
#endif


using namespace DirectX;


class BvQuat;
class BvMatrix;


class BvBoolVec
{
public:
	BV_DEFAULTCOPYMOVE(BvBoolVec);

	BV_INLINE BvBoolVec() {}
	BV_INLINE BvBoolVec(bool s) : m_Vec(XMVectorReplicateInt(u32(s) * kU32Max)) {}
	BV_INLINE explicit BvBoolVec(FXMVECTOR v) : m_Vec(v) {}

	BV_INLINE bool AnyTrue() const { return XMVector4NotEqualInt(m_Vec, XMVectorFalseInt()); }
	BV_INLINE bool AllTrue() const { return XMVector4EqualInt(m_Vec, XMVectorTrueInt()); }
	BV_INLINE bool AnyFalse() const { return XMVector4NotEqualInt(m_Vec, XMVectorTrueInt()); }
	BV_INLINE bool AllFalse() const { return XMVector4EqualInt(m_Vec, XMVectorFalseInt()); }

	BV_INLINE bool operator==(const BvBoolVec& rhs) const { return BvBoolVec(XMVectorEqualInt(m_Vec, rhs.m_Vec)); }
	BV_INLINE bool operator!=(const BvBoolVec& rhs) const { return !(*this == rhs); }
	
	BV_INLINE operator bool() const { return AllTrue(); }
	BV_INLINE operator XMVECTOR() const { return m_Vec; }

private:
	XMVECTOR m_Vec;
};


class BvSVec
{
public:
	BV_DEFAULTCOPYMOVE(BvSVec);

	BV_INLINE BvSVec() {}
	BV_INLINE BvSVec(f32 s)	: m_Vec(XMVectorReplicate(s)) {}
	BV_INLINE explicit BvSVec(FXMVECTOR v) : m_Vec(v) {}

	BV_INLINE BvSVec operator+() const { return BvSVec(m_Vec); }
	BV_INLINE BvSVec operator-() const { return BvSVec(XMVectorNegate(m_Vec)); }

	BV_INLINE BvSVec operator+(const BvSVec& v) const { return BvSVec(XMVectorAdd(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec operator-(const BvSVec& v) const { return BvSVec(XMVectorSubtract(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec operator*(const BvSVec& v) const { return BvSVec(XMVectorMultiply(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec operator/(const BvSVec& v) const { return BvSVec(XMVectorDivide(m_Vec, v.m_Vec)); }

	BV_INLINE BvSVec operator+(f32 s) const { return BvSVec(XMVectorAdd(m_Vec, XMVectorReplicate(s))); }
	BV_INLINE BvSVec operator-(f32 s) const { return BvSVec(XMVectorSubtract(m_Vec, XMVectorReplicate(s))); }
	BV_INLINE BvSVec operator*(f32 s) const { return BvSVec(XMVectorMultiply(m_Vec, XMVectorReplicate(s))); }
	BV_INLINE BvSVec operator/(f32 s) const { return BvSVec(XMVectorDivide(m_Vec, XMVectorReplicate(s))); }

	BV_INLINE friend BvSVec operator+(f32 s, const BvSVec& v) { return BvSVec(s) + v; }
	BV_INLINE friend BvSVec operator-(f32 s, const BvSVec& v) { return BvSVec(s) - v; }
	BV_INLINE friend BvSVec operator*(f32 s, const BvSVec& v) { return BvSVec(s) * v; }
	BV_INLINE friend BvSVec operator/(f32 s, const BvSVec& v) { return BvSVec(s) / v; }

	BV_INLINE BvSVec& operator+=(const BvSVec& v) { m_Vec = XMVectorAdd(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvSVec& operator-=(const BvSVec& v) { m_Vec = XMVectorSubtract(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvSVec& operator*=(const BvSVec& v) { m_Vec = XMVectorMultiply(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvSVec& operator/=(const BvSVec& v) { m_Vec = XMVectorDivide(m_Vec, v.m_Vec); return *this; }

	BV_INLINE BvSVec& operator+=(f32 s) { m_Vec = XMVectorAdd(m_Vec, XMVectorReplicate(s)); return *this; }
	BV_INLINE BvSVec& operator-=(f32 s) { m_Vec = XMVectorSubtract(m_Vec, XMVectorReplicate(s)); return *this; }
	BV_INLINE BvSVec& operator*=(f32 s) { m_Vec = XMVectorMultiply(m_Vec, XMVectorReplicate(s)); return *this; }
	BV_INLINE BvSVec& operator/=(f32 s) { m_Vec = XMVectorDivide(m_Vec, XMVectorReplicate(s)); return *this; }
	
	BV_INLINE BvBoolVec IsNearlyEqual(const BvSVec& v, f32 epsilon = kEpsilon) const { return BvBoolVec(XMVectorNearEqual(m_Vec, v.m_Vec, XMVectorReplicate(epsilon))); }
	BV_INLINE BvBoolVec operator==(const BvSVec& v) const { return BvBoolVec(XMVectorEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>(const BvSVec& v) const { return BvBoolVec(XMVectorGreater(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(const BvSVec& v) const { return BvBoolVec(XMVectorGreaterOrEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<(const BvSVec& v) const { return BvBoolVec(XMVectorLess(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(const BvSVec& v) const { return BvBoolVec(XMVectorLessOrEqual(m_Vec, v.m_Vec)); }

	BV_INLINE operator XMVECTOR() const { return m_Vec; }
	BV_INLINE operator f32() const { return XMVectorGetX(m_Vec); }

private:
	XMVECTOR m_Vec;
};


class BvVec2
{
public:
	BV_DEFAULTCOPYMOVE(BvVec2);

	BV_INLINE BvVec2() {}
	
	BV_INLINE BvVec2(f32 s) : m_Vec(XMVectorSet(s, s, 0.0f, 0.0f)) {}
	BV_INLINE BvVec2(f32 x, f32 y) : m_Vec(XMVectorSet(x, y, 0.0f, 0.0f)) {}
	
	BV_INLINE BvVec2(const XMFLOAT2& v) : m_Vec(XMLoadFloat2(&v)) {}
	BV_INLINE BvVec2(const XMFLOAT2A& v) : m_Vec(XMLoadFloat2A(&v)) {}
	
	BV_INLINE explicit BvVec2(XMVECTOR v) : m_Vec(v) {}

	BV_INLINE static BvVec2 Zero() { return BvVec2(XMVectorZero()); }
	BV_INLINE static BvVec2 One() { return BvVec2(XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f)); }
	BV_INLINE static BvVec2 UnitX() { return BvVec2(g_XMIdentityR0); }
	BV_INLINE static BvVec2 UnitY() { return BvVec2(g_XMIdentityR1); }

	BV_INLINE BvSVec GetX() const { return BvSVec(XMVectorSplatX(m_Vec)); }
	BV_INLINE BvSVec GetY() const { return BvSVec(XMVectorSplatY(m_Vec)); }

	BV_INLINE void SetX(f32 x) { m_Vec = XMVectorSetX(m_Vec, x); }
	BV_INLINE void SetY(f32 y) { m_Vec = XMVectorSetY(m_Vec, y); }

	BV_INLINE BvVec2 operator+() const { return BvVec2(m_Vec); }
	BV_INLINE BvVec2 operator-() const { return BvVec2(XMVectorAndInt(XMVectorNegate(m_Vec), g_XMMaskXY)); }

	BV_INLINE BvVec2 operator+(const BvVec2& v) const { return BvVec2(XMVectorAdd(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec2 operator-(const BvVec2& v) const { return BvVec2(XMVectorSubtract(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec2 operator*(const BvVec2& v) const { return BvVec2(XMVectorMultiply(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec2 operator/(const BvVec2& v) const { return BvVec2(XMVectorDivide(m_Vec, v.m_Vec)); }

	BV_INLINE BvVec2 operator+(const BvSVec& v) const { return BvVec2(XMVectorAdd(m_Vec, XMVectorAndInt(v, g_XMMaskXY))); }
	BV_INLINE BvVec2 operator-(const BvSVec& v) const { return BvVec2(XMVectorSubtract(m_Vec, XMVectorAndInt(v, g_XMMaskXY))); }
	BV_INLINE BvVec2 operator*(const BvSVec& v) const { return BvVec2(XMVectorMultiply(m_Vec, XMVectorAndInt(v, g_XMMaskXY))); }
	BV_INLINE BvVec2 operator/(const BvSVec& v) const { return BvVec2(XMVectorDivide(m_Vec, XMVectorAndInt(v, g_XMMaskXY))); }

	BV_INLINE BvVec2 operator+(f32 s) const { return BvVec2(XMVectorAdd(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f))); }
	BV_INLINE BvVec2 operator-(f32 s) const { return BvVec2(XMVectorSubtract(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f))); }
	BV_INLINE BvVec2 operator*(f32 s) const { return BvVec2(XMVectorMultiply(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f))); }
	BV_INLINE BvVec2 operator/(f32 s) const { return BvVec2(XMVectorDivide(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f))); }

	BV_INLINE BvVec2 operator*(const BvMatrix& m) const;

	BV_INLINE BvVec2& operator+=(const BvVec2& v) { m_Vec = XMVectorAdd(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec2& operator-=(const BvVec2& v) { m_Vec = XMVectorSubtract(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec2& operator*=(const BvVec2& v) { m_Vec = XMVectorMultiply(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec2& operator/=(const BvVec2& v) { m_Vec = XMVectorDivide(m_Vec, v.m_Vec); return *this; }

	BV_INLINE BvVec2& operator+=(const BvSVec& v) { m_Vec = XMVectorAdd(m_Vec, XMVectorAndInt(v, g_XMMaskXY)); return *this; }
	BV_INLINE BvVec2& operator-=(const BvSVec& v) { m_Vec = XMVectorSubtract(m_Vec, XMVectorAndInt(v, g_XMMaskXY)); return *this; }
	BV_INLINE BvVec2& operator*=(const BvSVec& v) { m_Vec = XMVectorMultiply(m_Vec, XMVectorAndInt(v, g_XMMaskXY)); return *this; }
	BV_INLINE BvVec2& operator/=(const BvSVec& v) { m_Vec = XMVectorDivide(m_Vec, XMVectorAndInt(v, g_XMMaskXY)); return *this; }

	BV_INLINE BvVec2& operator+=(f32 s) { m_Vec = XMVectorAdd(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f)); return *this; }
	BV_INLINE BvVec2& operator-=(f32 s) { m_Vec = XMVectorSubtract(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f)); return *this; }
	BV_INLINE BvVec2& operator*=(f32 s) { m_Vec = XMVectorMultiply(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f)); return *this; }
	BV_INLINE BvVec2& operator/=(f32 s) { m_Vec = XMVectorDivide(m_Vec, XMVectorSet(s, s, 0.0f, 0.0f)); return *this; }

	BV_INLINE BvVec2& operator*=(const BvMatrix& m);

	BV_INLINE BvSVec Dot(const BvVec2& v) const { return BvSVec(XMVector2Dot(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec Length() const { return BvSVec(XMVector2Length(m_Vec)); }
	BV_INLINE BvSVec LengthSqr() const { return BvSVec(XMVector2LengthSq(m_Vec)); }
	BV_INLINE BvVec2 Normalize() const { return BvVec2(XMVector2Normalize(m_Vec)); }
	BV_INLINE BvSVec Cross(const BvVec2& v) const { return BvSVec(XMVector2Cross(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec2 Lerp(const BvVec2& v, f32 t) const { return BvVec2(XMVectorLerp(m_Vec, v.m_Vec, t)); }
	BV_INLINE BvVec2 Rotate(f32 rad) const { return BvVec2(XMVector2TransformNormal(m_Vec, XMMatrixRotationZ(rad))); }
	BV_INLINE BvVec2 InvRotate(f32 rad) const { return BvVec2(XMVector2TransformNormal(m_Vec, XMMatrixRotationZ(-rad))); }
	BV_INLINE BvVec2 TransformPoint(const BvMatrix& m) const;
	BV_INLINE BvVec2 TransformNormal(const BvMatrix& m) const;

	BV_INLINE BvBoolVec IsNearlyEqual(const BvVec2& v, f32 epsilon = kEpsilon) const { return BvBoolVec(XMVector2NearEqual(m_Vec, v.m_Vec, XMVectorReplicate(epsilon))); }
	BV_INLINE BvBoolVec operator==(const BvVec2& v) const { return BvBoolVec(XMVector2Equal(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>(const BvVec2& v) const { return BvBoolVec(XMVector2Greater(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(const BvVec2& v) const { return BvBoolVec(XMVector2GreaterOrEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<(const BvVec2& v) const { return BvBoolVec(XMVector2Less(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(const BvVec2& v) const { return BvBoolVec(XMVector2LessOrEqual(m_Vec, v.m_Vec)); }

	BV_INLINE operator XMVECTOR() const { return m_Vec; }
	BV_INLINE XMFLOAT2 ToFloat() const { XMFLOAT2 result; XMStoreFloat2(&result, m_Vec); return result; }

private:
	XMVECTOR m_Vec;
};


class BvVec3
{
public:
	BV_DEFAULTCOPYMOVE(BvVec3);

	BV_INLINE BvVec3() {}
	
	BV_INLINE BvVec3(f32 s) : m_Vec(XMVectorSet(s, s, s, 0.0f)) {}
	BV_INLINE BvVec3(f32 x, f32 y, f32 z) : m_Vec(XMVectorSet(x, y, z, 0.0f)) {}

	BV_INLINE BvVec3(const XMFLOAT3& v) : m_Vec(XMLoadFloat3(&v)) {}
	BV_INLINE BvVec3(const XMFLOAT3A& v) : m_Vec(XMLoadFloat3A(&v)) {}
	BV_INLINE BvVec3(const XMFLOAT2& xy, f32 z) : m_Vec(XMVectorSet(xy.x, xy.y, z, 0.0f)) {}
	BV_INLINE BvVec3(f32 x, const XMFLOAT2& yz) : m_Vec(XMVectorSet(x, yz.x, yz.y, 0.0f)) {}
	
	BV_INLINE explicit BvVec3(const BvVec2& xy, f32 z) : m_Vec(XMVectorSetZ(xy, z)) {}
	BV_INLINE explicit BvVec3(f32 x, const BvVec2& yz) : m_Vec(XMVectorSetX(yz, x)) {}
	BV_INLINE explicit BvVec3(XMVECTOR v) : m_Vec(XMVectorSetW(v, 0.0f)) {}

	BV_INLINE static BvVec3 Zero() { return BvVec3(XMVectorZero()); }
	BV_INLINE static BvVec3 One() { return BvVec3(g_XMOne3); }
	BV_INLINE static BvVec3 UnitX() { return BvVec3(g_XMIdentityR0); }
	BV_INLINE static BvVec3 UnitY() { return BvVec3(g_XMIdentityR1); }
	BV_INLINE static BvVec3 UnitZ() { return BvVec3(g_XMIdentityR2); }

	BV_INLINE BvSVec GetX() const { return BvSVec(XMVectorSplatX(m_Vec)); }
	BV_INLINE BvSVec GetY() const { return BvSVec(XMVectorSplatY(m_Vec)); }
	BV_INLINE BvSVec GetZ() const { return BvSVec(XMVectorSplatZ(m_Vec)); }

	BV_INLINE void SetX(f32 x) { m_Vec = XMVectorSetX(m_Vec, x); }
	BV_INLINE void SetY(f32 y) { m_Vec = XMVectorSetY(m_Vec, y); }
	BV_INLINE void SetZ(f32 z) { m_Vec = XMVectorSetZ(m_Vec, z); }

	BV_INLINE BvVec3 operator+() const { return BvVec3(m_Vec); }
	BV_INLINE BvVec3 operator-() const { return BvVec3(XMVectorAndInt(XMVectorNegate(m_Vec), g_XMMask3)); }

	BV_INLINE BvVec3 operator+(const BvVec3& v) const { return BvVec3(XMVectorAdd(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec3 operator-(const BvVec3& v) const { return BvVec3(XMVectorSubtract(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec3 operator*(const BvVec3& v) const { return BvVec3(XMVectorMultiply(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec3 operator/(const BvVec3& v) const { return BvVec3(XMVectorDivide(m_Vec, v.m_Vec)); }

	BV_INLINE BvVec3 operator+(const BvSVec& v) const { return BvVec3(XMVectorAdd(m_Vec, XMVectorAndInt(v, g_XMMask3))); }
	BV_INLINE BvVec3 operator-(const BvSVec& v) const { return BvVec3(XMVectorSubtract(m_Vec, XMVectorAndInt(v, g_XMMask3))); }
	BV_INLINE BvVec3 operator*(const BvSVec& v) const { return BvVec3(XMVectorMultiply(m_Vec, XMVectorAndInt(v, g_XMMask3))); }
	BV_INLINE BvVec3 operator/(const BvSVec& v) const { return BvVec3(XMVectorDivide(m_Vec, XMVectorAndInt(v, g_XMMask3))); }

	BV_INLINE BvVec3 operator+(f32 s) const { return BvVec3(XMVectorAdd(m_Vec, XMVectorSet(s, s, s, 0.0f))); }
	BV_INLINE BvVec3 operator-(f32 s) const { return BvVec3(XMVectorSubtract(m_Vec, XMVectorSet(s, s, s, 0.0f))); }
	BV_INLINE BvVec3 operator*(f32 s) const { return BvVec3(XMVectorMultiply(m_Vec, XMVectorSet(s, s, s, 0.0f))); }
	BV_INLINE BvVec3 operator/(f32 s) const { return BvVec3(XMVectorDivide(m_Vec, XMVectorSet(s, s, s, 0.0f))); }

	BV_INLINE BvVec3 operator*(const BvQuat& q) const;
	BV_INLINE BvVec3 operator*(const BvMatrix& m) const;

	BV_INLINE BvVec3& operator+=(const BvVec3& v) { m_Vec = XMVectorAdd(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec3& operator-=(const BvVec3& v) { m_Vec = XMVectorSubtract(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec3& operator*=(const BvVec3& v) { m_Vec = XMVectorMultiply(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec3& operator/=(const BvVec3& v) { m_Vec = XMVectorDivide(m_Vec, v.m_Vec); return *this; }

	BV_INLINE BvVec3& operator+=(const BvSVec& v) { m_Vec = XMVectorAdd(m_Vec, XMVectorAndInt(v, g_XMMask3)); return *this; }
	BV_INLINE BvVec3& operator-=(const BvSVec& v) { m_Vec = XMVectorSubtract(m_Vec, XMVectorAndInt(v, g_XMMask3)); return *this; }
	BV_INLINE BvVec3& operator*=(const BvSVec& v) { m_Vec = XMVectorMultiply(m_Vec, XMVectorAndInt(v, g_XMMask3)); return *this; }
	BV_INLINE BvVec3& operator/=(const BvSVec& v) { m_Vec = XMVectorDivide(m_Vec, XMVectorAndInt(v, g_XMMask3)); return *this; }

	BV_INLINE BvVec3& operator+=(f32 s) { m_Vec = XMVectorAdd(m_Vec, XMVectorSet(s, s, s, 0.0f)); return *this; }
	BV_INLINE BvVec3& operator-=(f32 s) { m_Vec = XMVectorSubtract(m_Vec, XMVectorSet(s, s, s, 0.0f)); return *this; }
	BV_INLINE BvVec3& operator*=(f32 s) { m_Vec = XMVectorMultiply(m_Vec, XMVectorSet(s, s, s, 0.0f)); return *this; }
	BV_INLINE BvVec3& operator/=(f32 s) { m_Vec = XMVectorDivide(m_Vec, XMVectorSet(s, s, s, 0.0f)); return *this; }

	BV_INLINE BvVec3& operator*=(const BvQuat& q);
	BV_INLINE BvVec3& operator*=(const BvMatrix& m);

	BV_INLINE BvSVec Dot(const BvVec3& v) const { return BvSVec(XMVector3Dot(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec Length() const { return BvSVec(XMVector3Length(m_Vec)); }
	BV_INLINE BvSVec LengthSqr() const { return BvSVec(XMVector3LengthSq(m_Vec)); }
	BV_INLINE BvVec3 Normalize() const { return BvVec3(XMVector3Normalize(m_Vec)); }
	BV_INLINE BvVec3 Cross(const BvVec3& v) const { return BvVec3(XMVector3Cross(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec3 Lerp(const BvVec3& v, f32 t) const { return BvVec3(XMVectorLerp(m_Vec, v.m_Vec, t)); }
	BV_INLINE BvVec3 Rotate(const BvQuat& q) const;
	BV_INLINE BvVec3 InvRotate(const BvQuat& q) const;
	BV_INLINE BvVec3 TransformPoint(const BvMatrix& m) const;
	BV_INLINE BvVec3 TransformNormal(const BvMatrix& m) const;

	BV_INLINE BvBoolVec IsNearlyEqual(const BvVec3& v, f32 epsilon = kEpsilon) const { return BvBoolVec(XMVector3NearEqual(m_Vec, v.m_Vec, XMVectorReplicate(epsilon))); }
	BV_INLINE BvBoolVec operator==(const BvVec3& v) const { return BvBoolVec(XMVector3Equal(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>(const BvVec3& v) const { return BvBoolVec(XMVector3Greater(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(const BvVec3& v) const { return BvBoolVec(XMVector3GreaterOrEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<(const BvVec3& v) const { return BvBoolVec(XMVector3Less(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(const BvVec3& v) const { return BvBoolVec(XMVector3LessOrEqual(m_Vec, v.m_Vec)); }

	BV_INLINE operator XMVECTOR() const { return m_Vec; }
	BV_INLINE XMFLOAT3 ToFloat() const { XMFLOAT3 result; XMStoreFloat3(& result, m_Vec); return result; }

private:
	XMVECTOR m_Vec;
};


class BvVec4
{
public:
	BV_DEFAULTCOPYMOVE(BvVec4);

	BV_INLINE BvVec4() {}
	
	BV_INLINE BvVec4(f32 s) : m_Vec(XMVectorReplicate(s)) {}
	BV_INLINE BvVec4(f32 x, f32 y, f32 z, f32 w = 1.0f) : m_Vec(XMVectorSet(x, y, z, w)) {}

	BV_INLINE BvVec4(const XMFLOAT4& v) : m_Vec(XMLoadFloat4(&v)) {}
	BV_INLINE BvVec4(const XMFLOAT4A& v) : m_Vec(XMLoadFloat4A(&v)) {}
	BV_INLINE BvVec4(const XMFLOAT3& xyz, f32 w) : m_Vec(XMVectorSet(xyz.x, xyz.y, xyz.z, w)) {}
	BV_INLINE BvVec4(f32 x, const XMFLOAT3& yzw) : m_Vec(XMVectorSet(x, yzw.x, yzw.y, yzw.z)) {}
	BV_INLINE BvVec4(const XMFLOAT2& xy, f32 z, f32 w) : m_Vec(XMVectorSet(xy.x, xy.y, z, w)) {}
	BV_INLINE BvVec4(f32 x, const XMFLOAT2& yz, f32 w) : m_Vec(XMVectorSet(x, yz.x, yz.y, w)) {}
	BV_INLINE BvVec4(f32 x, f32 y, const XMFLOAT2& zw) : m_Vec(XMVectorSet(x, y, zw.x, zw.y)) {}
	BV_INLINE BvVec4(const XMFLOAT2& xy, const XMFLOAT2& zw) : m_Vec(XMVectorSet(xy.x, xy.y, zw.x, zw.y)) {}
	
	BV_INLINE explicit BvVec4(const BvVec3& xyz, f32 w) : m_Vec(XMVectorSetW(xyz, w)) {}
	BV_INLINE explicit BvVec4(f32 x, const BvVec3& yzw) : m_Vec(XMVectorSetX(yzw, x)) {}
	BV_INLINE explicit BvVec4(const BvVec2& xy, f32 z, f32 w) : m_Vec(XMVectorPermute<0, 1, 6, 7>(xy, XMVectorSet(0.0f, 0.0f, z, w))) {}
	BV_INLINE explicit BvVec4(f32 x, const BvVec2& yz, f32 w) : m_Vec(XMVectorPermute<4, 0, 1, 7>(yz, XMVectorSet(x, 0.0f, 0.0f, w))) {}
	BV_INLINE explicit BvVec4(f32 x, f32 y, const BvVec2& zw) : m_Vec(XMVectorPermute<4, 5, 0, 1>(zw, XMVectorSet(x, y, 0.0f, 0.0f))) {}
	BV_INLINE explicit BvVec4(const BvVec2& xy, const BvVec2& zw) : m_Vec(XMVectorPermute<0, 1, 4, 5>(xy, zw)) {}
	
	BV_INLINE explicit BvVec4(FXMVECTOR v) : m_Vec(v) {}

	BV_INLINE static BvVec4 Zero() { return BvVec4(XMVectorZero()); }
	BV_INLINE static BvVec4 One() { return BvVec4(g_XMOne); }
	BV_INLINE static BvVec4 UnitX() { return BvVec4(g_XMIdentityR0); }
	BV_INLINE static BvVec4 UnitY() { return BvVec4(g_XMIdentityR1); }
	BV_INLINE static BvVec4 UnitZ() { return BvVec4(g_XMIdentityR2); }
	BV_INLINE static BvVec4 UnitW() { return BvVec4(g_XMIdentityR3); }

	BV_INLINE BvSVec GetX() const { return BvSVec(XMVectorSplatX(m_Vec)); }
	BV_INLINE BvSVec GetY() const { return BvSVec(XMVectorSplatY(m_Vec)); }
	BV_INLINE BvSVec GetZ() const { return BvSVec(XMVectorSplatZ(m_Vec)); }
	BV_INLINE BvSVec GetW() const { return BvSVec(XMVectorSplatW(m_Vec)); }

	BV_INLINE void SetX(f32 x) { m_Vec = XMVectorSetX(m_Vec, x); }
	BV_INLINE void SetY(f32 y) { m_Vec = XMVectorSetY(m_Vec, y); }
	BV_INLINE void SetZ(f32 z) { m_Vec = XMVectorSetZ(m_Vec, z); }
	BV_INLINE void SetW(f32 w) { m_Vec = XMVectorSetW(m_Vec, w); }

	BV_INLINE BvVec4 operator+() const { return BvVec4(m_Vec); }
	BV_INLINE BvVec4 operator-() const { return BvVec4(XMVectorNegate(m_Vec)); }

	BV_INLINE BvVec4 operator+(const BvVec4& v) const { return BvVec4(XMVectorAdd(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec4 operator-(const BvVec4& v) const { return BvVec4(XMVectorSubtract(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec4 operator*(const BvVec4& v) const { return BvVec4(XMVectorMultiply(m_Vec, v.m_Vec)); }
	BV_INLINE BvVec4 operator/(const BvVec4& v) const { return BvVec4(XMVectorDivide(m_Vec, v.m_Vec)); }

	BV_INLINE BvVec4 operator+(const BvSVec& v) const { return BvVec4(XMVectorAdd(m_Vec, v)); }
	BV_INLINE BvVec4 operator-(const BvSVec& v) const { return BvVec4(XMVectorSubtract(m_Vec, v)); }
	BV_INLINE BvVec4 operator*(const BvSVec& v) const { return BvVec4(XMVectorMultiply(m_Vec, v)); }
	BV_INLINE BvVec4 operator/(const BvSVec& v) const { return BvVec4(XMVectorDivide(m_Vec, v)); }

	BV_INLINE BvVec4 operator+(f32 s) const { return BvVec4(XMVectorAdd(m_Vec, XMVectorReplicate(s))); }
	BV_INLINE BvVec4 operator-(f32 s) const { return BvVec4(XMVectorSubtract(m_Vec, XMVectorReplicate(s))); }
	BV_INLINE BvVec4 operator*(f32 s) const { return BvVec4(XMVectorMultiply(m_Vec, XMVectorReplicate(s))); }
	BV_INLINE BvVec4 operator/(f32 s) const { return BvVec4(XMVectorDivide(m_Vec, XMVectorReplicate(s))); }

	BV_INLINE BvVec4& operator+=(const BvVec4& v) { m_Vec = XMVectorAdd(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec4& operator-=(const BvVec4& v) { m_Vec = XMVectorSubtract(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec4& operator*=(const BvVec4& v) { m_Vec = XMVectorMultiply(m_Vec, v.m_Vec); return *this; }
	BV_INLINE BvVec4& operator/=(const BvVec4& v) { m_Vec = XMVectorDivide(m_Vec, v.m_Vec); return *this; }

	BV_INLINE BvVec4& operator+=(const BvSVec& v) { m_Vec = XMVectorAdd(m_Vec, v); return *this; }
	BV_INLINE BvVec4& operator-=(const BvSVec& v) { m_Vec = XMVectorSubtract(m_Vec, v); return *this; }
	BV_INLINE BvVec4& operator*=(const BvSVec& v) { m_Vec = XMVectorMultiply(m_Vec, v); return *this; }
	BV_INLINE BvVec4& operator/=(const BvSVec& v) { m_Vec = XMVectorDivide(m_Vec, v); return *this; }

	BV_INLINE BvVec4& operator+=(f32 s) { m_Vec = XMVectorAdd(m_Vec, XMVectorReplicate(s)); return *this; }
	BV_INLINE BvVec4& operator-=(f32 s) { m_Vec = XMVectorSubtract(m_Vec, XMVectorReplicate(s)); return *this; }
	BV_INLINE BvVec4& operator*=(f32 s) { m_Vec = XMVectorMultiply(m_Vec, XMVectorReplicate(s)); return *this; }
	BV_INLINE BvVec4& operator/=(f32 s) { m_Vec = XMVectorDivide(m_Vec, XMVectorReplicate(s)); return *this; }

	BV_INLINE BvSVec Dot(const BvVec4& v) const { return BvSVec(XMVector4Dot(m_Vec, v.m_Vec)); }
	BV_INLINE BvSVec Length() const { return BvSVec(XMVector4Length(m_Vec)); }
	BV_INLINE BvSVec LengthSqr() const { return BvSVec(XMVector4LengthSq(m_Vec)); }
	BV_INLINE BvVec4 Lerp(const BvVec4& v, f32 t) const { return BvVec4(XMVectorLerp(m_Vec, v.m_Vec, t)); }
	BV_INLINE BvVec4 Transform(const BvMatrix& m) const;

	BV_INLINE BvBoolVec IsNearlyEqual(const BvVec4& v, f32 epsilon = kEpsilon) const { return BvBoolVec(XMVectorNearEqual(m_Vec, v.m_Vec, XMVectorReplicate(epsilon))); }
	BV_INLINE BvBoolVec operator==(const BvVec4& v) const { return BvBoolVec(XMVectorEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>(const BvVec4& v) const { return BvBoolVec(XMVectorGreater(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(const BvVec4& v) const { return BvBoolVec(XMVectorGreaterOrEqual(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<(const BvVec4& v) const { return BvBoolVec(XMVectorLess(m_Vec, v.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(const BvVec4& v) const { return BvBoolVec(XMVectorLessOrEqual(m_Vec, v.m_Vec)); }

	BV_INLINE operator XMVECTOR() const { return m_Vec; }
	BV_INLINE XMFLOAT4 ToFloat() const { XMFLOAT4 result; XMStoreFloat4(&result, m_Vec); return result; }

private:
	XMVECTOR m_Vec;
};


class BvMatrix
{
public:
	BV_DEFAULTCOPYMOVE(BvMatrix);

	BV_INLINE BvMatrix() {}
	BV_INLINE BvMatrix(const XMFLOAT4X4& m) : m_Mat(XMLoadFloat4x4(&m)) {}
	BV_INLINE BvMatrix(const XMFLOAT4X4A& m) : m_Mat(XMLoadFloat4x4A(&m)) {}
	BV_INLINE BvMatrix(const XMFLOAT4X3& m) : m_Mat(XMLoadFloat4x3(&m)) {}
	BV_INLINE BvMatrix(const XMFLOAT4X3A& m) : m_Mat(XMLoadFloat4x3A(&m)) {}
	BV_INLINE BvMatrix(const BvVec3& r0, const BvVec3& r1, const BvVec3& r2) : m_Mat(r0, r1, r2, g_XMIdentityR3) {}
	BV_INLINE BvMatrix(const BvVec4& r0, const BvVec4& r1, const BvVec4& r2, const BvVec4& r3) : m_Mat(r0, r1, r2, r3) {}
	BV_INLINE explicit BvMatrix(const BvQuat& q);
	BV_INLINE explicit BvMatrix(FXMMATRIX m) : m_Mat(m) {}

	BV_INLINE static BvMatrix Identity() { return BvMatrix(XMMatrixIdentity()); }
	BV_INLINE static BvMatrix Scale(f32 x, f32 y, f32 z) { return BvMatrix(XMMatrixScaling(x, y, z)); }
	BV_INLINE static BvMatrix Scale(const BvVec3& v) { return BvMatrix(XMMatrixScalingFromVector(v)); }
	BV_INLINE static BvMatrix RotationX(f32 rad) { return BvMatrix(XMMatrixRotationX(rad)); }
	BV_INLINE static BvMatrix RotationY(f32 rad) { return BvMatrix(XMMatrixRotationY(rad)); }
	BV_INLINE static BvMatrix RotationZ(f32 rad) { return BvMatrix(XMMatrixRotationZ(rad)); }
	BV_INLINE static BvMatrix RotationAxis(const BvVec3& axis, f32 rad) { return BvMatrix(XMMatrixRotationAxis(axis, rad)); }
	BV_INLINE static BvMatrix Translation(f32 x, f32 y, f32 z) { return BvMatrix(XMMatrixTranslation(x, y, z)); }
	BV_INLINE static BvMatrix Translation(const BvVec3& v) { return BvMatrix(XMMatrixTranslationFromVector(v)); }
	BV_INLINE static BvMatrix LookAtLH(const BvVec3& eyePos, const BvVec3& lookPos, const BvVec3& upVec) { return BvMatrix(XMMatrixLookAtLH(eyePos, lookPos, upVec)); }
	BV_INLINE static BvMatrix LookAtRH(const BvVec3& eyePos, const BvVec3& lookPos, const BvVec3& upVec) { return BvMatrix(XMMatrixLookAtRH(eyePos, lookPos, upVec)); }
	BV_INLINE static BvMatrix PerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { return BvMatrix(XMMatrixPerspectiveLH(nearZ, farZ, aspectRatio, fovY)); }
	BV_INLINE static BvMatrix PerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { return BvMatrix(XMMatrixPerspectiveRH(nearZ, farZ, aspectRatio, fovY)); }
	//BV_INLINE static BvMatrix PerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { return BvMatrix(MatrixPerspectiveLH_GL(nearZ, farZ, aspectRatio, fovY)); }
	//BV_INLINE static BvMatrix PerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { return BvMatrix(MatrixPerspectiveRH_GL(nearZ, farZ, aspectRatio, fovY)); }
	//BV_INLINE static BvMatrix PerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { return BvMatrix(MatrixPerspectiveLH_VK(nearZ, farZ, aspectRatio, fovY)); }
	//BV_INLINE static BvMatrix PerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { return BvMatrix(MatrixPerspectiveRH_VK(nearZ, farZ, aspectRatio, fovY)); }
	BV_INLINE static BvMatrix OrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { return BvMatrix(XMMatrixOrthographicOffCenterLH(right, left, top, bottom, nearZ, farZ)); }
	BV_INLINE static BvMatrix OrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { return BvMatrix(XMMatrixOrthographicOffCenterRH(right, left, top, bottom, nearZ, farZ)); }
	//BV_INLINE static BvMatrix OrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { return BvMatrix(MatrixOrthographicOffCenterLH_GL(right, left, top, bottom, nearZ, farZ)); }
	//BV_INLINE static BvMatrix OrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { return BvMatrix(MatrixOrthographicOffCenterRH_GL(right, left, top, bottom, nearZ, farZ)); }
	//BV_INLINE static BvMatrix OrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { return BvMatrix(MatrixOrthographicOffCenterLH_VK(right, left, top, bottom, nearZ, farZ)); }
	//BV_INLINE static BvMatrix OrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { return BvMatrix(MatrixOrthographicOffCenterRH_VK(right, left, top, bottom, nearZ, farZ)); }

	BV_INLINE void SetIdentity() { m_Mat = XMMatrixIdentity(); }
	BV_INLINE void SetScale(f32 x, f32 y, f32 z) { m_Mat = XMMatrixScaling(x, y, z); }
	BV_INLINE void SetScale(const BvVec3& v) { m_Mat = XMMatrixScalingFromVector(v); }
	BV_INLINE void SetRotationX(f32 rad) { m_Mat = XMMatrixRotationX(rad); }
	BV_INLINE void SetRotationY(f32 rad) { m_Mat = XMMatrixRotationY(rad); }
	BV_INLINE void SetRotationZ(f32 rad) { m_Mat = XMMatrixRotationZ(rad); }
	BV_INLINE void SetRotationAxis(const BvVec3& axis, f32 rad) { m_Mat = XMMatrixRotationAxis(axis, rad); }
	BV_INLINE void SetTranslation(f32 x, f32 y, f32 z) { m_Mat = XMMatrixTranslation(x, y, z); }
	BV_INLINE void SetTranslation(const BvVec3& v) { m_Mat = XMMatrixTranslationFromVector(v); }
	BV_INLINE void SetLookAtLH(const BvVec3& eyePos, const BvVec3& lookPos, const BvVec3& upVec) { m_Mat = XMMatrixLookAtLH(eyePos, lookPos, upVec); }
	BV_INLINE void SetLookAtRH(const BvVec3& eyePos, const BvVec3& lookPos, const BvVec3& upVec) { m_Mat = XMMatrixLookAtRH(eyePos, lookPos, upVec); }
	BV_INLINE void SetPerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { m_Mat = XMMatrixPerspectiveLH(nearZ, farZ, aspectRatio, fovY); }
	BV_INLINE void SetPerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { m_Mat = XMMatrixPerspectiveRH(nearZ, farZ, aspectRatio, fovY); }
	//BV_INLINE void SetPerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { m_Mat = MatrixPerspectiveLH_GL(nearZ, farZ, aspectRatio, fovY); }
	//BV_INLINE void SetPerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { m_Mat = MatrixPerspectiveRH_GL(nearZ, farZ, aspectRatio, fovY); }
	//BV_INLINE void SetPerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { m_Mat = MatrixPerspectiveLH_VK(nearZ, farZ, aspectRatio, fovY); }
	//BV_INLINE void SetPerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) { m_Mat = MatrixPerspectiveRH_VK(nearZ, farZ, aspectRatio, fovY); }
	BV_INLINE void SetOrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { m_Mat = XMMatrixOrthographicOffCenterLH(right, left, top, bottom, nearZ, farZ); }
	BV_INLINE void SetOrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { m_Mat = XMMatrixOrthographicOffCenterRH(right, left, top, bottom, nearZ, farZ); }
	//BV_INLINE void SetOrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { m_Mat = MatrixOrthographicOffCenterLH_GL(right, left, top, bottom, nearZ, farZ); }
	//BV_INLINE void SetOrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { m_Mat = MatrixOrthographicOffCenterRH_GL(right, left, top, bottom, nearZ, farZ); }
	//BV_INLINE void SetOrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { m_Mat = MatrixOrthographicOffCenterLH_VK(right, left, top, bottom, nearZ, farZ); }
	//BV_INLINE void SetOrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) { m_Mat = MatrixOrthographicOffCenterRH_VK(right, left, top, bottom, nearZ, farZ); }

	BV_INLINE BvVec4 GetX() const { return BvVec4(m_Mat.r[0]); }
	BV_INLINE BvVec4 GetY() const { return BvVec4(m_Mat.r[1]); }
	BV_INLINE BvVec4 GetZ() const { return BvVec4(m_Mat.r[2]); }
	BV_INLINE BvVec4 GetW() const { return BvVec4(m_Mat.r[3]); }

	BV_INLINE void SetX(const BvVec4& v) { m_Mat.r[0] = v; }
	BV_INLINE void SetY(const BvVec4& v) { m_Mat.r[1] = v; }
	BV_INLINE void SetZ(const BvVec4& v) { m_Mat.r[2] = v; }
	BV_INLINE void SetW(const BvVec4& v) { m_Mat.r[3] = v; }

	BV_INLINE BvSVec Determinant() const { return BvSVec(XMMatrixDeterminant(m_Mat)); }
	BV_INLINE BvMatrix Inverse() const { return BvMatrix(XMMatrixInverse(nullptr, m_Mat)); }
	BV_INLINE BvMatrix Transpose() const { return BvMatrix(XMMatrixTranspose(m_Mat)); }

	BV_INLINE void SetInverse() { m_Mat = XMMatrixInverse(nullptr, m_Mat); }
	BV_INLINE void SetTranspose() { m_Mat = XMMatrixTranspose(m_Mat); }

	BV_INLINE BvMatrix operator*(const BvMatrix& m) const { return BvMatrix(XMMatrixMultiply(m_Mat, m)); }
	BV_INLINE BvMatrix& operator*=(const BvMatrix& m) { m_Mat = XMMatrixMultiply(m_Mat, m); return *this; }

	BV_INLINE BvVec4 operator[](size_t index) const { return BvVec4(m_Mat.r[index]); }

	BV_INLINE operator XMMATRIX() const { return m_Mat; }

private:
	XMMATRIX m_Mat;
};


class BvQuat
{
public:
	BV_DEFAULTCOPYMOVE(BvQuat);

	BV_INLINE BvQuat() {}
	BV_INLINE BvQuat(f32 x, f32 y, f32 z, f32 w) : m_Vec(XMVectorSet(x, y, z, w)) {}
	BV_INLINE BvQuat(const XMFLOAT4& q) : m_Vec(XMLoadFloat4(&q)) {}
	BV_INLINE BvQuat(const XMFLOAT4A& q) : m_Vec(XMLoadFloat4A(&q)) {}
	BV_INLINE explicit BvQuat(const BvVec3& v) : m_Vec(XMVectorSetW(v, 0.0f)) {}
	BV_INLINE explicit BvQuat(const BvVec3& axis, f32 rad) : m_Vec(XMQuaternionNormalize(XMQuaternionRotationAxis(axis, rad))) {}
	BV_INLINE explicit BvQuat(const BvMatrix& m) : m_Vec(XMQuaternionRotationMatrix(m)) {}
	BV_INLINE explicit BvQuat(FXMVECTOR v) : m_Vec(v) {}

	BV_INLINE static BvQuat Identity() { return BvQuat(XMQuaternionIdentity()); }
	BV_INLINE static BvQuat RotationAxis(const BvVec3& v, f32 rad) { return BvQuat(XMQuaternionNormalize(XMQuaternionRotationAxis(v, rad))); }
	BV_INLINE void SetIdentity() { m_Vec = XMQuaternionIdentity(); }
	BV_INLINE void SetRotationAxis(const BvVec3& v, f32 rad) { m_Vec = XMQuaternionNormalize(XMQuaternionRotationAxis(v, rad)); }

	BV_INLINE BvSVec GetX() const { return BvSVec(XMVectorSplatX(m_Vec)); }
	BV_INLINE BvSVec GetY() const { return BvSVec(XMVectorSplatY(m_Vec)); }
	BV_INLINE BvSVec GetZ() const { return BvSVec(XMVectorSplatZ(m_Vec)); }
	BV_INLINE BvSVec GetW() const { return BvSVec(XMVectorSplatW(m_Vec)); }

	BV_INLINE void SetX(f32 x) { m_Vec = XMVectorSetX(m_Vec, x); }
	BV_INLINE void SetY(f32 y) { m_Vec = XMVectorSetY(m_Vec, y); }
	BV_INLINE void SetZ(f32 z) { m_Vec = XMVectorSetZ(m_Vec, z); }
	BV_INLINE void SetW(f32 w) { m_Vec = XMVectorSetW(m_Vec, w); }

	BV_INLINE BvSVec Dot(const BvQuat& q) const { return BvSVec(XMQuaternionDot(m_Vec, q.m_Vec)); }
	BV_INLINE BvSVec Length() const { return BvSVec(XMQuaternionLength(m_Vec)); }
	BV_INLINE BvSVec LengthSqr() const { return BvSVec(XMQuaternionLengthSq(m_Vec)); }
	BV_INLINE BvQuat Normalize() const { return BvQuat(XMQuaternionNormalize(m_Vec)); }
	BV_INLINE BvQuat Conjugate() const { return BvQuat(XMQuaternionConjugate(m_Vec)); }
	BV_INLINE BvQuat Inverse() const { return BvQuat(XMQuaternionInverse(m_Vec)); }
	BV_INLINE BvQuat Lerp(const BvQuat& q, f32 t) const { return BvQuat(XMVectorLerp(m_Vec, q.m_Vec, t)); }
	BV_INLINE BvQuat Slerp(const BvQuat& q, f32 t) const { return BvQuat(XMQuaternionSlerp(m_Vec, q.m_Vec, t)); }
	BV_INLINE BvSVec GetAngle() const { XMVECTOR axis; f32 angle; XMQuaternionToAxisAngle(&axis, &angle, m_Vec); return BvSVec(angle); }

	BV_INLINE BvVec4 operator+() const { return BvVec4(m_Vec); }
	BV_INLINE BvVec4 operator-() const { return BvVec4(XMQuaternionConjugate(m_Vec)); }

	BV_INLINE BvQuat operator*(const BvQuat& q) const { return BvQuat(XMQuaternionMultiply(m_Vec, q.m_Vec)); }
	BV_INLINE BvQuat& operator*=(const BvQuat& q) { m_Vec = XMQuaternionMultiply(m_Vec, q.m_Vec); return *this; }

	BV_INLINE BvBoolVec IsNearlyEqual(const BvQuat& q, f32 epsilon = kEpsilon) const { return BvBoolVec(XMVector4NearEqual(m_Vec, q.m_Vec, XMVectorReplicate(epsilon))); }
	BV_INLINE BvBoolVec operator==(const BvQuat& q) const { return BvBoolVec(XMVectorEqual(m_Vec, q.m_Vec)); }
	BV_INLINE BvBoolVec operator>(const BvQuat& q) const { return BvBoolVec(XMVectorGreater(m_Vec, q.m_Vec)); }
	BV_INLINE BvBoolVec operator>=(const BvQuat& q) const { return BvBoolVec(XMVectorGreaterOrEqual(m_Vec, q.m_Vec)); }
	BV_INLINE BvBoolVec operator<(const BvQuat& q) const { return BvBoolVec(XMVectorLess(m_Vec, q.m_Vec)); }
	BV_INLINE BvBoolVec operator<=(const BvQuat& q) const { return BvBoolVec(XMVectorLessOrEqual(m_Vec, q.m_Vec)); }

	BV_INLINE operator XMVECTOR() const { return m_Vec; }

private:
	XMVECTOR m_Vec;
};

BV_INLINE BvVec2 BvVec2::operator*(const BvMatrix& m) const
{
	return BvVec2(XMVector2TransformCoord(m_Vec, m));
}

BV_INLINE BvVec2& BvVec2::operator*=(const BvMatrix& m)
{
	m_Vec = XMVector2TransformCoord(m_Vec, m);
	return *this;
}

BV_INLINE BvVec2 BvVec2::TransformPoint(const BvMatrix& m) const
{
	return BvVec2(XMVector2TransformCoord(m_Vec, m));
}

BV_INLINE BvVec2 BvVec2::TransformNormal(const BvMatrix& m) const
{
	return BvVec2(XMVector2TransformNormal(m_Vec, m));
}

BV_INLINE BvVec3 BvVec3::operator*(const BvQuat& q) const
{
	return BvVec3(XMVector3Rotate(m_Vec, q));
}

BV_INLINE BvVec3 BvVec3::operator*(const BvMatrix& m) const
{
	return BvVec3(XMVector3TransformCoord(m_Vec, m));
}

BV_INLINE BvVec3& BvVec3::operator*=(const BvQuat& q)
{
	m_Vec = XMVector3Rotate(m_Vec, q);
	return *this;
}

BV_INLINE BvVec3& BvVec3::operator*=(const BvMatrix& m)
{
	m_Vec = XMVector3TransformNormal(m_Vec, m);
	return *this;
}

BV_INLINE BvVec3 BvVec3::Rotate(const BvQuat& q) const
{
	return BvVec3(XMVector3Rotate(m_Vec, q));
}

BV_INLINE BvVec3 BvVec3::InvRotate(const BvQuat& q) const
{
	return BvVec3(XMVector3InverseRotate(m_Vec, q));
}

BV_INLINE BvVec3 BvVec3::TransformPoint(const BvMatrix& m) const
{
	return BvVec3(XMVector3TransformCoord(m_Vec, m));
}

BV_INLINE BvVec3 BvVec3::TransformNormal(const BvMatrix& m) const
{
	return BvVec3(XMVector3TransformNormal(m_Vec, m));
}

BV_INLINE BvVec4 BvVec4::Transform(const BvMatrix& m) const
{
	return BvVec4(XMVector4Transform(m_Vec, m));
}

BV_INLINE BvMatrix::BvMatrix(const BvQuat& q)
	: m_Mat(XMMatrixRotationQuaternion(q))
{
}