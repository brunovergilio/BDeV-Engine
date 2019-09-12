#pragma once


#include "../Vector/BvVec.h"
#include "../Matrix/BvMat.h"
#include "../../Math/BvLoaders.h"
#include "../../Math/BvFastQuat.h"


inline BvQuat::BvQuat()
	: m128(VectorSet(0.0f, 0.0f, 0.0f, 1.0f)) {}

inline BvQuat::BvQuat(const BvQuat & rhs)
	: m128(rhs.m128) {}

inline BvQuat::BvQuat(BvQuat && rhs)
	: m128(rhs.m128) {}

inline BvQuat & BvQuat::operator=(const BvQuat & rhs)
{
	if (this != &rhs)
	{
		m128 = rhs.m128;
	}

	return *this;
}

inline BvQuat & BvQuat::operator=(BvQuat && rhs)
{
	m128 = rhs.m128;

	return *this;
}

inline BvQuat::BvQuat(const float x, const float y, const float z, const float w)
	: m128(VectorSet(x, y, z, w)) {}

inline BvQuat::BvQuat(const Float4 & v)
	: m128(Load4(v.v)) {}

inline BvQuat::BvQuat(const BvFastQuat & m128)
	: m128(m128) {}

inline BvQuat::BvQuat(const BvVec & v)
	: m128(v.m128) {}

inline BvQuat::BvQuat(const BvVec & axis, const float rad)
	: m128(QuaternionRotationAxis(axis.m128, rad)) {}

inline BvQuat::BvQuat(const BvMat & m)
	: m128(QuaternionFromMatrix(m.mat)) {}

inline void BvQuat::Set(const float x, const float y, const float z, const float w)
{
	m128 = VectorSet(x, y, z, w);
}

inline void BvQuat::Set(const Float4 & v)
{
	m128 = Load4(v.v);
}

inline void BvQuat::Set(const BvFastQuat & q)
{
	m128 = q;
}

inline void BvQuat::Set(const BvVec & v)
{
	m128 = v.m128;
}

inline void BvQuat::Set(const BvVec & axis, const float rad)
{
	m128 = QuaternionRotationAxis(axis.m128, rad);
}

inline void BvQuat::Set(const BvMat & m)
{
	m128 = QuaternionFromMatrix(m.mat);
}

inline void BvQuat::SetZero()
{
	m128 = VectorZero();
}

inline float BvQuat::GetX() const
{
	return VectorGetX(m128);
}

inline float BvQuat::GetY() const
{
	return VectorGetY(m128);
}

inline float BvQuat::GetZ() const
{
	return VectorGetZ(m128);
}

inline float BvQuat::GetW() const
{
	return VectorGetW(m128);
}

inline Float4 BvQuat::AsFloat4() const
{
	return v4;
}

inline BvQuat BvQuat::Identity() const
{
	return BvQuat(QuaternionIdentity());
}

inline BvQuat BvQuat::Mul(const BvVec & v) const
{
	return BvQuat(QuaternionMul(m128, v.m128));
}

inline BvQuat BvQuat::Mul(const BvMat & m) const
{
	return BvQuat(QuaternionMul(m128, QuaternionFromMatrix(m.mat)));
}

inline BvQuat BvQuat::Mul(const BvQuat & q) const
{
	return BvQuat(QuaternionMul(m128, q.m128));
}

inline float BvQuat::Dot(const BvQuat & q) const
{
	return QuaternionDot(m128, q.m128);
}

inline float BvQuat::Length() const
{
	return QuaternionLength(m128);
}

inline float BvQuat::LengthSqr() const
{
	return QuaternionLengthSqr(m128);
}

inline void BvQuat::Normalize()
{
	m128 = QuaternionNormalize(m128);
}

inline BvQuat BvQuat::Normal() const
{
	return BvQuat(QuaternionNormalize(m128));
}

inline BvQuat BvQuat::Conjugate() const
{
	return BvQuat(QuaternionConjugate(m128));
}

inline BvQuat BvQuat::Inverse() const
{
	return BvQuat(QuaternionInverse(m128));
}

inline BvQuat BvQuat::RotAxis(const BvVec & axis, const float rad) const
{
	return BvQuat(QuaternionRotationAxis(axis.m128, rad));
}

inline float BvQuat::Angle() const
{
	return QuaternionAngle(m128);
}

inline BvQuat BvQuat::Slerp(const BvQuat & toQuat, const float t) const
{
	return BvQuat(QuaternionSlerp(m128, toQuat.m128, t));
}

inline BvMat BvQuat::ToMatrix() const
{
	return BvMat(QuaternionToMatrix(m128));
}

inline BvQuat BvQuat::FromMatrix(const BvMat & m) const
{
	return BvQuat(QuaternionFromMatrix(m.mat));
}

inline BvQuat BvQuat::QVQC(const BvVec & v) const
{
	return BvQuat(QuaternionQVQC(m128, v.m128));
}

inline BvQuat BvQuat::QCVQ(const BvVec & v) const
{
	return BvQuat(QuaternionQCVQ(m128, v.m128));
}

inline const float & BvQuat::operator[](const unsigned int index) const
{
	return arr[index];
}

inline float & BvQuat::operator[](const unsigned int index)
{
	return arr[index];
}

inline BvQuat operator*(const BvQuat & lhs, const BvVec & rhs)
{
	return BvQuat(QuaternionMul(lhs.m128, rhs.m128));
}

inline BvQuat operator*(const BvVec & lhs, const BvQuat & rhs)
{
	return BvQuat(QuaternionMul(lhs.m128, rhs.m128));
}

inline BvQuat operator*(const BvQuat & lhs, const BvQuat & rhs)
{
	return BvQuat(QuaternionMul(lhs.m128, rhs.m128));
}

inline BvQuat & BvQuat::operator*=(const BvQuat & q)
{
	m128 = QuaternionMul(m128, q.m128);

	return *this;
}

inline BvQuat & BvQuat::operator*=(const BvVec & v)
{
	m128 = QuaternionMul(m128, v.m128);

	return *this;
}

inline BvQuat & BvQuat::operator*=(const BvMat & m)
{
	m128 = QuaternionMul(m128, QuaternionFromMatrix(m.mat));

	return *this;
}