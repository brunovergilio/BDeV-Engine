#include "BvMath.h"
#pragma once


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_AVX)


BV_INLINE f32 BV_VCALL VectorGetX(cvf32 v)
{
	return _mm_cvtss_f32(v);
}

BV_INLINE f32 BV_VCALL VectorGetY(cvf32 v)
{
	vf32 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
	return _mm_cvtss_f32(y);
}

BV_INLINE f32 BV_VCALL VectorGetZ(cvf32 v)
{
	vf32 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	return _mm_cvtss_f32(z);
}

BV_INLINE f32 BV_VCALL VectorGetW(cvf32 v)
{
	vf32 w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
	return _mm_cvtss_f32(w);
}

BV_INLINE vf32 BV_VCALL VectorSetX(cvf32 v, f32 s)
{
	return _mm_insert_ps(v, _mm_set_ss(s), 0x0);
}

BV_INLINE vf32 BV_VCALL VectorSetY(cvf32 v, f32 s)
{
	return _mm_insert_ps(v, _mm_set_ss(s), 0x10);
}

BV_INLINE vf32 BV_VCALL VectorSetZ(cvf32 v, f32 s)
{
	return _mm_insert_ps(v, _mm_set_ss(s), 0x20);
}

BV_INLINE vf32 BV_VCALL VectorSetW(cvf32 v, f32 s)
{
	return _mm_insert_ps(v, _mm_set_ss(s), 0x30);
}

BV_INLINE vf32 BV_VCALL VectorSet(f32 s)
{
	return _mm_set1_ps(s);
}

BV_INLINE vf32 BV_VCALL VectorSet(f32 x, f32 y, f32 z, f32 w)
{
	return _mm_set_ps(w, z, y, x);
}

BV_INLINE vf32 BV_VCALL VectorReplicateX(cvf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}

BV_INLINE vf32 BV_VCALL VectorReplicateY(cvf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}

BV_INLINE vf32 BV_VCALL VectorReplicateZ(cvf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}

BV_INLINE vf32 BV_VCALL VectorReplicateW(cvf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

BV_INLINE vf32 BV_VCALL VectorMaskX()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0xFFFFFFFF, 0, 0, 0));
}

BV_INLINE vf32 BV_VCALL VectorMaskY()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0, 0xFFFFFFFF, 0, 0));
}

BV_INLINE vf32 BV_VCALL VectorMaskZ()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0, 0, 0xFFFFFFFF, 0));
}

BV_INLINE vf32 BV_VCALL VectorMaskW()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0, 0, 0, 0xFFFFFFFF));
}

BV_INLINE vf32 BV_VCALL VectorMaskAll()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
}

BV_INLINE vf32 BV_VCALL VectorMaskInvX()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
}

BV_INLINE vf32 BV_VCALL VectorMaskInvY()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0xFFFFFFFF, 0, 0xFFFFFFFF, 0xFFFFFFFF));
}

BV_INLINE vf32 BV_VCALL VectorMaskInvZ()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0xFFFFFFFF));
}

BV_INLINE vf32 BV_VCALL VectorMaskInvW()
{
	return _mm_castsi128_ps(_mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0));
}

template<u32 X, u32 Y, u32 Z, u32 W>
BV_INLINE vf32 BV_VCALL VectorPermute(cvf32 v)
{
	return _mm_shuffle_ps(v, v, _MM_SHUFFLE(W, Z, Y, X));
}

BV_INLINE vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2, i32 mask)
{
	__m128i imm = _mm_setr_epi32((mask & 1) ? 0xFFFFFFFF : 0, (mask & 2) ? 0xFFFFFFFF : 0, (mask & 4) ? 0xFFFFFFFF : 0, (mask & 8) ? 0xFFFFFFFF : 0);

	return _mm_blendv_ps(v2, v1, _mm_castsi128_ps(imm));
}

BV_INLINE vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2, cvf32 mask)
{
	return _mm_blendv_ps(v2, v1, mask);
}

template<u32 X, u32 Y, u32 Z, u32 W>
BV_INLINE vf32 BV_VCALL VectorBlend(cvf32 v1, cvf32 v2)
{
	constexpr i32 s = _MM_SHUFFLE((W & 3), (Z & 3), (Y & 3), (X & 3));
	vf32 s1 = _mm_shuffle_ps(v1, v1, s);
	vf32 s2 = _mm_shuffle_ps(v2, v2, s);

	constexpr i32 b = (X <= 3) | ((Y <= 3) << 1) | ((Z <= 3) << 2) | ((W <= 3) << 3);

	return _mm_blend_ps(s2, s1, b);
}

template<u32 X, u32 Y, u32 Z, u32 W>
BV_INLINE vf32 BV_VCALL VectorChangeSign(cvf32 v)
{
	if constexpr (X | Y | Z | W == 0)
	{
		return v;
	}

	vi32 m = _mm_setr_epi32(X ? 0x80000000 : 0, Y ? 0x80000000 : 0, Z ? 0x80000000 : 0, W ? 0x80000000 : 0);

	return _mm_xor_ps(v, _mm_castsi128_ps(m));
}

BV_INLINE vf32 BV_VCALL VectorMax(cvf32 v1, cvf32 v2)
{
	return _mm_max_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorMin(cvf32 v1, cvf32 v2)
{
	return _mm_min_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorClamp(cvf32 v, cvf32 min, cvf32 max)
{
	return _mm_max_ps(min, _mm_min_ps(v, max));
}

BV_INLINE vf32 BV_VCALL VectorAbs(cvf32 v)
{
	vf32 signMask = _mm_set1_ps(-0.0f);
	return _mm_andnot_ps(signMask, v);
}

BV_INLINE vf32 BV_VCALL VectorFloor(cvf32 v)
{
	return _mm_floor_ps(v);
}

BV_INLINE vf32 BV_VCALL VectorCeil(cvf32 v)
{
	return _mm_ceil_ps(v);
}

BV_INLINE vf32 BV_VCALL VectorRound(cvf32 v)
{
	return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT);
}

BV_INLINE vf32 BV_VCALL VectorSqrt(cvf32 v)
{
	return _mm_sqrt_ps(v);
}

BV_INLINE vf32 BV_VCALL VectorRSqrt(cvf32 v)
{
	return _mm_rsqrt_ps(v);
}

BV_INLINE vf32 BV_VCALL VectorRcp(cvf32 v)
{
	return _mm_rcp_ps(v);
}

BV_INLINE vf32 BV_VCALL VectorNegate(cvf32 v)
{
	return _mm_xor_ps(v, _mm_set1_ps(-0.0f));
}

BV_INLINE vf32 BV_VCALL VectorZero()
{
	return _mm_setzero_ps();
}

BV_INLINE vf32 BV_VCALL VectorOne()
{
	return _mm_set1_ps(1.0f);
}

BV_INLINE vf32 BV_VCALL VectorUnitX()
{
	return _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
}

BV_INLINE vf32 BV_VCALL VectorUnitY()
{
	return _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
}

BV_INLINE vf32 BV_VCALL VectorUnitZ()
{
	return _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
}

BV_INLINE vf32 BV_VCALL VectorUnitW()
{
	return _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
}

BV_INLINE vf32 BV_VCALL Vector2Equal(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4Equal(v1, v2);
	return VectorPermute<0, 1, 1, 1>(r);
}

BV_INLINE vf32 BV_VCALL Vector2NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon)
{
	vf32 r = Vector4NearlyEqual(v1, v2, kEpsilon);
	return VectorPermute<0, 1, 1, 1>(r);
}

BV_INLINE vf32 BV_VCALL Vector2Greater(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4Greater(v1, v2);
	return VectorPermute<0, 1, 1, 1>(r);
}

BV_INLINE vf32 BV_VCALL Vector2GreaterEqual(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4GreaterEqual(v1, v2);
	return VectorPermute<0, 1, 1, 1>(r);
}

BV_INLINE vf32 BV_VCALL Vector2Less(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4Less(v1, v2);
	return VectorPermute<0, 1, 1, 1>(r);
}

BV_INLINE vf32 BV_VCALL Vector2LessEqual(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4LessEqual(v1, v2);
	return VectorPermute<0, 1, 1, 1>(r);
}

BV_INLINE vf32 BV_VCALL Vector3Equal(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4Equal(v1, v2);
	return VectorPermute<0, 1, 2, 2>(r);
}

BV_INLINE vf32 BV_VCALL Vector3NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon = kEpsilon)
{
	vf32 r = Vector4NearlyEqual(v1, v2, kEpsilon);
	return VectorPermute<0, 1, 2, 2>(r);
}

BV_INLINE vf32 BV_VCALL Vector3Greater(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4Greater(v1, v2);
	return VectorPermute<0, 1, 2, 2>(r);
}

BV_INLINE vf32 BV_VCALL Vector3GreaterEqual(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4GreaterEqual(v1, v2);
	return VectorPermute<0, 1, 2, 2>(r);
}

BV_INLINE vf32 BV_VCALL Vector3Less(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4Less(v1, v2);
	return VectorPermute<0, 1, 2, 2>(r);
}

BV_INLINE vf32 BV_VCALL Vector3LessEqual(cvf32 v1, cvf32 v2)
{
	vf32 r = Vector4LessEqual(v1, v2);
	return VectorPermute<0, 1, 2, 2>(r);
}

BV_INLINE vf32 BV_VCALL Vector4Equal(cvf32 v1, cvf32 v2)
{
	return _mm_cmpeq_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL Vector4NearlyEqual(cvf32 v1, cvf32 v2, f32 epsilon)
{
	vf32 vDiff = VectorAbs(VectorSub(v1, v2));
	vf32 vE = VectorSet(epsilon);

	return Vector4LessEqual(vDiff, vE);
}

BV_INLINE vf32 BV_VCALL Vector4Greater(cvf32 v1, cvf32 v2)
{
	return _mm_cmpgt_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL Vector4GreaterEqual(cvf32 v1, cvf32 v2)
{
	return _mm_cmpge_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL Vector4Less(cvf32 v1, cvf32 v2)
{
	return _mm_cmplt_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL Vector4LessEqual(cvf32 v1, cvf32 v2)
{
	return _mm_cmple_ps(v1, v2);
}

BV_INLINE bool BV_VCALL Vector3AllTrue(cvf32 v)
{
	return VectorAllTrue(VectorPermute<0, 1, 2, 2>(v));
}

BV_INLINE bool BV_VCALL Vector3AllFalse(cvf32 v)
{
	return VectorAllFalse(VectorPermute<0, 1, 2, 2>(v));
}

BV_INLINE bool BV_VCALL Vector3AnyTrue(cvf32 v)
{
	return VectorAnyTrue(VectorPermute<0, 1, 2, 2>(v));
}

BV_INLINE bool BV_VCALL Vector3AnyFalse(cvf32 v)
{
	return VectorAnyFalse(VectorPermute<0, 1, 2, 2>(v));
}

BV_INLINE bool BV_VCALL VectorAllTrue(cvf32 v)
{
	return VectorGetMask(v) == 0xF;
}

BV_INLINE bool BV_VCALL VectorAllFalse(cvf32 v)
{
	return VectorGetMask(v) == 0x0;
}

BV_INLINE bool BV_VCALL VectorAnyTrue(cvf32 v)
{
	return VectorGetMask(v) != 0x0;
}

BV_INLINE bool BV_VCALL VectorAnyFalse(cvf32 v)
{
	return VectorGetMask(v) != 0xF;
}

BV_INLINE vf32 BV_VCALL VectorOr(cvf32 v1, cvf32 v2)
{
	return _mm_or_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorAnd(cvf32 v1, cvf32 v2)
{
	return _mm_and_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorXor(cvf32 v1, cvf32 v2)
{
	return _mm_xor_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorNot(cvf32 v)
{
	return _mm_xor_ps(v, _mm_castsi128_ps(_mm_set1_epi32(0xFFFFFFFF)));
}

BV_INLINE vf32 BV_VCALL VectorAndNot(cvf32 v1, cvf32 v2)
{
	return _mm_andnot_ps(v1, v2);
}

BV_INLINE i32 BV_VCALL VectorGetMask(cvf32 v)
{
	return _mm_movemask_ps(v);
}

BV_INLINE vf32 BV_VCALL VectorAdd(cvf32 v1, cvf32 v2)
{
	return _mm_add_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorSub(cvf32 v1, cvf32 v2)
{
	return _mm_sub_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorMul(cvf32 v1, cvf32 v2)
{
	return _mm_mul_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorDiv(cvf32 v1, cvf32 v2)
{
	return _mm_div_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL VectorMAdd(cvf32 v1, cvf32 v2, cvf32 v3)
{
	return _mm_fmadd_ps(v1, v2, v3);
}

BV_INLINE vf32 BV_VCALL VectorNMAdd(cvf32 v1, cvf32 v2, cvf32 v3)
{
	return _mm_fnmadd_ps(v1, v2, v3);
}

BV_INLINE vf32 BV_VCALL VectorMSub(cvf32 v1, cvf32 v2, cvf32 v3)
{
	return _mm_fmsub_ps(v1, v2, v3);
}

BV_INLINE vf32 BV_VCALL VectorNMSub(cvf32 v1, cvf32 v2, cvf32 v3)
{
	return _mm_fnmsub_ps(v1, v2, v3);
}

BV_INLINE vf32 BV_VCALL VectorAdd(cvf32 v, f32 val)
{
	return _mm_add_ps(v, _mm_set_ps1(val));
}

BV_INLINE vf32 BV_VCALL VectorSub(cvf32 v, f32 val)
{
	return _mm_sub_ps(v, _mm_set_ps1(val));
}

BV_INLINE vf32 BV_VCALL VectorMul(cvf32 v, f32 val)
{
	return _mm_mul_ps(v, _mm_set_ps1(val));
}

BV_INLINE vf32 BV_VCALL VectorDiv(cvf32 v, f32 val)
{
	return _mm_div_ps(v, _mm_set_ps1(val));
}

BV_INLINE vf32 BV_VCALL VectorLerp(cvf32 v1, cvf32 v2, f32 t)
{
	// Lerp = V0 + t * (V1 - V0)
	vf32 vT = _mm_set1_ps(t);

	// (t * v2) + (-(t * v1) + v1)
	return VectorMAdd(vT, v2, VectorNMAdd(vT, v1, v1));
}

BV_INLINE vf32 BV_VCALL Vector2Dot(cvf32 v1, cvf32 v2)
{
	return _mm_dp_ps(v1, v2, 0x3F);
}

BV_INLINE vf32 BV_VCALL Vector2Cross(cvf32 v1, cvf32 v2)
{
	vf32 s2 = VectorPermute<1, 0, 1, 0>(v2);
	vf32 r = VectorMul(v1, s2);

	s2 = VectorPermute<1, 1, 1, 1>(r);

	r = VectorSub(r, s2);

	return VectorPermute<0, 0, 0, 0>(r);
}

BV_INLINE vf32 BV_VCALL Vector2LengthSqr(cvf32 v)
{
	return Vector2Dot(v, v);
}

BV_INLINE vf32 BV_VCALL Vector2Length(cvf32 v)
{
	return _mm_sqrt_ps(Vector2LengthSqr(v));
}

BV_INLINE vf32 BV_VCALL Vector2LengthRcp(cvf32 v)
{
	return _mm_rsqrt_ps(Vector2LengthSqr(v));
}

BV_INLINE vf32 BV_VCALL Vector2Normalize(cvf32 v)
{
	return _mm_mul_ps(v, _mm_rsqrt_ps(Vector2LengthSqr(v)));
}

BV_INLINE vf32 BV_VCALL Vector2TransformDir(cvf32 v, cmf32 m)
{
	vf32 c = VectorReplicateX(v);
	vf32 r = _mm_mul_ps(c, m.r[0]);

	c = VectorReplicateY(v);
	r = _mm_fmadd_ps(c, m.r[1], r);

	return r;
}

BV_INLINE vf32 BV_VCALL Vector2TransformPoint(cvf32 v, cmf32 m)
{
	vf32 c = VectorReplicateX(v);
	vf32 r = _mm_fmadd_ps(c, m.r[0], m.r[3]);

	c = VectorReplicateY(v);
	r = _mm_fmadd_ps(c, m.r[1], r);

	return r;
}

BV_INLINE vf32 BV_VCALL Vector3Dot(cvf32 v1, cvf32 v2)
{
	// _mm_dp_ps will use a 8 bit mask where:
	// bits 7, 6, 5 and 4 describe which elements to multiply
	// bits 3, 2, 1 and 0 describe where elements will be stored

	// For this implementation, 0111 1111 will be used, so x, y and z will get multiplied
	// and the value is stored across all elements
	return _mm_dp_ps(v1, v2, 0x7F);
}

BV_INLINE vf32 BV_VCALL Vector3Cross(cvf32 v1, cvf32 v2)
{
	// Using _MM_SHUFFLE macro to change the element places (reverse order)
	// A(3, 2, 1, 0)
	// B(3, 2, 1, 0)
	// SHUFFLE AB (0, 1, 2, 3) = (B0, B1, A2, A3) (reverse)
	// Result => (A.w, A.z, B.y, B.x)

	// (v1.y, v1.z, v1.x, v1.w) * (v2.z, v2.x, v2.y, v2.w)
	// -
	// (v1.z, v1.x, v1.y, v1.w) * (v2.y, v2.z, v2.x, v2.w)
	vf32 r = _mm_sub_ps
	(
		_mm_mul_ps(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 1, 0, 2))),
		_mm_mul_ps(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 0, 2, 1)))
	);

	vi32 w0 = _mm_set_epi32(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	return _mm_and_ps(r, _mm_castsi128_ps(w0));
}

BV_INLINE vf32 BV_VCALL Vector3LengthSqr(cvf32 v)
{
	return Vector3Dot(v, v);
}

BV_INLINE vf32 BV_VCALL Vector3Length(cvf32 v)
{
	return _mm_sqrt_ps(Vector3LengthSqr(v));
}

BV_INLINE vf32 BV_VCALL Vector3LengthRcp(cvf32 v)
{
	return _mm_rsqrt_ps(Vector3LengthSqr(v));
}

BV_INLINE vf32 BV_VCALL Vector3Normalize(cvf32 v)
{
	return _mm_mul_ps(v, _mm_rsqrt_ps(Vector3LengthSqr(v)));
}

BV_INLINE vf32 BV_VCALL Vector3Project(cvf32 v, cvf32 p)
{
	// Projection = (V.P) * (P / ||P||)
	vf32 vDotP = Vector3Dot(v, p);
	vf32 pN = Vector3Normalize(p);
	return VectorMul(vDotP, pN);
}

BV_INLINE vf32 BV_VCALL Vector3ProjectNormal(cvf32 v, cvf32 p)
{
	// Projection = (V.P) * (P / ||P||)
	vf32 vDotP = Vector3Dot(v, p);
	return VectorMul(vDotP, p);
}

BV_INLINE vf32 BV_VCALL Vector3Reflection(cvf32 i, cvf32 n)
{
	// Reflection = I - 2(I.N)N
	vf32 twoTimesProj = VectorMul(Vector3Dot(i, n), n);
	twoTimesProj = VectorAdd(twoTimesProj, twoTimesProj);

	return VectorSub(i, twoTimesProj);
}

BV_INLINE vf32 BV_VCALL Vector3Refraction(cvf32 i, cvf32 n, f32 eta)
{
	vf32 v1 = VectorSet(1.0f);
	vf32 iDotN = Vector3Dot(i, n);
	vf32 iDotN2 = VectorMul(iDotN, iDotN);
	vf32 vEta = VectorSet(eta);
	vf32 vEta2 = VectorMul(vEta, vEta);

	// 1.0f - (-(iDotN^2 * eta^2) + eta^2) = 1.0f - eta^2 * (1.0f - iDotN^2)
	vf32 vK = VectorSub(v1, _mm_fnmadd_ps(iDotN2, vEta2, vEta2));

	vf32 c0 = Vector3Less(vK, VectorZero());

	// eta * i - (eta * iDotN + sqrtf(k)) * n;
	auto r0 = VectorMul(n, iDotN); // n * iDotN
	auto r1 = VectorMul(n, _mm_sqrt_ps(vK)); // n * sqrtf(k)
	r1 = VectorMAdd(vEta, r0, r1); // eta * n * iDotN + n * sqrtf(k)
	r0 = VectorMul(vEta, i); // eta * i
	r0 = VectorSub(r0, r1); // eta * i - eta * n * iDotN - n * sqrtf(k)

	return _mm_andnot_ps(c0, r0);
}

BV_INLINE vf32 BV_VCALL Vector3TransformDir(cvf32 v, cmf32 m)
{
	vf32 c = VectorReplicateX(v);
	vf32 r = _mm_mul_ps(c, m.r[0]);

	c = VectorReplicateY(v);
	r = _mm_fmadd_ps(c, m.r[1], r);

	c = VectorReplicateZ(v);
	r = _mm_fmadd_ps(c, m.r[2], r);

	return r;
}

BV_INLINE vf32 BV_VCALL Vector3TransformPoint(cvf32 v, cmf32 m)
{
	vf32 c = VectorReplicateX(v);
	vf32 r = _mm_fmadd_ps(c, m.r[0], m.r[3]);

	c = VectorReplicateY(v);
	r = _mm_fmadd_ps(c, m.r[1], r);

	c = VectorReplicateZ(v);
	r = _mm_fmadd_ps(c, m.r[2], r);

	return r;
}

BV_INLINE vf32 BV_VCALL Vector4Dot(cvf32 v1, cvf32 v2)
{
	// _mm_dp_ps will use a 8 bit mask where:
	// bits 7, 6, 5 and 4 describe which elements to multiply
	// bits 3, 2, 1 and 0 describe where elements will be stored

	// For this implementation, 1111 1111 will be used, so x, y, z and w will get multiplied
	// and the value is stored across all elements
	return _mm_dp_ps(v1, v2, 0xFF);
}

BV_INLINE vf32 BV_VCALL Vector4LengthSqr(cvf32 v)
{
	return Vector4Dot(v, v);
}

BV_INLINE vf32 BV_VCALL Vector4Length(cvf32 v)
{
	return _mm_sqrt_ps(Vector4LengthSqr(v));
}

BV_INLINE vf32 BV_VCALL Vector4LengthRcp(cvf32 v)
{
	return _mm_rsqrt_ps(Vector4LengthSqr(v));
}

BV_INLINE vf32 BV_VCALL VectorTransform(cvf32 v, cmf32 m)
{
	vf32 c = VectorReplicateX(v);
	vf32 r = _mm_mul_ps(c, m.r[0]);

	c = VectorReplicateY(v);
	r = _mm_fmadd_ps(c, m.r[1], r);

	c = VectorReplicateZ(v);
	r = _mm_fmadd_ps(c, m.r[2], r);

	c = VectorReplicateW(v);
	r = _mm_fmadd_ps(c, m.r[3], r);

	return r;
}

BV_INLINE vf32 BV_VCALL operator + (cvf32 v)
{
	return v;
}

BV_INLINE vf32 BV_VCALL operator - (cvf32 v)
{
	return VectorNegate(v);
}

BV_INLINE vf32 BV_VCALL operator + (cvf32 v1, cvf32 v2)
{
	return VectorAdd(v1, v2);
}

BV_INLINE vf32 BV_VCALL operator+(cvf32 v, f32 s)
{
	return VectorAdd(v, s);
}

BV_INLINE vf32 BV_VCALL operator+(f32 s, cvf32 v)
{
	return VectorAdd(v, s);
}

BV_INLINE vf32& BV_VCALL operator += (cvf32& v1, cvf32 v2)
{
	v1 = VectorAdd(v1, v2);
	return v1;
}

BV_INLINE vf32 BV_VCALL operator - (cvf32 v1, cvf32 v2)
{
	return VectorSub(v1, v2);
}

BV_INLINE vf32 BV_VCALL operator-(cvf32 v, f32 s)
{
	return VectorSub(v, s);
}

BV_INLINE vf32 BV_VCALL operator-(f32 s, cvf32 v)
{
	return VectorSub(VectorSet(s), v);
}

BV_INLINE vf32& BV_VCALL operator -= (cvf32& v1, cvf32 v2)
{
	v1 = VectorSub(v1, v2);
	return v1;
}

BV_INLINE vf32 BV_VCALL operator * (cvf32 v1, cvf32 v2)
{
	return VectorMul(v1, v2);
}

BV_INLINE vf32& BV_VCALL operator *= (cvf32& v1, cvf32 v2)
{
	v1 = VectorMul(v1, v2);
	return v1;
}

BV_INLINE vf32 BV_VCALL operator * (cvf32 v, f32 s)
{
	return VectorMul(v, s);
}

BV_INLINE vf32 BV_VCALL operator * (f32 s, cvf32 v)
{
	return VectorMul(v, s);
}

BV_INLINE vf32& BV_VCALL operator *= (cvf32& v, f32 s)
{
	v = VectorMul(v, s);
	return v;
}

BV_INLINE vf32 BV_VCALL operator * (cvf32 v, cmf32 m)
{
	return VectorTransform(v, m);
}

BV_INLINE vf32& BV_VCALL operator *= (cvf32& v, cmf32 m)
{
	v = VectorTransform(v, m);
	return v;
}

BV_INLINE vf32 BV_VCALL operator / (cvf32 v1, cvf32 v2)
{
	return VectorDiv(v1, v2);
}

BV_INLINE vf32& BV_VCALL operator /= (cvf32& v1, cvf32 v2)
{
	v1 = VectorDiv(v1, v2);
	return v1;
}

BV_INLINE vf32 BV_VCALL operator / (cvf32 v, f32 s)
{
	return VectorDiv(v, s);
}

BV_INLINE vf32& BV_VCALL operator /= (cvf32& v, f32 s)
{
	v = VectorDiv(v, s);
	return v;
}

#else

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

inline vf32& operator += (vf32& v1, crvf32 v2)
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

inline vf32& operator -= (vf32& v1, crvf32 v2)
{
	v1 = VectorSub(v1, v2);
	return v1;
}

inline vf32 operator * (crvf32 v1, crvf32 v2)
{
	return VectorMul(v1, v2);
}

inline vf32& operator *= (vf32& v1, crvf32 v2)
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

inline vf32& operator *= (vf32& v, f32 s)
{
	v = VectorMul(v, s);
	return v;
}

inline vf32 operator * (crvf32 v, const mf32& m)
{
	return VectorMul(v, m);
}

inline vf32 operator*(const mf32& m, crvf32 v)
{
	return VectorMul(m, v);
}

inline vf32 operator / (crvf32 v1, crvf32 v2)
{
	return VectorDiv(v1, v2);
}

inline vf32& operator /= (vf32& v1, crvf32 v2)
{
	v1 = VectorDiv(v1, v2);
	return v1;
}

inline vf32 operator / (crvf32 v, f32 s)
{
	return VectorDiv(v, s);
}

inline vf32& operator /= (vf32& v, f32 s)
{
	v = VectorDiv(v, s);
	return v;
}

inline vf32 VectorTransformDir(crvf32 v, const mf32& m)
{
	return vf32(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z,
		0.0f
	);
}

inline vf32 VectorTransformPoint(crvf32 v, const mf32& m)
{
	return vf32(
		v.x * m.r[0].x + v.y * m.r[1].x + v.z * m.r[2].x + m.r[3].x,
		v.x * m.r[0].y + v.y * m.r[1].y + v.z * m.r[2].y + m.r[3].y,
		v.x * m.r[0].z + v.y * m.r[1].z + v.z * m.r[2].z + m.r[3].z,
		1.0f
	);
}

#endif