#pragma once


#include "BvFastVec_FPU.inl"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_FPU)


// ==================================
// Matrix operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Access
// ======================

mf32 MatrixSet(const vf32 & v0, const vf32 & v1, const vf32 & v2, const vf32 & v3);

// ======================
// Basic Operations
// ======================

mf32 MatrixAdd(const mf32 & m1, const mf32 & m2);
mf32 MatrixAdd(const mf32 & m, const float s);

mf32 MatrixSub(const mf32 & m1, const mf32 & m2);
mf32 MatrixSub(const mf32 & m, const float s);

mf32 MatrixMul(const mf32 & m1, const mf32 & m2);
mf32 MatrixMul(const mf32 & m, const float s);

mf32 MatrixDiv(const mf32 & m, const float s);

mf32 MatrixIdentity();

mf32 MatrixTranspose(const mf32 & m);

vf32 MatrixDeterminantV(const mf32 & m);
float MatrixDeterminant(const mf32 & m);

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

mf32 operator + (const mf32 & m, const float s);
mf32 & operator += (mf32 & m, const float s);

mf32 operator - (const mf32 & m, const float s);
mf32 & operator -= (mf32 & m, const float s);

mf32 operator * (const mf32 & m, const float s);
mf32 & operator *= (mf32 & m, const float s);

mf32 operator / (const mf32 & m, const float s);
mf32 & operator /= (mf32 & m, const float s);

// ======================
// Specialized Operations
// ======================

mf32 MatrixScaling(const float sX, const float sY, const float sZ);

mf32 MatrixTranslation(const float tX, const float tY, const float tZ);

mf32 MatrixRotationX(const float angle);
mf32 MatrixRotationY(const float angle);
mf32 MatrixRotationZ(const float angle);
mf32 MatrixRotationAxis(const vf32 & axis, const float angle);

mf32 MatrixLookAt(const vf32 & eyePos, const vf32 & dirVec, const vf32 & upVec);
mf32 MatrixLookAtLH(const vf32 & eyePos, const vf32 & lookPos, const vf32 & upVec);
mf32 MatrixLookAtRH(const vf32 & eyePos, const vf32 & lookPos, const vf32 & upVec);

mf32 MatrixPerspectiveLH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY);
mf32 MatrixPerspectiveRH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY);

mf32 MatrixPerspectiveLH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY);
mf32 MatrixPerspectiveRH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY);

mf32 MatrixPerspectiveLH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY);
mf32 MatrixPerspectiveRH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY);

mf32 MatrixOrthographicOffCenterLH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

mf32 MatrixOrthographicOffCenterRH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

mf32 MatrixOrthographicOffCenterLH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

mf32 MatrixOrthographicOffCenterRH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

mf32 MatrixOrthographicOffCenterLH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

mf32 MatrixOrthographicOffCenterRH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ);

// ======================
// Definitions
// ======================

// ======================
// Access
// ======================

inline mf32 MatrixSet(const vf32 & v0, const vf32 & v1, const vf32 & v2, const vf32 & v3)
{
	return mf32(v0, v1, v2, v3);
}

// ======================
// Basic Operations
// ======================

inline mf32 MatrixAdd(const mf32 & m1, const mf32 & m2)
{
	mf32 m;
	m.r[0] = VectorAdd(m1.r[0], m2.r[0]);
	m.r[1] = VectorAdd(m1.r[1], m2.r[1]);
	m.r[2] = VectorAdd(m1.r[2], m2.r[2]);
	m.r[3] = VectorAdd(m1.r[3], m2.r[3]);

	return m;
}

inline mf32 MatrixAdd(const mf32 & m, const float s)
{
	mf32 r;
	r.r[0] = VectorAdd(m.r[0], s);
	r.r[1] = VectorAdd(m.r[1], s);
	r.r[2] = VectorAdd(m.r[2], s);
	r.r[3] = VectorAdd(m.r[3], s);

	return r;
}

inline mf32 MatrixSub(const mf32 & m1, const mf32 & m2)
{
	mf32 m;
	m.r[0] = VectorSub(m1.r[0], m2.r[0]);
	m.r[1] = VectorSub(m1.r[1], m2.r[1]);
	m.r[2] = VectorSub(m1.r[2], m2.r[2]);
	m.r[3] = VectorSub(m1.r[3], m2.r[3]);

	return m;
}

inline mf32 MatrixSub(const mf32 & m, const float s)
{
	mf32 r;
	r.r[0] = VectorSub(m.r[0], s);
	r.r[1] = VectorSub(m.r[1], s);
	r.r[2] = VectorSub(m.r[2], s);
	r.r[3] = VectorSub(m.r[3], s);

	return r;
}

inline mf32 MatrixMul(const mf32 & m1, const mf32 & m2)
{
	mf32 m;
	m.r[0] = VectorSet(
			m1.r[0].x * m2.r[0].x + m1.r[0].y * m2.r[1].x + m1.r[0].z * m2.r[2].x + m1.r[0].w * m2.r[3].x,
			m1.r[0].x * m2.r[0].y + m1.r[0].y * m2.r[1].y + m1.r[0].z * m2.r[2].y + m1.r[0].w * m2.r[3].y,
			m1.r[0].x * m2.r[0].z + m1.r[0].y * m2.r[1].z + m1.r[0].z * m2.r[2].z + m1.r[0].w * m2.r[3].z,
			m1.r[0].x * m2.r[0].w + m1.r[0].y * m2.r[1].w + m1.r[0].z * m2.r[2].w + m1.r[0].w * m2.r[3].w
		);

	m.r[1] = VectorSet(
		m1.r[1].x * m2.r[0].x + m1.r[1].y * m2.r[1].x + m1.r[1].z * m2.r[2].x + m1.r[1].w * m2.r[3].x,
		m1.r[1].x * m2.r[0].y + m1.r[1].y * m2.r[1].y + m1.r[1].z * m2.r[2].y + m1.r[1].w * m2.r[3].y,
		m1.r[1].x * m2.r[0].z + m1.r[1].y * m2.r[1].z + m1.r[1].z * m2.r[2].z + m1.r[1].w * m2.r[3].z,
		m1.r[1].x * m2.r[0].w + m1.r[1].y * m2.r[1].w + m1.r[1].z * m2.r[2].w + m1.r[1].w * m2.r[3].w
	);

	m.r[2] = VectorSet(
		m1.r[2].x * m2.r[0].x + m1.r[2].y * m2.r[1].x + m1.r[2].z * m2.r[2].x + m1.r[2].w * m2.r[3].x,
		m1.r[2].x * m2.r[0].y + m1.r[2].y * m2.r[1].y + m1.r[2].z * m2.r[2].y + m1.r[2].w * m2.r[3].y,
		m1.r[2].x * m2.r[0].z + m1.r[2].y * m2.r[1].z + m1.r[2].z * m2.r[2].z + m1.r[2].w * m2.r[3].z,
		m1.r[2].x * m2.r[0].w + m1.r[2].y * m2.r[1].w + m1.r[2].z * m2.r[2].w + m1.r[2].w * m2.r[3].w
	);

	m.r[1] = VectorSet(
		m1.r[3].x * m2.r[0].x + m1.r[3].y * m2.r[1].x + m1.r[3].z * m2.r[2].x + m1.r[3].w * m2.r[3].x,
		m1.r[3].x * m2.r[0].y + m1.r[3].y * m2.r[1].y + m1.r[3].z * m2.r[2].y + m1.r[3].w * m2.r[3].y,
		m1.r[3].x * m2.r[0].z + m1.r[3].y * m2.r[1].z + m1.r[3].z * m2.r[2].z + m1.r[3].w * m2.r[3].z,
		m1.r[3].x * m2.r[0].w + m1.r[3].y * m2.r[1].w + m1.r[3].z * m2.r[2].w + m1.r[3].w * m2.r[3].w
	);

	return m;
}

inline mf32 MatrixMul(const mf32 & m, const float s)
{
	mf32 r;
	r.r[0] = VectorSet(
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s,
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s,
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s,
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s
	);

	r.r[1] = VectorSet(
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s,
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s,
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s,
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s
	);

	r.r[2] = VectorSet(
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s,
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s,
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s,
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s
	);

	r.r[1] = VectorSet(
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s,
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s,
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s,
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s
	);

	return r;
}

inline mf32 MatrixDiv(const mf32 & m, const float s)
{
	mf32 r;
	r.r[0] = VectorDiv(m.r[0], s);
	r.r[1] = VectorDiv(m.r[1], s);
	r.r[2] = VectorDiv(m.r[2], s);
	r.r[3] = VectorDiv(m.r[3], s);

	return r;
}

inline mf32 MatrixIdentity()
{
	mf32 m;
	m.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	m.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return m;
}

inline mf32 MatrixTranspose(const mf32 & m)
{
	mf32 mT;
	mT.r[0] = VectorSet(m.r[0].x, m.r[1].x, m.r[2].x, m.r[3].x);
	mT.r[1] = VectorSet(m.r[0].y, m.r[1].y, m.r[2].y, m.r[3].y);
	mT.r[2] = VectorSet(m.r[0].z, m.r[1].z, m.r[2].z, m.r[3].z);
	mT.r[3] = VectorSet(m.r[0].w, m.r[1].w, m.r[2].w, m.r[3].w);

	return mT;
}

inline vf32 MatrixDeterminantV(const mf32 & m)
{
	float a1 = m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z;
	float a2 = m.r[2].y * m.r[3].w - m.r[2].w * m.r[3].y;
	float a3 = m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y;
	float a4 = m.r[2].x * m.r[3].w - m.r[2].w * m.r[3].x;
	float a5 = m.r[2].x * m.r[3].z - m.r[2].z * m.r[3].x;
	float a6 = m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x;

	float det =
			m.r[0].x * (m.r[1].y * (a1) - m.r[1].z * (a2) + m.r[1].w * (a3))
		- m.r[0].y * (m.r[1].x * (a1) - m.r[1].z * (a4) + m.r[1].w * (a5))
		+ m.r[0].z * (m.r[1].x * (a2) - m.r[1].y * (a4) + m.r[1].w * (a6))
		- m.r[0].w * (m.r[1].x * (a3) - m.r[1].y * (a5) + m.r[1].z * (a6));

	return VectorSet(det, det, det, det);
}

inline float MatrixDeterminant(const mf32 & m)
{
	return MatrixDeterminantV(m).x;
}

inline mf32 MatrixInverse(const mf32 & m)
{
	mf32 mT = MatrixTranspose(m);

	// First 12 coefficients
	float tmp[12];
	tmp[0] =  mT.r[2].z * mT.r[3].w;
	tmp[1] =  mT.r[2].w * mT.r[3].z;
	tmp[2] =  mT.r[2].y * mT.r[3].w;
	tmp[3] =  mT.r[2].w * mT.r[3].y;
	tmp[4] =  mT.r[2].y * mT.r[3].z;
	tmp[5] =  mT.r[2].z * mT.r[3].y;
	tmp[6] =  mT.r[2].x * mT.r[3].w;
	tmp[7] =  mT.r[2].w * mT.r[3].x;
	tmp[8] =  mT.r[2].x * mT.r[3].z;
	tmp[9] =  mT.r[2].z * mT.r[3].x;
	tmp[10] = mT.r[2].x * mT.r[3].y;
	tmp[11] = mT.r[2].y * mT.r[3].x;

	mf32 r;
	r.r[0].x =  tmp[0] * mT.r[1].y + tmp[3] * mT.r[1].z + tmp[4] *  mT.r[1].w;
	r.r[0].x -= tmp[1] * mT.r[1].y + tmp[2] * mT.r[1].z + tmp[5] *  mT.r[1].w;
	r.r[0].y =  tmp[1] * mT.r[1].x + tmp[6] * mT.r[1].z + tmp[9] *  mT.r[1].w;
	r.r[0].y -= tmp[0] * mT.r[1].x + tmp[7] * mT.r[1].z + tmp[8] *  mT.r[1].w;
	r.r[0].z =  tmp[2] * mT.r[1].x + tmp[7] * mT.r[1].y + tmp[10] * mT.r[1].w;
	r.r[0].z -= tmp[3] * mT.r[1].x + tmp[6] * mT.r[1].y + tmp[11] * mT.r[1].w;
	r.r[0].w =  tmp[5] * mT.r[1].x + tmp[8] * mT.r[1].y + tmp[11] * mT.r[1].z;
	r.r[0].w -= tmp[4] * mT.r[1].x + tmp[9] * mT.r[1].y + tmp[10] * mT.r[1].z;
	r.r[1].x =  tmp[1] * mT.r[0].y + tmp[2] * mT.r[0].z + tmp[5] *  mT.r[0].w;
	r.r[1].x -= tmp[0] * mT.r[0].y + tmp[3] * mT.r[0].z + tmp[4] *  mT.r[0].w;
	r.r[1].y =  tmp[0] * mT.r[0].x + tmp[7] * mT.r[0].z + tmp[8] *  mT.r[0].w;
	r.r[1].y -= tmp[1] * mT.r[0].x + tmp[6] * mT.r[0].z + tmp[9] *  mT.r[0].w;
	r.r[1].z =  tmp[3] * mT.r[0].x + tmp[6] * mT.r[0].y + tmp[11] * mT.r[0].w;
	r.r[1].z -= tmp[2] * mT.r[0].x + tmp[7] * mT.r[0].y + tmp[10] * mT.r[0].w;
	r.r[1].w =  tmp[4] * mT.r[0].x + tmp[9] * mT.r[0].y + tmp[10] * mT.r[0].z;
	r.r[1].w -= tmp[5] * mT.r[0].x + tmp[8] * mT.r[0].y + tmp[11] * mT.r[0].z;

	// Second 12 coefficients
	tmp[0] =  mT.r[0].z * mT.r[1].w;
	tmp[1] =  mT.r[0].w * mT.r[1].z;
	tmp[2] =  mT.r[0].y * mT.r[1].w;
	tmp[3] =  mT.r[0].w * mT.r[1].y;
	tmp[4] =  mT.r[0].y * mT.r[1].z;
	tmp[5] =  mT.r[0].z * mT.r[1].y;
	tmp[6] =  mT.r[0].x * mT.r[1].w;
	tmp[7] =  mT.r[0].w * mT.r[1].x;
	tmp[8] =  mT.r[0].x * mT.r[1].z;
	tmp[9] =  mT.r[0].z * mT.r[1].x;
	tmp[10] = mT.r[0].x * mT.r[1].y;
	tmp[11] = mT.r[0].y * mT.r[1].x;

	r.r[2].x =  tmp[0]  * mT.r[3].y + tmp[3] *  mT.r[3].z + tmp[4] *  mT.r[3].w;
	r.r[2].x -= tmp[1]  * mT.r[3].y + tmp[2] *  mT.r[3].z + tmp[5] *  mT.r[3].w;
	r.r[2].y =  tmp[1]  * mT.r[3].x + tmp[6] *  mT.r[3].z + tmp[9] *  mT.r[3].w;
	r.r[2].y -= tmp[0]  * mT.r[3].x + tmp[7] *  mT.r[3].z + tmp[8] *  mT.r[3].w;
	r.r[2].z =  tmp[2]  * mT.r[3].x + tmp[7] *  mT.r[3].y + tmp[10] * mT.r[3].w;
	r.r[2].z -= tmp[3]  * mT.r[3].x + tmp[6] *  mT.r[3].y + tmp[11] * mT.r[3].w;
	r.r[2].w =  tmp[5]  * mT.r[3].x + tmp[8] *  mT.r[3].y + tmp[11] * mT.r[3].z;
	r.r[2].w -= tmp[4]  * mT.r[3].x + tmp[9] *  mT.r[3].y + tmp[10] * mT.r[3].z;
	r.r[3].x =  tmp[2]  * mT.r[2].z + tmp[5] *  mT.r[2].w + tmp[1] *  mT.r[2].y;
	r.r[3].x -= tmp[4]  * mT.r[2].w + tmp[0] *  mT.r[2].y + tmp[3] *  mT.r[2].z;
	r.r[3].y =  tmp[8]  * mT.r[2].w + tmp[0] *  mT.r[2].x + tmp[7] *  mT.r[2].z;
	r.r[3].y -= tmp[6]  * mT.r[2].z + tmp[9] *  mT.r[2].w + tmp[1] *  mT.r[2].x;
	r.r[3].z =  tmp[6]  * mT.r[2].y + tmp[11] * mT.r[2].w + tmp[3] *  mT.r[2].x;
	r.r[3].z -= tmp[10] * mT.r[2].w + tmp[2] *  mT.r[2].x + tmp[7] *  mT.r[2].y;
	r.r[3].w =  tmp[10] * mT.r[2].z + tmp[4] *  mT.r[2].x + tmp[9] *  mT.r[2].y;
	r.r[3].w -= tmp[8]  * mT.r[2].y + tmp[11] * mT.r[2].z + tmp[5] *  mT.r[2].x;

	float det = 1.0f / (mT.r[0].x * r.r[0].x + mT.r[0].y * r.r[0].y + mT.r[0].z * r.r[0].z + mT.r[0].w * r.r[0].w);

	r.r[0].x *= det;
	r.r[0].y *= det;
	r.r[0].z *= det;
	r.r[0].w *= det;
	r.r[1].x *= det;
	r.r[1].y *= det;
	r.r[1].z *= det;
	r.r[1].w *= det;
	r.r[2].x *= det;
	r.r[2].y *= det;
	r.r[2].z *= det;
	r.r[2].w *= det;
	r.r[3].x *= det;
	r.r[3].y *= det;
	r.r[3].z *= det;
	r.r[3].w *= det;
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

inline mf32 operator + (const mf32 & m, const float s)
{
	vf32 v = VectorReplicate(s);

	mf32 r;
	r.r[0] = VectorAdd(m.r[0], v);
	r.r[1] = VectorAdd(m.r[1], v);
	r.r[2] = VectorAdd(m.r[2], v);
	r.r[3] = VectorAdd(m.r[3], v);

	return r;
}

inline mf32 & operator += (mf32 & m, const float s)
{
	m.r[0] = VectorAdd(m.r[0], s);
	m.r[1] = VectorAdd(m.r[1], s);
	m.r[2] = VectorAdd(m.r[2], s);
	m.r[3] = VectorAdd(m.r[3], s);

	return m;
}

inline mf32 operator - (const mf32 & m, const float s)
{
	vf32 v = VectorReplicate(s);

	mf32 r;
	r.r[0] = VectorSub(m.r[0], v);
	r.r[1] = VectorSub(m.r[1], v);
	r.r[2] = VectorSub(m.r[2], v);
	r.r[3] = VectorSub(m.r[3], v);

	return r;
}

inline mf32 & operator -= (mf32 & m, const float s)
{
	m.r[0] = VectorSub(m.r[0], s);
	m.r[1] = VectorSub(m.r[1], s);
	m.r[2] = VectorSub(m.r[2], s);
	m.r[3] = VectorSub(m.r[3], s);

	return m;
}

inline mf32 operator * (const mf32 & m, const float s)
{
	vf32 v = VectorReplicate(s);

	mf32 r;
	r.r[0] = VectorMul(m.r[0], v);
	r.r[1] = VectorMul(m.r[1], v);
	r.r[2] = VectorMul(m.r[2], v);
	r.r[3] = VectorMul(m.r[3], v);

	return r;
}

inline mf32 & operator *= (mf32 & m, const float s)
{
	m.r[0] = VectorMul(m.r[0], s);
	m.r[1] = VectorMul(m.r[1], s);
	m.r[2] = VectorMul(m.r[2], s);
	m.r[3] = VectorMul(m.r[3], s);

	return m;
}

inline mf32 operator / (const mf32 & m, const float s)
{
	return MatrixDiv(m, s);
}

inline mf32 & operator /= (mf32 & m, const float s)
{
	m = MatrixDiv(m, s);
	return m;
}

// ======================
// Specialized Operations
// ======================

inline mf32 MatrixScaling(const float sX, const float sY, const float sZ)
{
	mf32 r;
	r.r[0] = VectorSet(sX, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, sY, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, sZ, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixTranslation(const float tX, const float tY, const float tZ)
{
	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(tX, tY, tZ, 1.0f);

	return r;
}

inline mf32 MatrixRotationX(const float angle)
{
	float cosSin[] = { cosf(angle), sinf(angle) };

	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, cosSin[0], cosSin[1], 0.0f);
	r.r[2] = VectorSet(0.0f, -cosSin[1], cosSin[0], 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationY(const float angle)
{
	float cosSin[] = { cosf(angle), sinf(angle) };

	mf32 r;
	r.r[0] = VectorSet(cosSin[0], 0.0f, -cosSin[1], 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(cosSin[1], 0.0f, cosSin[0], 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationZ(const float angle)
{
	float cosSin[] = { cosf(angle), sinf(angle) };

	mf32 r;
	r.r[0] = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	r.r[1] = VectorSet(-cosSin[1], cosSin[0], 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationAxis(const vf32 & axis, const float angle)
{
	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);
	float oneMinusCos = 1.0f - cosAngle;

	mf32 r;
	r.r[0].x = cosAngle + axis.x * axis.x * oneMinusCos;
	r.r[0].y = axis.y * axis.x * oneMinusCos + axis.z * sinAngle;
	r.r[0].z = axis.z * axis.x * oneMinusCos - axis.y * sinAngle;
	r.r[0].w = 0.0f;

	r.r[1].x = axis.x * axis.y * oneMinusCos - axis.z * sinAngle;
	r.r[1].y = cosAngle + axis.y * axis.y * oneMinusCos;
	r.r[1].z = axis.z * axis.y * oneMinusCos + axis.x * sinAngle;
	r.r[1].w = 0.0f;

	r.r[3].x = axis.x * axis.z * oneMinusCos + axis.y * sinAngle;
	r.r[3].y = axis.y * axis.z * oneMinusCos - axis.x * sinAngle;
	r.r[3].z = cosAngle + axis.z * axis.z * oneMinusCos;
	r.r[3].w = 0.0f;

	r.r[3].x = r.r[3].y = r.r[3].z = 0.0f;
	r.r[3].w = 1.0f;

	return r;
}

inline mf32 MatrixLookAt(const vf32 & eyePos, const vf32 & dirVec, const vf32 & upVec)
{
	vf32 z = VectorNormalize(dirVec);

	vf32 x = VectorCross(upVec, z);
	x = VectorNormalize(x);

	vf32 y = VectorCross(z, x);

	vf32 w = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	mf32 r(x, y, z, w);

	r = MatrixTranspose(r);
	r.r[3] = VectorSet(-VectorDot(x, eyePos), -VectorDot(y, eyePos), -VectorDot(z, eyePos), 1.0f);

	return r;
}

inline mf32 MatrixLookAtLH(const vf32 & eyePos, const vf32 & lookPos, const vf32 & upVec)
{
	vf32 dir = VectorSub(lookPos, eyePos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline mf32 MatrixLookAtRH(const vf32 & eyePos, const vf32 & lookPos, const vf32 & upVec)
{
	vf32 dir = VectorSub(eyePos, lookPos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline mf32 MatrixPerspectiveLH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	const float vals[] = { a / aspectRatio, a, b, 1.0f, -nearZ * b};

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveRH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	const float vals[] = { a / aspectRatio, a, b, -1.0f, nearZ * b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveLH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ - nearZ;

	const float vals[] = { a / aspectRatio, a, -(farZ + nearZ) / b, 1.0f, (2.0f * farZ * nearZ) / b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveRH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ - nearZ;

	const float vals[] = { a / aspectRatio, a, -(farZ + nearZ) / b, -1.0f, (-2.0f * farZ * nearZ) / b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveLH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	const float vals[] = { a / aspectRatio, -a, b, 1.0f, -nearZ * b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveRH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY)
{
	float a = 1.0f / tanf(fovY * 0.5f);
	float b = farZ / (farZ - nearZ);

	const float vals[] = { a / aspectRatio, -a, b, -1.0f, nearZ * b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverFMinusN = 1.0f / (farZ - nearZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverFMinusN;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, -oneOverFMinusN * nearZ, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_DX(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverNMinusF = 1.0f / (nearZ - farZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverNMinusF;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, oneOverNMinusF * nearZ, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverNMinusF = 1.0f / (nearZ - farZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = -2.0f * oneOverNMinusF;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, (nearZ + farZ) * oneOverNMinusF, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_GL(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (top - bottom);
	float oneOverFMinusN = 1.0f / (farZ - nearZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = -2.0f * oneOverFMinusN;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, -(nearZ + farZ) * oneOverFMinusN, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (bottom - top);
	float oneOverFMinusN = 1.0f / (farZ - nearZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverFMinusN;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, -oneOverFMinusN * nearZ, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_VK(const float right, const float left, const float top, const float bottom, const float
	nearZ, const float farZ)
{
	float oneOverRMinusL = 1.0f / (right - left);
	float oneOverTMinusB = 1.0f / (bottom - top);
	float oneOverNMinusF = 1.0f / (nearZ - farZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverNMinusF;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, oneOverNMinusF * nearZ, 1.0f);

	return r;
}


#endif