#pragma once


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_AVX)

BV_INLINE vf32 BV_VCALL QuaternionIdentity()
{
	return VectorSet(0.0f, 0.0f, 0.0f, 1.0f);
}

BV_INLINE vf32 BV_VCALL QuaternionMul(cvf32 q1, cvf32 q2)
{
	// [q1s * q2v + q2s * q1v + q1 x q2] [q1s * q2s - q1.q2]
	// Expanding the equation
	// x => q1.w * q2.x + q2.w * q1.x + q1.y * q2.z - q1.z * q2.y
	// y => q1.w * q2.y + q2.w * q1.y + q1.z * q2.x - q1.x * q2.z
	// z => q1.w * q2.z + q2.w * q1.z + q1.x * q2.y - q1.y * q2.x
	// w => q1.w * q2.w - q2.x * q1.x - q1.y * q2.y - q1.z * q2.z

	// v0 = q1.w, q1.w, q1.w, q1.w
	vf32 v0 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(3, 3, 3, 3));

	// r = q1.w * q2.x, q1.w * q2.y, q1.w * q2.z, q1.w * q2.w
	vf32 r = _mm_mul_ps(v0, q2);

	// v1 = q2.w, q2.w, q2.w, q2.x
	vf32 v1 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(0, 3, 3, 3));
	// v2 = q1.x, q1.y, q1.z, q1.x
	vf32 v2 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(0, 2, 1, 0));

	// c0 = 1.0, 1.0, 1.0, -1.0
	vf32 c0 = _mm_set_ps(-1.0f, 1.0f, 1.0f, 1.0f);

	// v1 = q2.w * q1.x, q2.w * q1.y, q2.w * q1.z, q2.x * q1.x
	v1 = _mm_mul_ps(v1, v2);
	// v1 = q2.w * q1.x, q2.w * q1.y, q2.w * q1.z, -q2.x * q1.x
	v1 = _mm_mul_ps(v1, c0);

	// Perform first addition
	r = _mm_add_ps(r, v1);

	// v1 = q1.y, q1.z, q1.x, q1.y
	v1 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(1, 0, 2, 1));
	// v2 = q2.z, q2.x, q2.y, q2.y
	v2 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(1, 1, 0, 2));

	// v1 = q1.y * q2. z, q1.z * q2.x, q1.x * q2.y, q1.y * q2.y
	v1 = _mm_mul_ps(v1, v2);
	// v1 = q1.y * q2. z, q1.z * q2.x, q1.x * q2.y, -q1.y * q2.y
	v1 = _mm_mul_ps(v1, c0);

	// Perform second addition
	r = _mm_add_ps(r, v1);

	// v1 = q1.z, q1.x, q1.y, q1.z
	v1 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(2, 1, 0, 2));
	// v2 = q2.y, q2.z, q2.x, q2.z
	v2 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(2, 0, 2, 1));

	// v1 = q1.z * q2.y, q1.x * q2.z, q1.y * q2.x, q1.z * q2.z
	v1 = _mm_mul_ps(v1, v2);

	// Perform a subtraction
	r = _mm_sub_ps(r, v1);

	return r;
}

BV_INLINE vf32 BV_VCALL QuaternionMulKeenan(cvf32 q1, cvf32 q2)
{
	// [q1s * q2v + q2s * q1v + q2 x q1] [q1s * q2s - q1.q2]
	// Expanding the equation
	// x => q1.w * q2.x + q2.w * q1.x + q2.y * q1.z - q2.z * q1.y
	// y => q1.w * q2.y + q2.w * q1.y + q2.z * q1.x - q2.x * q1.z
	// z => q1.w * q2.z + q2.w * q1.z + q2.x * q1.y - q2.y * q1.x
	// w => q1.w * q2.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z

	// v0 = q1.w, q1.w, q1.w, q1.w
	vf32 v0 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(3, 3, 3, 3));

	// r = q1.w * q2.x, q1.w * q2.y, q1.w * q2.z, q1.w * q2.w
	vf32 r = _mm_mul_ps(v0, q2);

	// v1 = q2.w, q2.w, q2.w, q2.x
	vf32 v1 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(0, 3, 3, 3));
	// v2 = q1.x, q1.y, q1.z, q1.x
	vf32 v2 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(0, 2, 1, 0));

	// c0 = 1.0, 1.0, 1.0, -1.0
	vf32 c0 = _mm_set_ps(-1.0f, 1.0f, 1.0f, 1.0f);

	// v1 = q2.w * q1.x, q2.w * q1.y, q2.w * q1.z, q2.x * q1.x
	v1 = _mm_mul_ps(v1, v2);
	// v1 = q2.w * q1.x, q2.w * q1.y, q2.w * q1.z, -q2.x * q1.x
	v1 = _mm_mul_ps(v1, c0);

	// Perform first addition
	r = _mm_add_ps(r, v1);

	// v1 = q2.y, q2.z, q2.x, q2.y
	v1 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(1, 0, 2, 1));
	// v2 = q1.z, q1.x, q1.y, q1.y
	v2 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(1, 1, 0, 2));

	// v1 = q2.y * q1. z, q2.z * q1.x, q2.x * q1.y, q2.y * q1.y
	v1 = _mm_mul_ps(v1, v2);
	// v1 = q2.y * q1. z, q2.z * q1.x, q2.x * q1.y, -q2.y * q1.y
	v1 = _mm_mul_ps(v1, c0);

	// Perform second addition
	r = _mm_add_ps(r, v1);

	// v1 = q2.z, q2.x, q2.y, q2.z
	v1 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(2, 1, 0, 2));
	// v2 = q1.y, q1.z, q1.x, q1.z
	v2 = _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(2, 0, 2, 1));

	// v1 = q2.z * q1.y, q2.x * q1.z, q2.y * q1.x, q2.z * q1.z
	v1 = _mm_mul_ps(v1, v2);

	// Perform a subtraction
	r = _mm_sub_ps(r, v1);

	return r;
}

BV_INLINE vf32 BV_VCALL QuaternionConjugate(cvf32 q)
{
	return VectorChangeSign<1, 1, 1, 0>(q);
}

BV_INLINE vf32 BV_VCALL QuaternionInverse(cvf32 q)
{
	vf32 conj = QuaternionConjugate(q);
	vf32 oneOverLengthSqr = _mm_rcp_ps(QuaternionLengthSqr(q));

	return _mm_mul_ps(conj, oneOverLengthSqr);
}

BV_INLINE vf32 BV_VCALL QuaternionInverseN(cvf32 q)
{
	return QuaternionConjugate(q);
}

BV_INLINE vf32 BV_VCALL QuaternionNormalize(cvf32 q)
{
	return _mm_mul_ps(q, _mm_rsqrt_ps(QuaternionLengthSqr(q)));
}

BV_INLINE vf32 BV_VCALL QuaternionDot(cvf32 q1, cvf32 q2)
{
	// _mm_dp_ps will use a 8 bit mask where:
	// bits 7, 6, 5 and 4 describe which elements to Mul
	// bits 3, 2, 1 and 0 describe where elements will be stored

	// For this implementation, 1111 1111 will be used, so x, y, z and w will get multiplied
	// and the value is stored across all elements
	return _mm_dp_ps(q1, q2, 0xFF);
}

BV_INLINE vf32 BV_VCALL QuaternionLengthSqr(cvf32 q)
{
	return QuaternionDot(q, q);
}

BV_INLINE vf32 BV_VCALL QuaternionLength(cvf32 v)
{
	return _mm_sqrt_ps(QuaternionLengthSqr(v));
}

BV_INLINE vf32 BV_VCALL QuaternionRotationAxis(cvf32 v, f32 angle)
{
	f32 halfAngle = angle * 0.5f;
	f32 sinCos[] = { sinf(halfAngle), cosf(halfAngle) };
	vf32 q = VectorSet(sinCos[0], sinCos[0], sinCos[0], sinCos[1]);
	vf32 n = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	n = _mm_or_ps(v, n);

	return VectorMul(q, n);
}

BV_INLINE vf32 BV_VCALL QuaternionQVQC(cvf32 q, cvf32 v)
{
	vf32 v0 = VectorReplicateW(q);
	vf32 v1 = VectorMul(v0, v0);
	v0 = VectorAdd(v0, v0);

	vf32 v2 = Vector4Dot(q, q);
	v2 = VectorSub(v1, v2);
	v2 = VectorMul(v2, v);

	v1 = Vector3Cross(q, v);
	v1 = VectorMul(v0, v1);

	v0 = Vector3Dot(q, v);
	v0 = VectorAdd(v0, v0);
	v0 = VectorMul(v0, q);

	return VectorAdd(v0, VectorAdd(v1, v2));
}

BV_INLINE vf32 BV_VCALL QuaternionQCVQ(cvf32 q, cvf32 v)
{
	vf32 v0 = VectorReplicateW(q);
	vf32 v1 = VectorMul(v0, v0);
	v0 = VectorAdd(v0, v0);

	vf32 v2 = Vector4Dot(q, q);
	v2 = VectorSub(v1, v2);
	v2 = VectorMul(v2, v);

	v1 = Vector3Cross(v, q);
	v1 = VectorMul(v0, v1);

	v0 = Vector3Dot(q, v);
	v0 = VectorAdd(v0, v0);
	v0 = VectorMul(v0, q);

	return VectorAdd(v0, VectorAdd(v1, v2));
}

BV_INLINE vf32 BV_VCALL QuaternionQVQCKeenan(cvf32 q, cvf32 v)
{
	return QuaternionQCVQ(q, v);
}

BV_INLINE vf32 BV_VCALL QuaternionQCVQKeenan(cvf32 q, cvf32 v)
{
	return QuaternionQVQC(q, v);
}

BV_INLINE vf32 BV_VCALL QuaternionFromMatrix(cmf32 m)
{
	vf32 v0;

	// z0 = 0, 0, 0, 0
	vf32 z0 = _mm_setzero_ps();

	vf32 if0, if1, if2, if3;
	{
		// m00 = m00, m00, m00, m00
		vf32 m00 = _mm_shuffle_ps(m.r[0], m.r[0], _MM_SHUFFLE(0, 0, 0, 0));
		// m11 = m11, m11, m11, m11
		vf32 m11 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(1, 1, 1, 1));
		// m22 = m22, m22, m22, m22
		vf32 m22 = _mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(2, 2, 2, 2));

		// v0 = m00 + m11, m00 + m11, m00 + m11, m00 + m11
		v0 = _mm_add_ps(m00, m11);
		// v0 = m00 + m011 + m22, m00 + m011 + m22, m00 + m011 + m22, m00 + m011 + m22
		v0 = _mm_add_ps(v0, m22);

		// Check all 4 conditions

		// if0 = v0 > z0 ? 0xffffffff : 0
		if0 = _mm_cmpgt_ps(v0, z0);
		// if1 = m00 > m11 ? 0xffffffff : 0
		if1 = _mm_cmpgt_ps(m00, m11);
		// if11 = m00 > m22 ? 0xffffffff : 0
		vf32 if11 = _mm_cmpgt_ps(m00, m22);
		// if1 = if1  > 0 ? 0xffffffff : 0
		if1 = _mm_cmpgt_ps(if1, z0);
		// if11 = if11  > 0 ? 0xffffffff : 0
		if11 = _mm_cmpgt_ps(if11, z0);
		// if1 = if1 && if11
		if1 = _mm_and_ps(if1, if11);

		// if2 = m11 > m22 ? 0xffffffff : 0
		if2 = _mm_cmpgt_ps(m11, m22);

		// if3 = if0 | if1
		if3 = _mm_or_ps(if0, if1);
		// if3 = if0 | if1 | if2
		if3 = _mm_or_ps(if3, if2);
		// if3 = (if0 | if1 | if2) == 0 ? 0xffffffff : 0
		if3 = _mm_cmpeq_ps(if3, z0);
	}


	// m0 = m00, m00, m11, m11
	vf32 m0 = _mm_shuffle_ps(m.r[0], m.r[1], _MM_SHUFFLE(1, 1, 0, 0));
	// m1 = 1, 1, -1, -1
	vf32 m1 = _mm_set_ps(-1.0f, -1.0f, 1.0f, 1.0f);
	// m2 = m22, m22, 1, 1
	vf32 m2 = _mm_shuffle_ps(m.r[2], m1, _MM_SHUFFLE(0, 0, 2, 2));

	// m0 = m00, m11, m22, 1
	m0 = _mm_shuffle_ps(m0, m2, _MM_SHUFFLE(2, 0, 2, 0));
	// m0 = 1, m00, m11, m22
	m0 = _mm_shuffle_ps(m0, m0, _MM_SHUFFLE(2, 1, 0, 3));


	// Control signs

	// c0 = 1, 1, 1, 1
	vf32 c0 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(0, 0, 0, 0));
	// c1 = 1, 1, -1, -1
	vf32 c1 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(2, 2, 0, 0));
	// c2 = 1, -1, 1, -1
	vf32 c2 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(2, 0, 2, 0));
	// c3 = 1, -1, -1, 1
	vf32 c3 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(0, 2, 2, 0));

	// m2 = 0.25, 0, 0, 0
	m2 = _mm_set_ss(0.25f);

	// First block of the component-wise addition

	// s0 = m02, m02, m10, m10
	vf32 s0 = _mm_shuffle_ps(m.r[0], m.r[1], _MM_SHUFFLE(0, 0, 2, 2));
	// s0 = m23, m21, m02, m10
	s0 = _mm_shuffle_ps(m.r[2], s0, _MM_SHUFFLE(2, 0, 1, 3));
	// s0 = 0.25, m21, m02, m10
	s0 = _mm_move_ss(s0, m2);

	// Second block of the component-wise addition

	// s1 = m12, m12, m20, m20
	vf32 s1 = _mm_shuffle_ps(m.r[1], m.r[2], _MM_SHUFFLE(0, 0, 2, 2));
	// s1 = 0, m01, m12, m20
	s1 = _mm_shuffle_ps(m.r[0], s1, _MM_SHUFFLE(2, 0, 1, 3));
	// s1 = 0, m12, m20, m01
	s1 = _mm_shuffle_ps(s1, s1, _MM_SHUFFLE(1, 3, 2, 0));

	// Check every condition
	c0 = _mm_and_ps(c0, if0);
	c1 = _mm_and_ps(c1, if1);
	c2 = _mm_and_ps(c2, if2);
	c3 = _mm_and_ps(c3, if3);

	// Then concantenate them all
	c0 = _mm_or_ps(c0, c1);
	c0 = _mm_or_ps(c0, c2);
	c0 = _mm_or_ps(c0, c3);

	// Change the signs of the values that will be summed
	m0 = _mm_mul_ps(m0, c0);
	// m0 = m0.x + m0.y, m0.z + m0.w, m0.x + m0.y, m0.z + m0.w
	m0 = _mm_hadd_ps(m0, m0);
	// m0 = m0.x + m0.y + m0.z + m0.w, m0.x + m0.y + m0.z + m0.w, m0.x + m0.y + m0.z + m0.w, m0.x + m0.y + m0.z + m0.w
	m0 = _mm_hadd_ps(m0, m0);
	m0 = _mm_rsqrt_ps(m0);
	c0 = _mm_set1_ps(0.5f);
	m0 = _mm_mul_ps(m0, c0);
	m0 = _mm_rcp_ss(m0);


	// Control signs for the formula

	// c0 = -1, -1, -1, 1
	c0 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(0, 2, 2, 2));
	// c1 = 1, 1, 1, -1
	c1 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(2, 0, 0, 0));
	// c2 = 1, 1, 1, -1
	c2 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(2, 0, 0, 0));
	// c3 = 1, 1, 1, -1
	c3 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(2, 0, 0, 0));

	// Check every condition
	c0 = _mm_and_ps(c0, if0);
	c1 = _mm_and_ps(c1, if1);
	c2 = _mm_and_ps(c2, if2);
	c3 = _mm_and_ps(c3, if3);

	// Then concantenate them all
	c0 = _mm_or_ps(c0, c1);
	c0 = _mm_or_ps(c0, c2);
	c0 = _mm_or_ps(c0, c3);

	s1 = _mm_mul_ps(s1, c0);
	s0 = _mm_add_ps(s0, s1);
	s0 = _mm_mul_ps(s0, m0);

	{
		vf32 shuffle0 = _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(0, 3, 2, 1));
		vf32 shuffle1 = _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(1, 2, 3, 0));
		vf32 shuffle2 = _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(2, 1, 0, 3));
		vf32 shuffle3 = _mm_shuffle_ps(s0, s0, _MM_SHUFFLE(3, 0, 1, 2));

		shuffle0 = _mm_and_ps(shuffle0, if0);
		shuffle1 = _mm_and_ps(shuffle1, if1);
		shuffle2 = _mm_and_ps(shuffle2, if2);
		shuffle3 = _mm_and_ps(shuffle3, if3);

		// Then concatenate them all
		shuffle0 = _mm_or_ps(shuffle0, shuffle1);
		shuffle0 = _mm_or_ps(shuffle0, shuffle2);
		shuffle0 = _mm_or_ps(shuffle0, shuffle3);

		s0 = shuffle0;
	}

	return QuaternionNormalize(s0);
}

BV_INLINE vf32 BV_VCALL QuaternionSlerp(cvf32 q1, cvf32 q2, f32 t, f32 epsilon)
{
	vf32 c0 = QuaternionDot(q1, q2);
	vf32 v0 = VectorNegate(c0);
	vf32 q = VectorNegate(q2);

	vf32 v1 = VectorZero();
	vf32 c1 = _mm_cmplt_ps(c0, v1);

	v0 = _mm_and_ps(v0, c1);
	c0 = _mm_andnot_ps(c1, c0);
	c0 = _mm_or_ps(c0, v0);

	q = _mm_and_ps(q, c1);
	q = _mm_or_ps(q, _mm_andnot_ps(c1, q2));

	v0 = VectorSet(epsilon);

	vf32 vAngle = VectorSet(acosf(VectorGetX(c0)));
	c1 = _mm_cmpgt_ps(vAngle, v0);

	c0 = VectorSet(1.0f);
	vf32 lerp2 = VectorSet(t);
	vf32 lerp1 = _mm_sub_ps(c0, lerp2);

	vf32 oneOverSinAngle = VectorSet(sinf(VectorGetX(vAngle)));
	oneOverSinAngle = _mm_rcp_ps(oneOverSinAngle);
	v1 = _mm_mul_ps(lerp1, vAngle);
	v1 = VectorSet(sinf(VectorGetX(v1)));
	v1 = _mm_mul_ps(v1, oneOverSinAngle);

	vf32 v2 = _mm_mul_ps(lerp2, vAngle);
	v2 = VectorSet(sinf(VectorGetX(v2)));
	v2 = _mm_mul_ps(v2, oneOverSinAngle);

	v1 = _mm_and_ps(c1, v1);
	v2 = _mm_and_ps(c1, v2);

	lerp1 = _mm_andnot_ps(c1, lerp1);
	lerp2 = _mm_andnot_ps(c1, lerp2);

	v1 = _mm_or_ps(v1, lerp1);
	v2 = _mm_or_ps(v2, lerp2);

	v1 = _mm_mul_ps(v1, q1);
	v2 = _mm_mul_ps(v2, q);

	return _mm_add_ps(v1, v2);
}

BV_INLINE vf32 BV_VCALL QuaternionAngle(cvf32 q)
{
	return VectorSet(2.0f * acosf(VectorGetW(q)));
}

BV_INLINE bool BV_VCALL QuaternionIsUnit(cvf32 q, f32 epsilon)
{
	vf32 d = VectorSub(QuaternionLength(q), VectorOne());
	vf32 r = VectorLess(VectorAbs(d), VectorSet(epsilon));

	return VectorAllTrue(r);
}

#else

inline qf32 QuaternionIdentity()
{
	return qf32(0.0f, 0.0f, 0.0f, 1.0f);
}

inline qf32 QuaternionMul(crqf32 q1, crqf32 q2)
{
	// Quaternion Multiplication
	// [q1s * q2v + q2s * q1v + q1 x q2] [q1s * q2s - q1.q2]
	// Expanding the equation
	// x => q1.w * q2.x + q2.w * q1.x + q1.y * q2.z - q1.z * q2.y
	// y => q1.w * q2.y + q2.w * q1.y + q1.z * q2.x - q1.x * q2.z
	// z => q1.w * q2.z + q2.w * q1.z + q1.x * q2.y - q1.y * q2.x
	// w => q1.w * q2.w - q2.x * q1.x - q1.y * q2.y - q1.z * q2.z

	return VectorSet(
		q1.w * q2.x + q2.w * q1.x + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q2.w * q1.y + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q2.w * q1.z + q1.x * q2.y - q1.y * q2.x,
		q1.w * q2.w - q2.x * q1.x - q1.y * q2.y - q1.z * q2.z
	);
}

inline qf32 QuaternionMulKeenan(crqf32 q1, crqf32 q2)
{
	// Quaternion Multiplication
	// [q1s * q2v + q2s * q1v + q2 x q1] [q1s * q2s - q1.q2]
	// Expanding the equation
	// x => q1.w * q2.x + q2.w * q1.x + q2.y * q1.z - q2.z * q1.y
	// y => q1.w * q2.y + q2.w * q1.y + q2.z * q1.x - q2.x * q1.z
	// z => q1.w * q2.z + q2.w * q1.z + q2.x * q1.y - q2.y * q1.x
	// w => q1.w * q2.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z

	return VectorSet(
		q1.w * q2.x + q2.w * q1.x + q2.y * q1.z - q2.z * q1.y,
		q1.w * q2.y + q2.w * q1.y + q2.z * q1.x - q2.x * q1.z,
		q1.w * q2.z + q2.w * q1.z + q2.x * q1.y - q2.y * q1.x,
		q1.w * q2.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z
	);
}

inline qf32 QuaternionConjugate(crqf32 q)
{
	return VectorSet(-q.x, -q.y, -q.z, q.w);
}

inline qf32 QuaternionInverse(crqf32 q)
{
	qf32 conj = QuaternionConjugate(q);
	f32 oneOverLengthSqr = 1.0f / (QuaternionLengthSqr(q));

	return qf32(
		conj.x * oneOverLengthSqr,
		conj.y * oneOverLengthSqr,
		conj.z * oneOverLengthSqr,
		conj.w * oneOverLengthSqr
	);
}

inline qf32 QuaternionNormalize(crqf32 q)
{
	f32 oneOverLengthSqr = QuaternionLengthSqr(q);

	return qf32(
		q.x * oneOverLengthSqr,
		q.y * oneOverLengthSqr,
		q.z * oneOverLengthSqr,
		q.w * oneOverLengthSqr
	);
}

inline f32 QuaternionDot(crqf32 q1, crqf32 q2)
{
	// _mm_cvtss_f32 gets the value of the lowest f32, in this case, X
	// _mm_store_ss could also be used, but I think it might be slower
	return QuaternionDotV(q1, q2).x;
}

inline qf32 QuaternionDotV(crqf32 q1, crqf32 q2)
{
	return VectorReplicate(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
}

inline f32 QuaternionLengthSqr(crqf32 q)
{
	return QuaternionLengthSqrV(q).x;
}

inline qf32 QuaternionLengthSqrV(crqf32 q)
{
	return QuaternionDotV(q, q);
}

inline f32 QuaternionLength(crqf32 v)
{
	return QuaternionLengthV(v).x;
}

inline qf32 QuaternionLengthV(crqf32 v)
{
	f32 oneOverLengthSqr = 1.0f / QuaternionLengthSqr(v);
	return qf32(v.x * oneOverLengthSqr, v.y * oneOverLengthSqr, v.z * oneOverLengthSqr, v.w * oneOverLengthSqr);
}

inline qf32 QuaternionRotationAxis(const vf32& v, f32 angle)
{
	f32 halfAngle = angle * 0.5f;
	f32 sinCos[] = { sinf(halfAngle), cosf(halfAngle) };

	return vf32(v.x * sinCos[0], v.y * sinCos[0], v.z * sinCos[0], sinCos[1]);
}

inline vf32 QuaternionQVQC(crqf32 q, const vf32& v)
{
	f32 w2MinusQDotQ = q.w * q.w - VectorDot(q, q);
	f32 twoTimesQDotV = 2.0f * VectorDot(q, v);
	f32 wTimesTwo = 2.0f * q.w;
	vf32 qCrossV = VectorCross(q, v);

	return vf32(w2MinusQDotQ * v.x + twoTimesQDotV * q.x + wTimesTwo * qCrossV.x,
		w2MinusQDotQ * v.y + twoTimesQDotV * q.y + wTimesTwo * qCrossV.y,
		w2MinusQDotQ * v.z + twoTimesQDotV * q.z + wTimesTwo * qCrossV.z);
}

inline vf32 QuaternionQCVQ(crqf32 q, const vf32& v)
{
	f32 w2MinusQDotQ = q.w * q.w - VectorDot(q, q);
	f32 twoTimesQDotV = 2.0f * VectorDot(q, v);
	f32 wTimesTwo = 2.0f * q.w;
	vf32 vCrossQ = VectorCross(v, q);

	return vf32(w2MinusQDotQ * v.x + twoTimesQDotV * q.x + wTimesTwo * vCrossQ.x,
		w2MinusQDotQ * v.y + twoTimesQDotV * q.y + wTimesTwo * vCrossQ.y,
		w2MinusQDotQ * v.z + twoTimesQDotV * q.z + wTimesTwo * vCrossQ.z);
}

inline vf32 QuaternionQVQCKeenan(crqf32 q, const vf32& v)
{
	return QuaternionQCVQ(q, v);
}

inline vf32 QuaternionQCVQKeenan(crqf32 q, const vf32& v)
{
	return QuaternionQVQC(q, v);
}

inline mf32 QuaternionToMatrix(crqf32 q)
{
	f32 xy = q.x * q.y;
	f32 xz = q.x * q.z;
	f32 xw = q.x * q.w;

	f32 yz = q.x * q.z;
	f32 yw = q.x * q.w;

	f32 zw = q.x * q.w;

	f32 xx = q.x * q.x;
	f32 yy = q.y * q.y;
	f32 zz = q.z * q.z;

	mf32 r;
	r.r[0].x = 1.0f - 2.0f * (yy + zz);
	r.r[0].y = 2.0f * (xy + zw);
	r.r[0].z = 2.0f * (xz - yw);
	r.r[0].w = 0.0f;

	r.r[1].x = 2.0f * (xy - zw);
	r.r[1].y = 1.0f - 2.0f * (xx + zz);
	r.r[1].z = 2.0f * (yz + xw);
	r.r[1].w = 0.0f;

	r.r[2].x = 2.0f * (xz + yw);
	r.r[2].y = 2.0f * (yz - xw);
	r.r[2].z = 1.0f - 2.0f * (xx + yy);
	r.r[2].w = 0.0f;

	r.r[3].x = 0.0f;
	r.r[3].y = 0.0f;
	r.r[3].z = 0.0f;
	r.r[3].w = 1.0f;

	return r;
}

inline qf32 QuaternionFromMatrix(const mf32& m)
{
	f32 trace = m.r[0].x + m.r[1].y + m.r[2].z;

	qf32 q;
	if (trace > 0.0f)
	{
		f32 s = sqrtf(trace + 1.0f) * 2.0f; // S=4*qw 
		q.w = 0.25f * s;
		q.x = (m.r[2].y - m.r[1].z) / s;
		q.y = (m.r[0].z - m.r[2].x) / s;
		q.z = (m.r[1].x - m.r[0].y) / s;
	}
	else if ((m.r[0].x > m.r[1].y) && (m.r[0].x > m.r[2].z))
	{
		f32 s = sqrtf(1.0f + m.r[0].x - m.r[1].y - m.r[2].z) * 2.0f; // S=4*qx 
		q.w = (m.r[2].y - m.r[1].z) / s;
		q.x = 0.25f * s;
		q.y = (m.r[0].y + m.r[1].x) / s;
		q.z = (m.r[0].z + m.r[2].x) / s;
	}
	else if (m.r[1].y > m.r[2].z)
	{
		f32 s = sqrtf(1.0f + m.r[1].y - m.r[0].x - m.r[2].z) * 2.0f; // S=4*qy
		q.w = (m.r[0].z - m.r[2].x) / s;
		q.x = (m.r[0].y + m.r[1].x) / s;
		q.y = 0.25f * s;
		q.z = (m.r[1].z + m.r[2].y) / s;
	}
	else
	{
		f32 s = sqrtf(1.0f + m.r[2].z - m.r[0].x - m.r[1].y) * 2.0f; // S=4*qz
		q.w = (m.r[1].x - m.r[0].y) / s;
		q.x = (m.r[0].z + m.r[2].x) / s;
		q.y = (m.r[1].z + m.r[2].y) / s;
		q.z = 0.25f * s;
	}

	return QuaternionNormalize(q);
}

inline qf32 QuaternionSlerp(crqf32 q1, crqf32 q2, f32 t, f32 epsilon)
{
	f32 cosOmega = QuaternionDot(q1, q2);
	qf32 qt(q2);
	if (cosOmega < 0.0f)
	{
		cosOmega = -cosOmega;
		qt = QuaternionConjugate(qt);
	}

	f32 omega = acosf(cosOmega);

	f32 mult1 = 1.0f - t;
	f32 mult2 = t;
	if (omega > epsilon)
	{
		f32 oneOverSinOmega = 1.0f / sinf(omega);
		mult1 = sinf(mult1 * omega) * oneOverSinOmega;
		mult2 = sinf(mult2 * omega) * oneOverSinOmega;
	}

	qf32 r = q1 * mult1 + qt * mult2;

	return QuaternionNormalize(r);
}

inline f32 QuaternionAngle(crqf32 q)
{
	return 2.0f * acosf(q.w);
}

#endif