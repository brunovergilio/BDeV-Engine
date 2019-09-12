#pragma once


#include "../Vector/BvVec.h"
#include "../Quaternion/BvQuat.h"
#include "../../Math/BvLoaders.h"
#include "../../Math/BvFastMat.h"


inline BvMat::BvMat() : mat(MatrixIdentity()) {}

inline BvMat::BvMat(const BvMat & rhs)
	: mat(rhs.mat) {}

inline BvMat::BvMat(BvMat && rhs)
	: mat(rhs.mat) {}

inline BvMat & BvMat::operator=(const BvMat & rhs)
{
	if (this != &rhs)
	{
		mat = rhs.mat;
	}

	return *this;
}

inline BvMat & BvMat::operator=(BvMat && rhs)
{
	mat = rhs.mat;

	return *this;
}

inline BvMat::BvMat(const Float2 & r1, const Float2 & r2)
	: mat{ Load2(r1.v), Load2(r2.v), VectorSet(0.0f, 0.0f, 1.0f, 0.0f), VectorSet(0.0f, 0.0f, 0.0f, 1.0f) } {}

inline BvMat::BvMat(const Float3 & r1, const Float3 & r2, const Float3 & r3, const Float3 & r4)
	: mat{ Load3(r1.v), Load3(r2.v), Load3(r3.v), Load3(r4.v) } {}

inline BvMat::BvMat(const Float4 & r1, const Float4 & r2, const Float4 & r3, const Float4 & r4)
	: mat{ Load4(r1.v), Load4(r2.v), Load4(r3.v), Load4(r4.v) } {}

inline BvMat::BvMat(const BvVec & r1, const BvVec & r2, const BvVec & r3, const BvVec & r4)
	: arr{ r1, r2, r3, r4 } {}

inline BvMat::BvMat(BvFastVec r1, BvFastVec r2, BvFastVec r3, BvFastVec r4)
	: mat{ r1, r2, r3, r4 } {}

inline BvMat::BvMat(const BvFastMat m)
	: mat(m) {}

inline BvMat::BvMat(const BvQuat & q)
	: mat(QuaternionToMatrix(q.m128)) {}

inline void BvMat::Set(const Float2 & r1, const Float2 & r2)
{
	mat.r[0] = Load2(r1.v);
	mat.r[1] = Load2(r2.v);
	mat.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	mat.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);
}

inline void BvMat::Set(const Float3 & r1, const Float3 & r2, const Float3 & r3, const Float3 & r4)
{
	mat.r[0] = Load3(r1.v);
	mat.r[1] = Load3(r2.v);
	mat.r[2] = Load3(r3.v);
	mat.r[3] = Load3(r4.v);
}

inline void BvMat::Set(const Float4 & r1, const Float4 & r2, const Float4 & r3, const Float4 & r4)
{
	mat.r[0] = Load4(r1.v);
	mat.r[1] = Load4(r2.v);
	mat.r[2] = Load4(r3.v);
	mat.r[3] = Load4(r4.v);
}

inline void BvMat::Set(const BvVec & r1, const BvVec & r2, const BvVec & r3, const BvVec & r4)
{
	mat.r[0] = r1.m128;
	mat.r[1] = r2.m128;
	mat.r[2] = r3.m128;
	mat.r[3] = r4.m128;
}

inline void BvMat::Set(BvFastVec r1, BvFastVec r2, BvFastVec r3, BvFastVec r4)
{
	mat.r[0] = r1;
	mat.r[1] = r2;
	mat.r[2] = r3;
	mat.r[3] = r4;
}

inline void BvMat::Set(const BvQuat & q)
{
	mat = QuaternionToMatrix(q.m128);
}

inline BvVec BvMat::Get(const unsigned int row) const
{
	return arr[row];
}

inline Float22 BvMat::AsFloat22() const
{
	return Float22(Float2(m00, m01), Float2(m10, m11));
}

inline Float33 BvMat::AsFloat33() const
{
	return Float33(Float3(m00, m01, m02), Float3(m10, m11, m12), Float3(m20, m21, m22));
}

inline Float43 BvMat::AsFloat43() const
{
	return Float43(Float3(m00, m01, m02), Float3(m10, m11, m12), Float3(m20, m21, m22), Float3(m30, m31, m32));
}

inline Float44 BvMat::AsFloat44() const
{
	return Float44(Float4(m00, m01, m02, m03), Float4(m10, m11, m12, m13), Float4(m20, m21, m22, m23), Float4(m30, m31, m32, m33));
}

inline BvQuat BvMat::ToQuaternion() const
{
	return BvQuat(QuaternionFromMatrix(mat));
}

inline BvMat BvMat::FromQuaternion(const BvQuat & q) const
{
	return BvMat(QuaternionToMatrix(q.m128));
}

inline BvMat BvMat::Add(const BvMat & m) const
{
	return BvMat(MatrixAdd(mat, m.mat));
}

inline BvMat BvMat::Sub(const BvMat & m) const
{
	return BvMat(MatrixSub(mat, m.mat));
}

inline BvMat BvMat::Mul(const BvMat & m) const
{
	return BvMat(MatrixMul(mat, m.mat));
}

inline BvMat BvMat::Add(const float val) const
{
	return BvMat(MatrixAdd(mat, val));
}

inline BvMat BvMat::Sub(const float val) const
{
	return BvMat(MatrixSub(mat, val));
}

inline BvMat BvMat::Mul(const float val) const
{
	return BvMat(MatrixMul(mat, val));
}

inline BvMat BvMat::Div(const float val) const
{
	return BvMat(MatrixDiv(mat, val));
}

inline BvMat BvMat::Mul(const BvQuat & q) const
{
	return BvMat(MatrixMul(mat, QuaternionToMatrix(q.m128)));
}

inline BvMat BvMat::Identity() const
{
	return BvMat(MatrixIdentity());
}

inline BvMat BvMat::Transpose() const
{
	return BvMat(MatrixTranspose(mat));
}

inline float BvMat::Determinant() const
{
	return MatrixDeterminant(mat);
}

inline BvMat BvMat::Inverse() const
{
	return BvMat(MatrixInverse(mat));
}

inline BvMat BvMat::Scale(const float x, const float y, const float z) const
{
	return BvMat(MatrixScaling(x, y, z));
}

inline BvMat BvMat::RotX(const float rad) const
{
	return BvMat(MatrixRotationX(rad));
}

inline BvMat BvMat::RotY(const float rad) const
{
	return BvMat(MatrixRotationY(rad));
}

inline BvMat BvMat::RotZ(const float rad) const
{
	return BvMat(MatrixRotationZ(rad));
}

inline BvMat BvMat::RotAxis(const BvVec & axis, const float rad) const
{
	return BvMat(MatrixRotationAxis(axis.m128, rad));
}

inline BvMat BvMat::Translate(const float x, const float y, const float z) const
{
	return BvMat(MatrixTranslation(x, y, z));
}

inline BvMat BvMat::LookAt(const BvVec & eyePos, const BvVec & dirVec, const BvVec & upVec) const
{
	return BvMat(MatrixLookAt(eyePos.m128, dirVec.m128, upVec.m128));
}

inline BvMat BvMat::PerspectiveLH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const
{
	return BvMat(MatrixPerspectiveLH_DX(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveRH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const
{
	return BvMat(MatrixPerspectiveRH_DX(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveLH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const
{
	return BvMat(MatrixPerspectiveLH_GL(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveRH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const
{
	return BvMat(MatrixPerspectiveRH_GL(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveLH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const
{
	return BvMat(MatrixPerspectiveLH_VK(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveRH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const
{
	return BvMat(MatrixPerspectiveRH_VK(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::OrthoLH_DX(const float right, const float left, const float top, const float bottom,
	const float nearZ, const float farZ) const
{
	return BvMat(MatrixOrthographicOffCenterLH_DX(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoRH_DX(const float right, const float left, const float top, const float bottom,
	const float nearZ, const float farZ) const
{
	return BvMat(MatrixOrthographicOffCenterRH_DX(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoLH_GL(const float right, const float left, const float top, const float bottom,
	const float nearZ, const float farZ) const
{
	return BvMat(MatrixOrthographicOffCenterLH_GL(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoRH_GL(const float right, const float left, const float top, const float bottom,
	const float nearZ, const float farZ) const
{
	return BvMat(MatrixOrthographicOffCenterRH_GL(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoLH_VK(const float right, const float left, const float top, const float bottom,
	const float nearZ, const float farZ) const
{
	return BvMat(MatrixOrthographicOffCenterLH_VK(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoRH_VK(const float right, const float left, const float top, const float bottom,
	const float nearZ, const float farZ) const
{
	return BvMat(MatrixOrthographicOffCenterRH_VK(right, left, top, bottom, nearZ, farZ));
}

inline const BvVec & BvMat::operator[](const unsigned int index) const
{
	return arr[index];
}

inline BvVec & BvMat::operator[](const unsigned int index)
{
	return arr[index];
}

inline BvMat operator+(const BvMat & lhs, const BvMat & rhs)
{
	return BvMat(MatrixAdd(lhs.mat, rhs.mat));
}

inline BvMat operator-(const BvMat & lhs, const BvMat & rhs)
{
	return BvMat(MatrixSub(lhs.mat, rhs.mat));
}

inline BvMat operator*(const BvMat & lhs, const BvMat & rhs)
{
	return BvMat(MatrixMul(lhs.mat, rhs.mat));
}

inline BvMat operator+(const BvMat & lhs, const float val)
{
	return BvMat(MatrixAdd(lhs.mat, val));
}

inline BvMat operator-(const BvMat & lhs, const float val)
{
	return BvMat(MatrixSub(lhs.mat, val));
}

inline BvMat operator*(const BvMat & lhs, const float val)
{
	return BvMat(MatrixMul(lhs.mat, val));
}

inline BvMat operator/(const BvMat & lhs, const float val)
{
	return BvMat(MatrixDiv(lhs.mat, val));
}

inline BvMat operator+(const float val, const BvMat & rhs)
{
	return BvMat(MatrixAdd(rhs.mat, val));
}

inline BvMat operator-(const float val, const BvMat & rhs)
{
	BvFastMat lhs{ VectorReplicate(val), VectorReplicate(val), VectorReplicate(val), VectorReplicate(val) };
	return BvMat(MatrixSub(lhs, rhs.mat));
}

inline BvMat operator*(const float val, const BvMat & rhs)
{
	BvFastMat lhs{ VectorReplicate(val), VectorReplicate(val), VectorReplicate(val), VectorReplicate(val) };
	return BvMat(MatrixSub(lhs, rhs.mat));
}

inline BvMat BvMat::operator+()
{
	return *this;
}

inline BvMat BvMat::operator-()
{
	return BvMat(-arr[0], -arr[1], -arr[2], -arr[3]);
}

inline BvMat & BvMat::operator+=(const BvMat & m)
{
	mat = MatrixAdd(mat, m.mat);
	
	return *this;
}

inline BvMat & BvMat::operator-=(const BvMat & m)
{
	mat = MatrixSub(mat, m.mat);

	return *this;
}

inline BvMat & BvMat::operator*=(const BvMat & m)
{
	mat = MatrixMul(mat, m.mat);

	return *this;
}

inline BvMat & BvMat::operator+=(const float val)
{
	mat = MatrixAdd(mat, val);

	return *this;
}

inline BvMat & BvMat::operator-=(const float val)
{
	mat = MatrixSub(mat, val);

	return *this;
}

inline BvMat & BvMat::operator*=(const float val)
{
	mat = MatrixMul(mat, val);

	return *this;
}

inline BvMat & BvMat::operator/=(const float val)
{
	mat = MatrixDiv(mat, val);

	return *this;
}

inline BvMat operator*(const BvQuat & lhs, const BvMat & rhs)
{
	return BvMat(MatrixMul(QuaternionToMatrix(lhs.m128), rhs.mat));
}

inline BvMat operator*(const BvMat & lhs, const BvQuat & rhs)
{
	return BvMat(MatrixMul(lhs.mat, QuaternionToMatrix(rhs.m128)));
}