#pragma once


#include "BDeV/Core/Math/SIMD/BvFastVec_SIMD.inl"


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_SIMD)


// ==================================
// Matrix operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Access
// ======================

mf32 MatrixSet(vf32 v0, vf32 v1, vf32 v2, vf32 v3);

// ======================
// Basic Operations
// ======================

mf32 MatrixAdd(const mf32 & m1, const mf32 & m2);
mf32 MatrixAdd(const mf32 & m, f32 s);

mf32 MatrixSub(const mf32 & m1, const mf32 & m2);
mf32 MatrixSub(const mf32 & m, f32 s);

mf32 MatrixMul(const mf32 & m1, const mf32 & m2);
mf32 MatrixMul(const mf32 & m, f32 s);

mf32 MatrixDiv(const mf32 & m, f32 s);

mf32 MatrixIdentity();

mf32 MatrixTranspose(const mf32 & m);

vf32 MatrixDeterminantV(const mf32 & m);
f32 MatrixDeterminant(const mf32 & m);

mf32 MatrixInverse(const mf32 & m);

// ======================
// Operators
// ======================

mf32 operator + (const mf32 & m);
mf32 operator - (const mf32 & m);

mf32 operator + (const mf32 & m1, const mf32 & m2);
mf32 & operator += (mf32 & m1, const mf32 & m2);

mf32 operator - (const mf32 & m1, const mf32 & m2);
mf32 & operator -= (mf32 & m1, const mf32 & m2);

mf32 operator * (const mf32 & m1, const mf32 & m2);
mf32 & operator *= (mf32 & m1, const mf32 & m2);

mf32 operator + (const mf32 & m, f32 s);
mf32 & operator += (mf32 & m, f32 s);

mf32 operator - (const mf32 & m, f32 s);
mf32 & operator -= (mf32 & m, f32 s);

mf32 operator * (const mf32 & m, f32 s);
mf32 & operator *= (mf32 & m, f32 s);

mf32 operator / (const mf32 & m, f32 s);
mf32 & operator /= (mf32 & m, f32 s);

// ======================
// Specialized Operations
// ======================

mf32 MatrixScaling(f32 sX, f32 sY, f32 sZ);

mf32 MatrixTranslation(f32 tX, f32 tY, f32 tZ);

mf32 MatrixRotationX(f32 angle);
mf32 MatrixRotationY(f32 angle);
mf32 MatrixRotationZ(f32 angle);
mf32 MatrixRotationAxis(vf32 axis, f32 angle);
		 
mf32 MatrixLookAt(vf32 eyePos, vf32 dirVec, vf32 upVec);
mf32 MatrixLookAtLH(vf32 eyePos, vf32 lookPos, vf32 upVec);
mf32 MatrixLookAtRH(vf32 eyePos, vf32 lookPos, vf32 upVec);
		 
mf32 MatrixPerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 MatrixPerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
		 
mf32 MatrixPerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 MatrixPerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);

mf32 MatrixPerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);
mf32 MatrixPerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);

mf32 MatrixOrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ);

mf32 MatrixOrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ);

mf32 MatrixOrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ);

mf32 MatrixOrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ);

mf32 MatrixOrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ);

mf32 MatrixOrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ);

// ======================
// Definitions
// ======================

// ======================
// Access
// ======================

inline mf32 MatrixSet(vf32 v0, vf32 v1, vf32 v2, vf32 v3)
{
	mf32 r;
	r.r[0] = v0;
	r.r[1] = v1;
	r.r[2] = v2;
	r.r[3] = v3;

	return r;
}

// ======================
// Basic Operations
// ======================

inline mf32 MatrixAdd(const mf32 & m1, const mf32 & m2)
{
	mf32 m;
	m.r[0] = _mm_add_ps(m1.r[0], m2.r[0]);
	m.r[1] = _mm_add_ps(m1.r[1], m2.r[1]);
	m.r[2] = _mm_add_ps(m1.r[2], m2.r[2]);
	m.r[3] = _mm_add_ps(m1.r[3], m2.r[3]);

	return m;
}

inline mf32 MatrixAdd(const mf32 & m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_add_ps(m.r[0], v);
	r.r[1] = _mm_add_ps(m.r[1], v);
	r.r[2] = _mm_add_ps(m.r[2], v);
	r.r[3] = _mm_add_ps(m.r[3], v);

	return r;
}

inline mf32 MatrixSub(const mf32 & m1, const mf32 & m2)
{
	mf32 m;
	m.r[0] = _mm_sub_ps(m1.r[0], m2.r[0]);
	m.r[1] = _mm_sub_ps(m1.r[1], m2.r[1]);
	m.r[2] = _mm_sub_ps(m1.r[2], m2.r[2]);
	m.r[3] = _mm_sub_ps(m1.r[3], m2.r[3]);

	return m;
}

inline mf32 MatrixSub(const mf32 & m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_sub_ps(m.r[0], v);
	r.r[1] = _mm_sub_ps(m.r[1], v);
	r.r[2] = _mm_sub_ps(m.r[2], v);
	r.r[3] = _mm_sub_ps(m.r[3], v);

	return r;
}

inline mf32 MatrixMul(const mf32 & m1, const mf32 & m2)
{
	mf32 m;

	// Row 0
	vf32 tmp0 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	vf32 tmp1 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	vf32 tmp2 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	vf32 tmp3 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[0] = _mm_add_ps(tmp0, tmp1);
	m.r[0] = _mm_add_ps(m.r[0], _mm_add_ps(tmp2, tmp3));

	// Row 1
	tmp0 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	tmp1 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	tmp2 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	tmp3 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[1] = _mm_add_ps(tmp0, tmp1);
	m.r[1] = _mm_add_ps(m.r[1], _mm_add_ps(tmp2, tmp3));

	// Row 2
	tmp0 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	tmp1 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	tmp2 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	tmp3 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[2] = _mm_add_ps(tmp0, tmp1);
	m.r[2] = _mm_add_ps(m.r[2], _mm_add_ps(tmp2, tmp3));

	// Row 3
	tmp0 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	tmp1 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	tmp2 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	tmp3 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[3] = _mm_add_ps(tmp0, tmp1);
	m.r[3] = _mm_add_ps(m.r[3], _mm_add_ps(tmp2, tmp3));

	return m;
}

inline mf32 MatrixMul(const mf32 & m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_mul_ps(m.r[0], v);
	r.r[1] = _mm_mul_ps(m.r[1], v);
	r.r[2] = _mm_mul_ps(m.r[2], v);
	r.r[3] = _mm_mul_ps(m.r[3], v);

	return r;
}

inline mf32 MatrixDiv(const mf32 & m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_div_ps(m.r[0], v);
	r.r[1] = _mm_div_ps(m.r[1], v);
	r.r[2] = _mm_div_ps(m.r[2], v);
	r.r[3] = _mm_div_ps(m.r[3], v);

	return r;
}

inline mf32 MatrixIdentity()
{
	vf32 id = _mm_set_ss(1.0f);

	mf32 m;
	m.r[0] = id;
	m.r[1] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(1, 1, 0, 1));
	m.r[2] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(1, 0, 1, 1));
	m.r[3] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(0, 1, 1, 1));

	return m;
}

inline mf32 MatrixTranspose(const mf32 & m)
{
	// top left 2x2 matrix
	vf32 upperBlock = _mm_shuffle_ps(m.r[0], m.r[1], _MM_SHUFFLE(1, 0, 1, 0));
	// bottom left 2x2 matrix
	vf32 lowerBlock = _mm_shuffle_ps(m.r[2], m.r[3], _MM_SHUFFLE(1, 0, 1, 0));

	mf32 mT;
	mT.r[0] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[1] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	// top right 2x2 matrix
	upperBlock = _mm_shuffle_ps(m.r[0], m.r[1], _MM_SHUFFLE(3, 2, 3, 2));
	// bottom right 2x2 matrix
	lowerBlock = _mm_shuffle_ps(m.r[2], m.r[3], _MM_SHUFFLE(3, 2, 3, 2));

	mT.r[2] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[3] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	return mT;
}

inline vf32 MatrixDeterminantV(const mf32 & m)
{
	// Calculate the first 12 2x2 determinant multiplications, to avoid repeating them later
	vf32 cf1 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(1, 3, 2, 3)));
	vf32 cf2 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(0, 3, 1, 2)));
	vf32 cf3 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the first row
	vf32 tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	vf32 tmp2 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	vf32 row0 = _mm_mul_ps(tmp1, tmp2);

	// Compute the second multiplication and first subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the third multiplication and first addition for the first row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(0, 1, 2, 3));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(3, 1, 2, 0));
	tmp2 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and second subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fifth multiplication and second addition for the first row
	tmp1 = _mm_shuffle_ps(cf2, cf2, _MM_SHUFFLE(1, 3, 2, 0));
	tmp2 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and third subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the determinant
	vf32 det = _mm_mul_ps(row0, m.r[0]);

	// Sum all values and replicate them through all elements
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)));
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)));

	return det;
}

inline f32 MatrixDeterminant(const mf32 & m)
{
	return _mm_cvtss_f32(MatrixDeterminantV(m));
}

inline mf32 MatrixInverse(const mf32 & m)
{
	mf32 mTmp = MatrixTranspose(m);

	// Calculate the first 12 2x2 determinant multiplications, to avoid repeating them later
	vf32 cf1 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(1, 3, 2, 3)));
	vf32 cf2 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 3, 1, 2)));
	vf32 cf3 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the first row
	vf32 tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	vf32 tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	vf32 row0 = _mm_mul_ps(tmp1, tmp2);

	// Compute the first multiplication for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(2, 3, 0, 1));
	vf32 row1 = _mm_mul_ps(tmp1, tmp2);

	// Compute the second multiplication and first subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the second multiplication and first subtraction for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(2, 3, 0, 1));

	row1 = _mm_sub_ps(_mm_mul_ps(tmp1, tmp2), row1);

	// Compute the third multiplication and first addition for the first row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(0, 1, 2, 3));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(3, 1, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the third multiplication and second subtraction for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(1, 0, 3, 2));

	row1 = _mm_sub_ps(row1, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and second subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and first addition for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(1, 0, 3, 2));

	row1 = _mm_add_ps(_mm_mul_ps(tmp1, tmp2), row1);

	// Compute the fifth multiplication and second addition for the first row
	tmp1 = _mm_shuffle_ps(cf2, cf2, _MM_SHUFFLE(1, 3, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fifth multiplication and third subtraction for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(0, 1, 2, 3));

	row1 = _mm_sub_ps(row1, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and third subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 2, 3));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and second addition for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(0, 1, 2, 3));

	row1 = _mm_add_ps(row1, _mm_mul_ps(tmp1, tmp2));

	// Calculate the second 12 2x2 determinant multiplications, avoid repetitions
	cf1 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(1, 3, 2, 3)));
	cf2 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 3, 1, 2)));
	cf3 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the third row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(2, 3, 0, 1));

	vf32 row2 = _mm_mul_ps(tmp1, tmp2);

	// Compute the first multiplication for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(2, 3, 0, 1));
	vf32 row3 = _mm_mul_ps(tmp1, tmp2);

	// Compute the second multiplication and first subtraction for the third row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(2, 3, 0, 1));

	row2 = _mm_sub_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the second multiplication and first subtraction for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(2, 3, 0, 1));

	row3 = _mm_sub_ps(_mm_mul_ps(tmp1, tmp2), row3);

	// Compute the third multiplication and first addition for the third row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(0, 1, 2, 3));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(3, 1, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(1, 0, 3, 2));

	row2 = _mm_add_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the third multiplication and second subtraction for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(1, 0, 3, 2));

	row3 = _mm_sub_ps(row3, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and second subtraction for the third row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(1, 0, 3, 2));

	row2 = _mm_sub_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and first addition for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(1, 0, 3, 2));

	row3 = _mm_add_ps(_mm_mul_ps(tmp1, tmp2), row3);

	// Compute the fifth multiplication and second addition for the third row
	tmp1 = _mm_shuffle_ps(cf2, cf2, _MM_SHUFFLE(1, 3, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 1, 2, 3));

	row2 = _mm_add_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the fifth multiplication and third subtraction for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(0, 1, 2, 3));

	row3 = _mm_sub_ps(row3, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and third subtraction for the third row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 2, 3));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 1, 2, 3));

	row2 = _mm_sub_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and second addition for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(0, 1, 2, 3));

	row3 = _mm_add_ps(row3, _mm_mul_ps(tmp1, tmp2));

	// Compute the determinant
	vf32 det = _mm_mul_ps(row0, mTmp.r[0]);

	// Sum all values and replicate them through all elements
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)));
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)));

	// Get 1/Det
	det = _mm_rcp_ps(det); // _mm_rcp_ps is faster but less precise
							//det = _mm_div_ps(_mm_set1_ps(1.0f), det); // _mm_div_ps is not as fast as _mm_rcp_ps but is more precise

							// Multiply every element by 1/Det
	mTmp.r[0] = _mm_mul_ps(row0, det);
	mTmp.r[1] = _mm_mul_ps(row1, det);
	mTmp.r[2] = _mm_mul_ps(row2, det);
	mTmp.r[3] = _mm_mul_ps(row3, det);

	return mTmp;
}

// ======================
// Operators
// ======================

inline mf32 operator + (const mf32 & m)
{
	return m;
}

inline mf32 operator - (const mf32 & m)
{
	mf32 r;
	r.r[0] = VectorNegate(m.r[0]);
	r.r[1] = VectorNegate(m.r[1]);
	r.r[2] = VectorNegate(m.r[2]);
	r.r[3] = VectorNegate(m.r[3]);

	return r;
}

inline mf32 operator + (const mf32 & m1, const mf32 & m2)
{
	return MatrixAdd(m1, m2);
}

inline mf32 & operator += (mf32 & m1, const mf32 & m2)
{
	m1 = MatrixAdd(m1, m2);
	return m1;
}

inline mf32 operator - (const mf32 & m1, const mf32 & m2)
{
	return MatrixSub(m1, m2);
}

inline mf32 & operator -= (mf32 & m1, const mf32 & m2)
{
	m1 = MatrixSub(m1, m2);
	return m1;
}

inline mf32 operator * (const mf32 & m1, const mf32 & m2)
{
	return MatrixMul(m1, m2);
}

inline mf32 & operator *= (mf32 & m1, const mf32 & m2)
{
	m1 = MatrixMul(m1, m2);
	return m1;
}

inline mf32 operator+(const mf32 & m, f32 s)
{
	return MatrixAdd(m, s);
}

inline mf32 & operator+=(mf32 & m, f32 s)
{
	m = MatrixAdd(m, s);
	return m;
}

inline mf32 operator-(const mf32 & m, f32 s)
{
	return MatrixSub(m, s);
}

inline mf32 & operator-=(mf32 & m, f32 s)
{
	m = MatrixSub(m, s);
	return m;
}

inline mf32 operator * (const mf32 & m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_mul_ps(m.r[0], v);
	r.r[1] = _mm_mul_ps(m.r[1], v);
	r.r[2] = _mm_mul_ps(m.r[2], v);
	r.r[3] = _mm_mul_ps(m.r[3], v);

	return r;
}

inline mf32 & operator *= (mf32 & m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	m.r[0] = _mm_mul_ps(m.r[0], v);
	m.r[1] = _mm_mul_ps(m.r[1], v);
	m.r[2] = _mm_mul_ps(m.r[2], v);
	m.r[3] = _mm_mul_ps(m.r[3], v);

	return m;
}

inline mf32 operator / (const mf32 & m, f32 s)
{
	return MatrixDiv(m, s);
}

inline mf32 & operator /= (mf32 & m, f32 s)
{
	m = MatrixDiv(m, s);
	return m;
}

// ======================
// Specialized Operations
// ======================

inline mf32 MatrixScaling(f32 sX, f32 sY, f32 sZ)
{
	mf32 r;
	r.r[0] = VectorSet(sX, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, sY, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, sZ, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixTranslation(f32 tX, f32 tY, f32 tZ)
{
	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(tX, tY, tZ, 1.0f);

	return r;
}

inline mf32 MatrixRotationX(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };
	vf32 vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	vf32 vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = _mm_shuffle_ps(vCosSin, vCosSin, _MM_SHUFFLE(3, 1, 0, 2));
	r.r[2] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 0, 1, 2));
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationY(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };
	vf32 vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	vf32 vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 1, 2, 0));
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = _mm_shuffle_ps(vCosSin, vCosSin, _MM_SHUFFLE(3, 0, 2, 1));
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationZ(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };
	vf32 vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	vf32 vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	mf32 r;
	r.r[0] = vCosSin;
	r.r[1] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 2, 0, 1));
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationAxis(vf32 axis, f32 angle)
{
	f32 sin = sinf(angle);
	f32 cos = cosf(angle);

	vf32 vSin = _mm_set1_ps(sin);
	vf32 vCos = _mm_set_ps(0.0f, cos, cos, cos);
	vf32 vOneMinusCos = _mm_set1_ps(1.0f - cos);

	// x * (1 - cos), y * (1 - cos), z * (1 - cos)
	vf32 d = _mm_mul_ps(axis, vOneMinusCos);

	vf32 t1 = d;

	// x^2 * (1 - cos), y^2 * (1 - cos), z^2 * (1 - cos)
	d = _mm_mul_ps(d, axis);
	// cos + x^2 * (1 - cos), cos + y^2 * (1 - cos), cos + z^2 * (1 - cos)
	d = _mm_add_ps(d, vCos);

	// x * (sin), y * (sin), z * (sin)
	vf32 t0 = _mm_mul_ps(axis, vSin);

	// z * (1 - cos), x * (1 - cos), y * (1 - cos)
	t1 = _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 1, 0, 2));
	// y * z * (1 - cos), z * x * (1 - cos), x * y * (1 - cos)
	t1 = _mm_mul_ps(t1, _mm_shuffle_ps(axis, axis, _MM_SHUFFLE(3, 0, 2, 1)));
	// y * z * (1 - cos) - x * (sin), z * x * (1 - cos) - y * (sin), x * y * (1 - cos) - z * (sin)
	vf32 t2 = _mm_sub_ps(t1, t0);
	// y * z * (1 - cos) + x * (sin), z * x * (1 - cos) + y * (sin), x * y * (1 - cos) + z * (sin)
	t1 = _mm_add_ps(t1, t0);

	// y * z * (1 - cos) + x * (sin), x * y * (1 - cos) + z * (sin), z * x * (1 - cos) - y * (sin), x * y * (1 - cos) - z * (sin)
	vf32 g0 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(2, 1, 2, 0));
	// y * z * (1 - cos) + x * (sin), 0, y * z * (1 - cos) - x * (sin), 0
	vf32 g1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(3, 0, 3, 1));

	mf32 r;
	r.r[0] = _mm_shuffle_ps(d, g0, _MM_SHUFFLE(2, 1, 3, 0));
	r.r[0] = _mm_shuffle_ps(r.r[0], r.r[0], _MM_SHUFFLE(1, 3, 2, 0));

	r.r[1] = _mm_shuffle_ps(d, g0, _MM_SHUFFLE(0, 3, 3, 1));
	r.r[1] = _mm_shuffle_ps(r.r[1], r.r[1], _MM_SHUFFLE(1, 3, 0, 2));

	r.r[2] = _mm_shuffle_ps(d, g1, _MM_SHUFFLE(2, 0, 3, 2));
	r.r[2] = _mm_shuffle_ps(r.r[2], r.r[2], _MM_SHUFFLE(1, 0, 3, 2));

	r.r[3] = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);

	return r;
}

inline mf32 MatrixLookAt(vf32 eyePos, vf32 dirVec, vf32 upVec)
{
	vf32 z = VectorNormalize(dirVec);

	vf32 x = VectorCross(upVec, z);
	x = VectorNormalize(x);

	vf32 y = VectorCross(z, x);

	mf32 r;
	r.r[0] = x;
	r.r[1] = y;
	r.r[2] = z;
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	r = MatrixTranspose(r);
	r.r[3] = VectorSet(-VectorDot(x, eyePos), -VectorDot(y, eyePos), -VectorDot(z, eyePos), 1.0f);

	return r;
}

inline mf32 MatrixLookAtLH(vf32 eyePos, vf32 lookPos, vf32 upVec)
{
	vf32 dir = VectorSub(lookPos, eyePos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline mf32 MatrixLookAtRH(vf32 eyePos, vf32 lookPos, vf32 upVec)
{
	vf32 dir = VectorSub(eyePos, lookPos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline mf32 MatrixPerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	vf32 v0 = _mm_set_ps(0.0f, -nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline mf32 MatrixPerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (nearZ - farZ);

	vf32 v0 = _mm_set_ps(0.0f, nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline mf32 MatrixPerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = 1.0f / (farZ - nearZ);

	vf32 v0 = _mm_set_ps(0.0f, -2.0f * farZ * nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, (farZ + nearZ) * b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline mf32 MatrixPerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = 1.0f / (nearZ - farZ);

	vf32 v0 = _mm_set_ps(0.0f, 2.0f * farZ * nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, (farZ + nearZ) * b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline mf32 MatrixPerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	vf32 v0 = _mm_set_ps(0.0f, -nearZ * b, -a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline mf32 MatrixPerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (nearZ - farZ);

	vf32 v0 = _mm_set_ps(0.0f, nearZ * b, -a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverFMinusN, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-oneOverFMinusN * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverNMinusF, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		oneOverNMinusF * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, -2.0f * oneOverNMinusF, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		(nearZ + farZ) * oneOverNMinusF, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, -2.0f * oneOverFMinusN, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-(nearZ + farZ) * oneOverFMinusN, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (bottom - top);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverFMinusN, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-oneOverFMinusN * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (bottom - top);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverNMinusF, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		oneOverNMinusF * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}


#endif