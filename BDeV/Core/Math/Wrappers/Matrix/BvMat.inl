#pragma once


#include "../Vector/BvVec.h"
#include "../Quaternion/BvQuat.h"
#include "../../BvLoaders.h"
#include "../../BvFastMat.h"


inline BvMat::BvMat()
	: mat(MatrixIdentity()) {}

inline BvMat::BvMat(const BvMat& rhs)
	: mat(rhs.mat) {}

inline BvMat::BvMat(BvMat&& rhs) noexcept
	: mat(rhs.mat) {}

inline BvMat& BvMat::operator=(const BvMat& rhs)
{
	if (this != &rhs)
	{
		mat = rhs.mat;
	}

	return *this;
}

inline BvMat& BvMat::operator=(BvMat&& rhs) noexcept
{
	mat = rhs.mat;

	return *this;
}

inline BvMat::BvMat(const Float2& r1, const Float2& r2)
	: mat{ Load2(r1.v), Load2(r2.v), VectorSet(0.0f, 0.0f, 1.0f, 0.0f), VectorSet(0.0f, 0.0f, 0.0f, 1.0f) } {}

inline BvMat::BvMat(const Float3& r1, const Float3& r2, const Float3& r3, const Float3& r4)
	: mat{ Load3(r1.v), Load3(r2.v), Load3(r3.v), VectorSet(r4.x, r4.y, r4.z, 1.0f) } {}

inline BvMat::BvMat(const Float4& r1, const Float4& r2, const Float4& r3, const Float4& r4)
	: mat{ Load4(r1.v), Load4(r2.v), Load4(r3.v), Load4(r4.v) } {}

inline BvMat::BvMat(const Float22& m)
	: mat{ Load2(m.r[0].v), Load2(m.r[1].v), VectorSet(0.0f, 0.0f, 1.0f, 0.0f), VectorSet(0.0f, 0.0f, 0.0f, 1.0f) } {}
inline BvMat::BvMat(const Float33& m)
	: mat{ Load3(m.r[0].v), Load3(m.r[1].v), Load3(m.r[2].v), VectorSet(0.0f, 0.0f, 0.0f, 1.0f) } {}
inline BvMat::BvMat(const Float43& m)
	: mat{ Load3(m.r[0].v), Load3(m.r[1].v), Load3(m.r[2].v), Load3(m.r[3].v) } {}
inline BvMat::BvMat(const Float44& m)
	: mat{ Load4(m.r[0].v), Load4(m.r[1].v), Load4(m.r[2].v), VectorSet(m.r[3].x, m.r[3].y, m.r[3].z, 1.0f) } {}

inline BvMat::BvMat(CRBvVec r1, CRBvVec r2, CRBvVec r3, CRBvVec r4)
	: arr{ r1, r2, r3, r4 } {}

inline BvMat::BvMat(crvf32 r1, crvf32 r2, crvf32 r3, crvf32 r4)
	: mat{ r1, r2, r3, r4 } {}

inline BvMat::BvMat(const mf32& m)
	: mat(m) {}

inline BvMat::BvMat(const BvQuat& q)
	: mat(QuaternionToMatrix(q.m128)) {}

inline void BvMat::Set(const Float2& r1, const Float2& r2)
{
	mat.r[0] = Load2(r1.v);
	mat.r[1] = Load2(r2.v);
	mat.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	mat.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);
}

inline void BvMat::Set(const Float3& r1, const Float3& r2, const Float3& r3, const Float3& r4)
{
	mat.r[0] = Load3(r1.v);
	mat.r[1] = Load3(r2.v);
	mat.r[2] = Load3(r3.v);
	mat.r[3] = Load3(r4.v);
}

inline void BvMat::Set(const Float4& r1, const Float4& r2, const Float4& r3, const Float4& r4)
{
	mat.r[0] = Load4(r1.v);
	mat.r[1] = Load4(r2.v);
	mat.r[2] = Load4(r3.v);
	mat.r[3] = Load4(r4.v);
}

inline void BvMat::Set(CRBvVec r1, CRBvVec r2, CRBvVec r3, CRBvVec r4)
{
	mat.r[0] = r1.m128;
	mat.r[1] = r2.m128;
	mat.r[2] = r3.m128;
	mat.r[3] = r4.m128;
}

inline void BvMat::Set(crvf32 r1, crvf32 r2, crvf32 r3, crvf32 r4)
{
	mat.r[0] = r1;
	mat.r[1] = r2;
	mat.r[2] = r3;
	mat.r[3] = r4;
}

inline void BvMat::Set(const BvQuat& q)
{
	mat = QuaternionToMatrix(q.m128);
}

inline BvVec BvMat::Get(unsigned int row) const
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

inline BvQuat BvMat::ToQuaternion() const
{
	return BvQuat(QuaternionFromMatrix(mat));
}

inline BvMat BvMat::FromQuaternion(const BvQuat& q) const
{
	return BvMat(QuaternionToMatrix(q.m128));
}

inline BvMat BvMat::Add(const BvMat& m) const
{
	return BvMat(MatrixAdd(mat, m.mat));
}

inline BvMat BvMat::Sub(const BvMat& m) const
{
	return BvMat(MatrixSub(mat, m.mat));
}

inline BvMat BvMat::Mul(const BvMat& m) const
{
	return BvMat(MatrixMul(mat, m.mat));
}

inline BvMat BvMat::Add(f32 val) const
{
	return BvMat(MatrixAdd(mat, val));
}

inline BvMat BvMat::Sub(f32 val) const
{
	return BvMat(MatrixSub(mat, val));
}

inline BvMat BvMat::Mul(f32 val) const
{
	return BvMat(MatrixMul(mat, val));
}

inline BvMat BvMat::Div(f32 val) const
{
	return BvMat(MatrixDiv(mat, val));
}

inline BvMat BvMat::Mul(const BvQuat& q) const
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

inline f32 BvMat::Determinant() const
{
	return MatrixDeterminant(mat);
}

inline BvMat BvMat::Inverse() const
{
	return BvMat(MatrixInverse(mat));
}

inline BvMat BvMat::Scale(f32 x, f32 y, f32 z) const
{
	return BvMat(MatrixScaling(x, y, z));
}

inline BvMat BvMat::RotX(f32 rad) const
{
	return BvMat(MatrixRotationX(rad));
}

inline BvMat BvMat::RotY(f32 rad) const
{
	return BvMat(MatrixRotationY(rad));
}

inline BvMat BvMat::RotZ(f32 rad) const
{
	return BvMat(MatrixRotationZ(rad));
}

inline BvMat BvMat::RotAxis(CRBvVec axis, f32 rad) const
{
	return BvMat(MatrixRotationAxis(axis.m128, rad));
}

inline BvMat BvMat::Translate(f32 x, f32 y, f32 z) const
{
	return BvMat(MatrixTranslation(x, y, z));
}

inline BvMat BvMat::LookAt(CRBvVec eyePos, CRBvVec dirVec, CRBvVec upVec) const
{
	return BvMat(MatrixLookAt(eyePos.m128, dirVec.m128, upVec.m128));
}

inline BvMat BvMat::PerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const
{
	return BvMat(MatrixPerspectiveLH_DX(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const
{
	return BvMat(MatrixPerspectiveRH_DX(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const
{
	return BvMat(MatrixPerspectiveLH_GL(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const
{
	return BvMat(MatrixPerspectiveRH_GL(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const
{
	return BvMat(MatrixPerspectiveLH_VK(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::PerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const
{
	return BvMat(MatrixPerspectiveRH_VK(nearZ, farZ, aspectRatio, fovY));
}

inline BvMat BvMat::OrthoLH_DX(f32 right, f32 left, f32 top, f32 bottom,
	f32 nearZ, f32 farZ) const
{
	return BvMat(MatrixOrthographicOffCenterLH_DX(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoRH_DX(f32 right, f32 left, f32 top, f32 bottom,
	f32 nearZ, f32 farZ) const
{
	return BvMat(MatrixOrthographicOffCenterRH_DX(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoLH_GL(f32 right, f32 left, f32 top, f32 bottom,
	f32 nearZ, f32 farZ) const
{
	return BvMat(MatrixOrthographicOffCenterLH_GL(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoRH_GL(f32 right, f32 left, f32 top, f32 bottom,
	f32 nearZ, f32 farZ) const
{
	return BvMat(MatrixOrthographicOffCenterRH_GL(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoLH_VK(f32 right, f32 left, f32 top, f32 bottom,
	f32 nearZ, f32 farZ) const
{
	return BvMat(MatrixOrthographicOffCenterLH_VK(right, left, top, bottom, nearZ, farZ));
}

inline BvMat BvMat::OrthoRH_VK(f32 right, f32 left, f32 top, f32 bottom,
	f32 nearZ, f32 farZ) const
{
	return BvMat(MatrixOrthographicOffCenterRH_VK(right, left, top, bottom, nearZ, farZ));
}

inline const BvVec& BvMat::operator[](unsigned int index) const
{
	return arr[index];
}

inline BvVec& BvMat::operator[](unsigned int index)
{
	return arr[index];
}

inline BvMat operator+(const BvMat& lhs, const BvMat& rhs)
{
	return BvMat(MatrixAdd(lhs.mat, rhs.mat));
}

inline BvMat operator-(const BvMat& lhs, const BvMat& rhs)
{
	return BvMat(MatrixSub(lhs.mat, rhs.mat));
}

inline BvMat operator*(const BvMat& lhs, const BvMat& rhs)
{
	return BvMat(MatrixMul(lhs.mat, rhs.mat));
}

inline BvMat operator+(const BvMat& lhs, f32 val)
{
	return BvMat(MatrixAdd(lhs.mat, val));
}

inline BvMat operator-(const BvMat& lhs, f32 val)
{
	return BvMat(MatrixSub(lhs.mat, val));
}

inline BvMat operator*(const BvMat& lhs, f32 val)
{
	return BvMat(MatrixMul(lhs.mat, val));
}

inline BvMat operator/(const BvMat& lhs, f32 val)
{
	return BvMat(MatrixDiv(lhs.mat, val));
}

inline BvMat operator+(f32 val, const BvMat& rhs)
{
	return BvMat(MatrixAdd(rhs.mat, val));
}

inline BvMat operator-(f32 val, const BvMat& rhs)
{
	mf32 lhs{ VectorReplicate(val), VectorReplicate(val), VectorReplicate(val), VectorReplicate(val) };
	return BvMat(MatrixSub(lhs, rhs.mat));
}

inline BvMat operator*(f32 val, const BvMat& rhs)
{
	mf32 lhs{ VectorReplicate(val), VectorReplicate(val), VectorReplicate(val), VectorReplicate(val) };
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

inline BvMat& BvMat::operator+=(const BvMat& m)
{
	mat = MatrixAdd(mat, m.mat);
	
	return *this;
}

inline BvMat& BvMat::operator-=(const BvMat& m)
{
	mat = MatrixSub(mat, m.mat);

	return *this;
}

inline BvMat& BvMat::operator*=(const BvMat& m)
{
	mat = MatrixMul(mat, m.mat);

	return *this;
}

inline BvMat& BvMat::operator+=(f32 val)
{
	mat = MatrixAdd(mat, val);

	return *this;
}

inline BvMat& BvMat::operator-=(f32 val)
{
	mat = MatrixSub(mat, val);

	return *this;
}

inline BvMat& BvMat::operator*=(f32 val)
{
	mat = MatrixMul(mat, val);

	return *this;
}

inline BvMat& BvMat::operator/=(f32 val)
{
	mat = MatrixDiv(mat, val);

	return *this;
}

inline BvMat operator*(const BvQuat& lhs, const BvMat& rhs)
{
	return BvMat(MatrixMul(QuaternionToMatrix(lhs.m128), rhs.mat));
}

inline BvMat operator*(const BvMat& lhs, const BvQuat& rhs)
{
	return BvMat(MatrixMul(lhs.mat, QuaternionToMatrix(rhs.m128)));
}