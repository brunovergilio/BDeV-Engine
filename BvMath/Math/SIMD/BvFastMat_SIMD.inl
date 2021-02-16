#pragma once


#include "BvFastVec_SIMD.inl"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_SIMD)


// ==================================
// Matrix operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Access
// ======================

BvFastMat MatrixSet(BvFastVec v0, BvFastVec v1, BvFastVec v2, BvFastVec v3);

// ======================
// Basic Operations
// ======================

BvFastMat MatrixAdd(const BvFastMat & m1, const BvFastMat & m2);
BvFastMat MatrixAdd(const BvFastMat & m, const float s);

BvFastMat MatrixSub(const BvFastMat & m1, const BvFastMat & m2);
BvFastMat MatrixSub(const BvFastMat & m, const float s);

BvFastMat MatrixMul(const BvFastMat & m1, const BvFastMat & m2);
BvFastMat MatrixMul(const BvFastMat & m, const float s);

BvFastMat MatrixDiv(const BvFastMat & m, const float s);

BvFastMat MatrixIdentity();

BvFastMat MatrixTranspose(const BvFastMat & m);

BvFastVec MatrixDeterminantV(const BvFastMat & m);
float MatrixDeterminant(const BvFastMat & m);

BvFastMat MatrixInverse(const BvFastMat & m);

// ======================
// Operators
// ======================

BvFastMat operator + (const BvFastMat & m);
BvFastMat operator - (const BvFastMat & m);

BvFastMat operator + (const BvFastMat & m1, const BvFastMat & m2);
BvFastMat & operator += (BvFastMat & m1, const BvFastMat & m2);

BvFastMat operator - (const BvFastMat & m1, const BvFastMat & m2);
BvFastMat & operator -= (BvFastMat & m1, const BvFastMat & m2);

BvFastMat operator * (const BvFastMat & m1, const BvFastMat & m2);
BvFastMat & operator *= (BvFastMat & m1, const BvFastMat & m2);

BvFastMat operator + (const BvFastMat & m, const float s);
BvFastMat & operator += (BvFastMat & m, const float s);

BvFastMat operator - (const BvFastMat & m, const float s);
BvFastMat & operator -= (BvFastMat & m, const float s);

BvFastMat operator * (const BvFastMat & m, const float s);
BvFastMat & operator *= (BvFastMat & m, const float s);

BvFastMat operator / (const BvFastMat & m, const float s);
BvFastMat & operator /= (BvFastMat & m, const float s);

// ======================
// Specialized Operations
// ======================

BvFastMat MatrixScaling(const float sX, const float sY, const float sZ);

BvFastMat MatrixTranslation(const float tX, const float tY, const float tZ);

BvFastMat MatrixRotationX(const float angle);
BvFastMat MatrixRotationY(const float angle);
BvFastMat MatrixRotationZ(const float angle);
BvFastMat MatrixRotationAxis(BvFastVec axis, const float angle);
		 
BvFastMat MatrixLookAt(BvFastVec eyePos, BvFastVec dirVec, BvFastVec upVec);
BvFastMat MatrixLookAtLH(BvFastVec eyePos, BvFastVec lookPos, BvFastVec upVec);
BvFastMat MatrixLookAtRH(BvFastVec eyePos, BvFastVec lookPos, BvFastVec upVec);
		 
BvFastMat MatrixPerspectiveLH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY);
BvFastMat MatrixPerspectiveRH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY);
		 
BvFastMat MatrixPerspectiveLH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY);
BvFastMat MatrixPerspectiveRH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY);

BvFastMat MatrixPerspectiveLH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY);
BvFastMat MatrixPerspectiveRH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY);

BvFastMat MatrixOrthographicOffCenterLH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

BvFastMat MatrixOrthographicOffCenterRH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

BvFastMat MatrixOrthographicOffCenterLH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

BvFastMat MatrixOrthographicOffCenterRH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

BvFastMat MatrixOrthographicOffCenterLH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

BvFastMat MatrixOrthographicOffCenterRH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

// ======================
// Definitions
// ======================

// ======================
// Access
// ======================

inline BvFastMat MatrixSet(BvFastVec v0, BvFastVec v1, BvFastVec v2, BvFastVec v3)
{
	BvFastMat r;
	r.r[0] = v0;
	r.r[1] = v1;
	r.r[2] = v2;
	r.r[3] = v3;

	return r;
}

// ======================
// Basic Operations
// ======================

inline BvFastMat MatrixAdd(const BvFastMat & m1, const BvFastMat & m2)
{
	BvFastMat m;
	m.r[0] = _mm_add_ps(m1.r[0], m2.r[0]);
	m.r[1] = _mm_add_ps(m1.r[1], m2.r[1]);
	m.r[2] = _mm_add_ps(m1.r[2], m2.r[2]);
	m.r[3] = _mm_add_ps(m1.r[3], m2.r[3]);

	return m;
}

inline BvFastMat MatrixAdd(const BvFastMat & m, const float s)
{
	BvFastVec v = _mm_set1_ps(s);
	BvFastMat r;
	r.r[0] = _mm_add_ps(m.r[0], v);
	r.r[1] = _mm_add_ps(m.r[1], v);
	r.r[2] = _mm_add_ps(m.r[2], v);
	r.r[3] = _mm_add_ps(m.r[3], v);

	return r;
}

inline BvFastMat MatrixSub(const BvFastMat & m1, const BvFastMat & m2)
{
	BvFastMat m;
	m.r[0] = _mm_sub_ps(m1.r[0], m2.r[0]);
	m.r[1] = _mm_sub_ps(m1.r[1], m2.r[1]);
	m.r[2] = _mm_sub_ps(m1.r[2], m2.r[2]);
	m.r[3] = _mm_sub_ps(m1.r[3], m2.r[3]);

	return m;
}

inline BvFastMat MatrixSub(const BvFastMat & m, const float s)
{
	BvFastVec v = _mm_set1_ps(s);
	BvFastMat r;
	r.r[0] = _mm_sub_ps(m.r[0], v);
	r.r[1] = _mm_sub_ps(m.r[1], v);
	r.r[2] = _mm_sub_ps(m.r[2], v);
	r.r[3] = _mm_sub_ps(m.r[3], v);

	return r;
}

inline BvFastMat MatrixMul(const BvFastMat & m1, const BvFastMat & m2)
{
	BvFastMat m;

	// Row 0
	BvFastVec tmp0 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	BvFastVec tmp1 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	BvFastVec tmp2 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	BvFastVec tmp3 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(3, 3, 3, 3));
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

inline BvFastMat MatrixMul(const BvFastMat & m, const float s)
{
	BvFastVec v = _mm_set1_ps(s);
	BvFastMat r;
	r.r[0] = _mm_mul_ps(m.r[0], v);
	r.r[1] = _mm_mul_ps(m.r[1], v);
	r.r[2] = _mm_mul_ps(m.r[2], v);
	r.r[3] = _mm_mul_ps(m.r[3], v);

	return r;
}

inline BvFastMat MatrixDiv(const BvFastMat & m, const float s)
{
	BvFastVec v = _mm_set1_ps(s);
	BvFastMat r;
	r.r[0] = _mm_div_ps(m.r[0], v);
	r.r[1] = _mm_div_ps(m.r[1], v);
	r.r[2] = _mm_div_ps(m.r[2], v);
	r.r[3] = _mm_div_ps(m.r[3], v);

	return r;
}

inline BvFastMat MatrixIdentity()
{
	BvFastVec id = _mm_set_ss(1.0f);

	BvFastMat m;
	m.r[0] = id;
	m.r[1] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(1, 1, 0, 1));
	m.r[2] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(1, 0, 1, 1));
	m.r[3] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(0, 1, 1, 1));

	return m;
}

inline BvFastMat MatrixTranspose(const BvFastMat & m)
{
	// top left 2x2 matrix
	BvFastVec upperBlock = _mm_shuffle_ps(m.r[0], m.r[1], _MM_SHUFFLE(1, 0, 1, 0));
	// bottom left 2x2 matrix
	BvFastVec lowerBlock = _mm_shuffle_ps(m.r[2], m.r[3], _MM_SHUFFLE(1, 0, 1, 0));

	BvFastMat mT;
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

inline BvFastVec MatrixDeterminantV(const BvFastMat & m)
{
	// Calculate the first 12 2x2 determinant multiplications, to avoid repeating them later
	BvFastVec cf1 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(1, 3, 2, 3)));
	BvFastVec cf2 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(0, 3, 1, 2)));
	BvFastVec cf3 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the first row
	BvFastVec tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	BvFastVec tmp2 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	BvFastVec row0 = _mm_mul_ps(tmp1, tmp2);

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
	BvFastVec det = _mm_mul_ps(row0, m.r[0]);

	// Sum all values and replicate them through all elements
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)));
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)));

	return det;
}

inline float MatrixDeterminant(const BvFastMat & m)
{
	return _mm_cvtss_f32(MatrixDeterminantV(m));
}

inline BvFastMat MatrixInverse(const BvFastMat & m)
{
	BvFastMat mTmp = MatrixTranspose(m);

	// Calculate the first 12 2x2 determinant multiplications, to avoid repeating them later
	BvFastVec cf1 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(1, 3, 2, 3)));
	BvFastVec cf2 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 3, 1, 2)));
	BvFastVec cf3 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the first row
	BvFastVec tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	BvFastVec tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	BvFastVec row0 = _mm_mul_ps(tmp1, tmp2);

	// Compute the first multiplication for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(2, 3, 0, 1));
	BvFastVec row1 = _mm_mul_ps(tmp1, tmp2);

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

	BvFastVec row2 = _mm_mul_ps(tmp1, tmp2);

	// Compute the first multiplication for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(2, 3, 0, 1));
	BvFastVec row3 = _mm_mul_ps(tmp1, tmp2);

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
	BvFastVec det = _mm_mul_ps(row0, mTmp.r[0]);

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

inline BvFastMat operator + (const BvFastMat & m)
{
	return m;
}

inline BvFastMat operator - (const BvFastMat & m)
{
	BvFastMat r;
	r.r[0] = VectorNegate(m.r[0]);
	r.r[1] = VectorNegate(m.r[1]);
	r.r[2] = VectorNegate(m.r[2]);
	r.r[3] = VectorNegate(m.r[3]);

	return r;
}

inline BvFastMat operator + (const BvFastMat & m1, const BvFastMat & m2)
{
	return MatrixAdd(m1, m2);
}

inline BvFastMat & operator += (BvFastMat & m1, const BvFastMat & m2)
{
	m1 = MatrixAdd(m1, m2);
	return m1;
}

inline BvFastMat operator - (const BvFastMat & m1, const BvFastMat & m2)
{
	return MatrixSub(m1, m2);
}

inline BvFastMat & operator -= (BvFastMat & m1, const BvFastMat & m2)
{
	m1 = MatrixSub(m1, m2);
	return m1;
}

inline BvFastMat operator * (const BvFastMat & m1, const BvFastMat & m2)
{
	return MatrixMul(m1, m2);
}

inline BvFastMat & operator *= (BvFastMat & m1, const BvFastMat & m2)
{
	m1 = MatrixMul(m1, m2);
	return m1;
}

inline BvFastMat operator+(const BvFastMat & m, const float s)
{
	return MatrixAdd(m, s);
}

inline BvFastMat & operator+=(BvFastMat & m, const float s)
{
	m = MatrixAdd(m, s);
	return m;
}

inline BvFastMat operator-(const BvFastMat & m, const float s)
{
	return MatrixSub(m, s);
}

inline BvFastMat & operator-=(BvFastMat & m, const float s)
{
	m = MatrixSub(m, s);
	return m;
}

inline BvFastMat operator * (const BvFastMat & m, const float s)
{
	BvFastVec v = _mm_set1_ps(s);
	BvFastMat r;
	r.r[0] = _mm_mul_ps(m.r[0], v);
	r.r[1] = _mm_mul_ps(m.r[1], v);
	r.r[2] = _mm_mul_ps(m.r[2], v);
	r.r[3] = _mm_mul_ps(m.r[3], v);

	return r;
}

inline BvFastMat & operator *= (BvFastMat & m, const float s)
{
	BvFastVec v = _mm_set1_ps(s);
	m.r[0] = _mm_mul_ps(m.r[0], v);
	m.r[1] = _mm_mul_ps(m.r[1], v);
	m.r[2] = _mm_mul_ps(m.r[2], v);
	m.r[3] = _mm_mul_ps(m.r[3], v);

	return m;
}

inline BvFastMat operator / (const BvFastMat & m, const float s)
{
	return MatrixDiv(m, s);
}

inline BvFastMat & operator /= (BvFastMat & m, const float s)
{
	m = MatrixDiv(m, s);
	return m;
}

// ======================
// Specialized Operations
// ======================

inline BvFastMat MatrixScaling(const float sX, const float sY, const float sZ)
{
	BvFastMat r;
	r.r[0] = VectorSet(sX, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, sY, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, sZ, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline BvFastMat MatrixTranslation(const float tX, const float tY, const float tZ)
{
	BvFastMat r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(tX, tY, tZ, 1.0f);

	return r;
}

inline BvFastMat MatrixRotationX(const float angle)
{
	float cosSin[] = { cosf(angle), sinf(angle) };
	BvFastVec vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	BvFastVec vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	BvFastMat r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = _mm_shuffle_ps(vCosSin, vCosSin, _MM_SHUFFLE(3, 1, 0, 2));
	r.r[2] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 0, 1, 2));
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline BvFastMat MatrixRotationY(const float angle)
{
	float cosSin[] = { cosf(angle), sinf(angle) };
	BvFastVec vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	BvFastVec vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 1, 2, 0));
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = _mm_shuffle_ps(vCosSin, vCosSin, _MM_SHUFFLE(3, 0, 2, 1));
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline BvFastMat MatrixRotationZ(const float angle)
{
	float cosSin[] = { cosf(angle), sinf(angle) };
	BvFastVec vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	BvFastVec vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	BvFastMat r;
	r.r[0] = vCosSin;
	r.r[1] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 2, 0, 1));
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline BvFastMat MatrixRotationAxis(BvFastVec axis, const float angle)
{
	float sin = sinf(angle);
	float cos = cosf(angle);

	BvFastVec vSin = _mm_set1_ps(sin);
	BvFastVec vCos = _mm_set_ps(0.0f, cos, cos, cos);
	BvFastVec vOneMinusCos = _mm_set1_ps(1.0f - cos);

	// x * (1 - cos), y * (1 - cos), z * (1 - cos)
	BvFastVec d = _mm_mul_ps(axis, vOneMinusCos);

	BvFastVec t1 = d;

	// x^2 * (1 - cos), y^2 * (1 - cos), z^2 * (1 - cos)
	d = _mm_mul_ps(d, axis);
	// cos + x^2 * (1 - cos), cos + y^2 * (1 - cos), cos + z^2 * (1 - cos)
	d = _mm_add_ps(d, vCos);

	// x * (sin), y * (sin), z * (sin)
	BvFastVec t0 = _mm_mul_ps(axis, vSin);

	// z * (1 - cos), x * (1 - cos), y * (1 - cos)
	t1 = _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 1, 0, 2));
	// y * z * (1 - cos), z * x * (1 - cos), x * y * (1 - cos)
	t1 = _mm_mul_ps(t1, _mm_shuffle_ps(axis, axis, _MM_SHUFFLE(3, 0, 2, 1)));
	// y * z * (1 - cos) - x * (sin), z * x * (1 - cos) - y * (sin), x * y * (1 - cos) - z * (sin)
	BvFastVec t2 = _mm_sub_ps(t1, t0);
	// y * z * (1 - cos) + x * (sin), z * x * (1 - cos) + y * (sin), x * y * (1 - cos) + z * (sin)
	t1 = _mm_add_ps(t1, t0);

	// y * z * (1 - cos) + x * (sin), x * y * (1 - cos) + z * (sin), z * x * (1 - cos) - y * (sin), x * y * (1 - cos) - z * (sin)
	BvFastVec g0 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(2, 1, 2, 0));
	// y * z * (1 - cos) + x * (sin), 0, y * z * (1 - cos) - x * (sin), 0
	BvFastVec g1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(3, 0, 3, 1));

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(d, g0, _MM_SHUFFLE(2, 1, 3, 0));
	r.r[0] = _mm_shuffle_ps(r.r[0], r.r[0], _MM_SHUFFLE(1, 3, 2, 0));

	r.r[1] = _mm_shuffle_ps(d, g0, _MM_SHUFFLE(0, 3, 3, 1));
	r.r[1] = _mm_shuffle_ps(r.r[1], r.r[1], _MM_SHUFFLE(1, 3, 0, 2));

	r.r[2] = _mm_shuffle_ps(d, g1, _MM_SHUFFLE(2, 0, 3, 2));
	r.r[2] = _mm_shuffle_ps(r.r[2], r.r[2], _MM_SHUFFLE(1, 0, 3, 2));

	r.r[3] = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);

	return r;
}

inline BvFastMat MatrixLookAt(BvFastVec eyePos, BvFastVec dirVec, BvFastVec upVec)
{
	BvFastVec z = VectorNormalize(dirVec);

	BvFastVec x = VectorCross(upVec, z);
	x = VectorNormalize(x);

	BvFastVec y = VectorCross(z, x);

	BvFastMat r;
	r.r[0] = x;
	r.r[1] = y;
	r.r[2] = z;
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	r = MatrixTranspose(r);
	r.r[3] = VectorSet(-VectorDot(x, eyePos), -VectorDot(y, eyePos), -VectorDot(z, eyePos), 1.0f);

	return r;
}

inline BvFastMat MatrixLookAtLH(BvFastVec eyePos, BvFastVec lookPos, BvFastVec upVec)
{
	BvFastVec dir = VectorSub(lookPos, eyePos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline BvFastMat MatrixLookAtRH(BvFastVec eyePos, BvFastVec lookPos, BvFastVec upVec)
{
	BvFastVec dir = VectorSub(eyePos, lookPos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline BvFastMat MatrixPerspectiveLH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	BvFastVec v0 = _mm_set_ps(0.0f, -nearZ * b, a, a / aspectRatio);
	BvFastVec v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, b);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline BvFastMat MatrixPerspectiveRH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	BvFastVec v0 = _mm_set_ps(0.0f, nearZ * b, a, a / aspectRatio);
	BvFastVec v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, b);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline BvFastMat MatrixPerspectiveLH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ - nearZ;

	BvFastVec v0 = _mm_set_ps(0.0f, (2.0f * farZ * nearZ) / b, a, a / aspectRatio);
	BvFastVec v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, -(farZ + nearZ) / b);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline BvFastMat MatrixPerspectiveRH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ - nearZ;

	BvFastVec v0 = _mm_set_ps(0.0f, (-2.0f * farZ * nearZ) / b, a, a / aspectRatio);
	BvFastVec v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, -(farZ + nearZ) / b);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline BvFastMat MatrixPerspectiveLH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	BvFastVec v0 = _mm_set_ps(0.0f, -nearZ * b, -a, a / aspectRatio);
	BvFastVec v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, b);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline BvFastMat MatrixPerspectiveRH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	BvFastVec v0 = _mm_set_ps(0.0f, nearZ * b, -a, a / aspectRatio);
	BvFastVec v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, b);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

inline BvFastMat MatrixOrthographicOffCenterLH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverFMinusN = 1.0f / (farZ - nearZ);

	BvFastVec v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverFMinusN, 0.0f);
	BvFastVec v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-oneOverFMinusN * nearZ, 1.0f);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline BvFastMat MatrixOrthographicOffCenterRH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverNMinusF = 1.0f / (nearZ - farZ);

	BvFastVec v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverNMinusF, 0.0f);
	BvFastVec v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		oneOverNMinusF * nearZ, 1.0f);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline BvFastMat MatrixOrthographicOffCenterLH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverNMinusF = 1.0f / (nearZ - farZ);

	BvFastVec v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, -2.0f * oneOverNMinusF, 0.0f);
	BvFastVec v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		(nearZ + farZ) * oneOverNMinusF, 1.0f);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline BvFastMat MatrixOrthographicOffCenterRH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverFMinusN = 1.0f / (farZ - nearZ);

	BvFastVec v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, -2.0f * oneOverFMinusN, 0.0f);
	BvFastVec v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-(nearZ + farZ) * oneOverFMinusN, 1.0f);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline BvFastMat MatrixOrthographicOffCenterLH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (bottom - top);
	float oneOverFMinusN = 1.0f / (farZ - nearZ);

	BvFastVec v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverFMinusN, 0.0f);
	BvFastVec v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-oneOverFMinusN * nearZ, 1.0f);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

inline BvFastMat MatrixOrthographicOffCenterRH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (bottom - top);
	float oneOverNMinusF = 1.0f / (nearZ - farZ);

	BvFastVec v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverNMinusF, 0.0f);
	BvFastVec v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		oneOverNMinusF * nearZ, 1.0f);

	BvFastMat r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}


#endif