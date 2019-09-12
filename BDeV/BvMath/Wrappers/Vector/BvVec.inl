#pragma once


#include "../Matrix/BvMat.h"
#include "../Quaternion/BvQuat.h"
#include "../../Math/BvLoaders.h"
#include "../../Math/BvFastVec.h"


constexpr int g_Shuffles[4] =
{
	_MM_SHUFFLE(0, 0, 0, 0),
	_MM_SHUFFLE(1, 1, 1, 1),
	_MM_SHUFFLE(2, 2, 2, 2),
	_MM_SHUFFLE(3, 3, 3, 3)
};


inline BvVec::BvVec()
	: m128(VectorZero()) {}

inline BvVec::BvVec(const BvVec & rhs)
	: m128(rhs.m128) {}

inline BvVec::BvVec(BvVec && rhs)
	: m128(rhs.m128) {}

inline BvVec & BvVec::operator=(const BvVec & rhs)
{
	if (this != &rhs)
	{
		m128 = rhs.m128;
	}

	return *this;
}

inline BvVec & BvVec::operator=(BvVec && rhs)
{
	m128 = rhs.m128;

	return *this;
}

inline BvVec::BvVec(const float x, const float y, const float z, const float w)
	: m128(VectorSet(x, y, z, w)) {}

inline BvVec::BvVec(const float s)
	: m128(VectorReplicate(s)) {}

inline BvVec::BvVec(const Float2 & v)
	: m128(Load2(v.v)) {}

inline BvVec::BvVec(const Float3 & v)
	: m128(Load3(v.v)) {}

inline BvVec::BvVec(const Float4 & v)
	: m128(Load4(v.v)) {}

inline BvVec::BvVec(const BvFastVec & m128)
	: m128(m128) {}

inline void BvVec::Set(const float x, const float y, const float z, const float w)
{
	m128 = VectorSet(x, y, z, w);
}

inline void BvVec::Set(const float s)
{
	m128 = VectorReplicate(s);
}

inline void BvVec::Set(const Float2 & v)
{
	m128 = Load2(v.v);
}

inline void BvVec::Set(const Float3 & v)
{
	m128 = Load3(v.v);
}

inline void BvVec::Set(const Float4 & v)
{
	m128 = Load4(v.v);
}

inline void BvVec::Set(const BvFastVec & v)
{
	m128 = v;
}

inline void BvVec::SetZero()
{
	m128 = VectorZero();
}

inline float BvVec::GetX() const
{
	return VectorGetX(m128);
}

inline float BvVec::GetY() const
{
	return VectorGetY(m128);
}

inline float BvVec::GetZ() const
{
	return VectorGetZ(m128);
}

inline float BvVec::GetW() const
{
	return VectorGetW(m128);
}

inline Float2 BvVec::AsFloat2() const
{
	return v2;
}

inline Float3 BvVec::AsFloat3() const
{
	return v3;
}

inline Float4 BvVec::AsFloat4() const
{
	return v4;
}

inline BvQuat BvVec::ToQuat() const
{
	return BvQuat(m128);
}

inline BvVec BvVec::ReplicateX() const
{
	return BvVec(VectorReplicateX(m128));
}

inline BvVec BvVec::ReplicateY() const
{
	return BvVec(VectorReplicateY(m128));
}

inline BvVec BvVec::ReplicateZ() const
{
	return BvVec(VectorReplicateZ(m128));
}

inline BvVec BvVec::ReplicateW() const
{
	return BvVec(VectorReplicateW(m128));
}

template<unsigned int x, unsigned int y, unsigned int z, unsigned int w>
inline BvVec BvVec::Permute() const
{
	return BvVec(VectorPermute<x, y, z, w>());
}

inline BvVec BvVec::Max(const BvVec & v) const
{
	return BvVec(VectorMax(m128, v.m128));
}

inline BvVec BvVec::Min(const BvVec & v) const
{
	return BvVec(VectorMin(m128, v.m128));
}

inline BvVec BvVec::Abs() const
{
	return BvVec(VectorAbs(m128));
}

inline BvVec BvVec::Floor() const
{
	return BvVec(VectorFloor(m128));
}

inline BvVec BvVec::Ceil() const
{
	return BvVec(VectorCeil(m128));
}

inline BvVec BvVec::Round() const
{
	return BvVec(VectorRound(m128));
}

inline BvVec BvVec::Zero() const
{
	return BvVec(VectorZero());
}

inline BvVec BvVec::IsNearlyEqual(const BvVec & v, const float eps) const
{
	return BvVec(VectorIsEqual(m128, v.m128, eps));
}

inline BvVec BvVec::IsZero(const float eps) const
{
	return BvVec(VectorIsZero(m128, eps));
}

inline BvVec BvVec::Add(const BvVec & v) const
{
	return BvVec(VectorAdd(m128, v.m128));
}

inline BvVec BvVec::Sub(const BvVec & v) const
{
	return BvVec(VectorSub(m128, v.m128));
}

inline BvVec BvVec::Mul(const BvVec & v) const
{
	return BvVec(VectorMul(m128, v.m128));
}

inline BvVec BvVec::Div(const BvVec & v) const
{
	return BvVec(VectorDiv(m128, v.m128));
}

inline BvVec BvVec::Add(const float val) const
{
	return BvVec(VectorAdd(m128, val));
}

inline BvVec BvVec::Sub(const float val) const
{
	return BvVec(VectorSub(m128, val));
}

inline BvVec BvVec::Mul(const float val) const
{
	return BvVec(VectorMul(m128, val));
}

inline BvVec BvVec::Div(const float val) const
{
	return BvVec(VectorDiv(m128, val));
}

inline BvVec BvVec::Mul(const BvMat & m) const
{
	return BvVec(VectorMul(m128, m.mat));
}

inline float BvVec::Dot(const BvVec & v) const
{
	return VectorDot(m128, v.m128);
}

inline BvVec BvVec::Cross(const BvVec & v) const
{
	return BvVec(VectorCross(m128, v.m128));
}

inline float BvVec::Length() const
{
	return VectorLength(m128);
}

inline float BvVec::LengthSqr() const
{
	return VectorLengthSqr(m128);
}

inline void BvVec::Normalize()
{
	m128 = VectorNormalize(m128);
}

inline BvVec BvVec::Normal() const
{
	return BvVec(VectorNormalize(m128));
}

inline BvVec BvVec::Project(const BvVec & normal) const
{
	return BvVec(VectorProject(m128, normal.m128));
}

inline BvVec BvVec::Lerp(const BvVec & toVec, const float t) const
{
	return BvVec(VectorLerp(m128, toVec.m128, t));
}

inline BvVec BvVec::Reflect(const BvVec & normal)
{
	return BvVec(VectorReflection(m128, normal.m128));
}

inline const float & BvVec::operator[](const unsigned int index) const
{
	return arr[index];
}

inline float & BvVec::operator[](const unsigned int index)
{
	return arr[index];
}

inline BvVec operator+(const BvVec & lhs, const BvVec & rhs)
{
	return lhs.Add(rhs);
}

inline BvVec operator-(const BvVec & lhs, const BvVec & rhs)
{
	return lhs.Sub(rhs);
}

inline BvVec operator*(const BvVec & lhs, const BvVec & rhs)
{
	return lhs.Mul(rhs);
}

inline BvVec operator/(const BvVec & lhs, const BvVec & rhs)
{
	return lhs.Div(rhs);
}

inline BvVec operator+(const BvVec & lhs, const float val)
{
	return lhs.Add(val);
}

inline BvVec operator-(const BvVec & lhs, const float val)
{
	return lhs.Sub(val);
}

inline BvVec operator*(const BvVec & lhs, const float val)
{
	return lhs.Mul(val);
}

inline BvVec operator/(const BvVec & lhs, const float val)
{
	return lhs.Div(val);
}

inline BvVec operator+(const float val, const BvVec & rhs)
{
	return rhs.Add(val);
}

inline BvVec operator-(const float val, const BvVec & rhs)
{
	return BvVec(val).Sub(rhs);
}

inline BvVec operator*(const float val, const BvVec & rhs)
{
	return rhs.Mul(val);
}

inline BvVec operator/(const float val, const BvVec & rhs)
{
	return rhs.Div(val);
}

inline BvVec operator*(const BvVec & lhs, const BvMat & rhs)
{
	return lhs.Mul(rhs);
}

inline BvVec BvVec::operator+() const
{
	return *this;
}

inline BvVec BvVec::operator-() const
{
	return BvVec(VectorNegate(m128));
}

inline BvVec & BvVec::operator+=(const BvVec & v)
{
	m128 = VectorAdd(m128, v.m128);

	return *this;
}

inline BvVec & BvVec::operator-=(const BvVec & v)
{
	m128 = VectorSub(m128, v.m128);

	return *this;
}

inline BvVec & BvVec::operator*=(const BvVec & v)
{
	m128 = VectorMul(m128, v.m128);

	return *this;
}

inline BvVec & BvVec::operator/=(const BvVec & v)
{
	m128 = VectorDiv(m128, v.m128);

	return *this;
}

inline BvVec & BvVec::operator+=(const float val)
{
	m128 = VectorAdd(m128, val);

	return *this;
}

inline BvVec & BvVec::operator-=(const float val)
{
	m128 = VectorSub(m128, val);

	return *this;
}

inline BvVec & BvVec::operator*=(const float val)
{
	m128 = VectorMul(m128, val);

	return *this;
}

inline BvVec & BvVec::operator/=(const float val)
{
	m128 = VectorDiv(m128, val);

	return *this;
}

inline BvVec & BvVec::operator*=(const BvMat & m)
{
	m128 = VectorMul(m128, m.mat);

	return *this;
}

inline BvVec operator==(const BvVec & lhs, const BvVec & rhs)
{
	return BvVec(VectorIsExactlyEqualV(lhs.m128, rhs.m128));
}

inline BvVec operator<(const BvVec & lhs, const BvVec & rhs)
{
	return BvVec(VectorLess(lhs.m128, rhs.m128));
}

inline BvVec operator<=(const BvVec & lhs, const BvVec & rhs)
{
	return BvVec(VectorLessEq(lhs.m128, rhs.m128));
}

inline BvVec operator>(const BvVec & lhs, const BvVec & rhs)
{
	return BvVec(VectorGreater(lhs.m128, rhs.m128));
}

inline BvVec operator>=(const BvVec & lhs, const BvVec & rhs)
{
	return BvVec(VectorGreaterEq(lhs.m128, rhs.m128));
}

inline bool BvVec::AllTrue() const
{
	return (VectorGetMask(m128) & 0xF) == 0xF;
}

inline bool BvVec::AllFalse() const
{
	return VectorGetMask(m128) == 0;
}

inline bool BvVec::AnyTrue() const
{
	return (VectorGetMask(m128) & 0xF) > 0;
}

inline bool BvVec::AnyFalse() const
{
	return (VectorGetMask(m128) & 0xF) < 0xF;
}

template<bool x, bool y, bool z, bool w>
inline bool BvVec::True() const
{
	unsigned int mask = (x ? 1 : 0)
		| (y ? 1 : 0) << 1
		| (z ? 1 : 0) << 2
		| (w ? 1 : 0) << 3;

	return (VectorGetMask(m128) & mask) == mask;
}

template<bool x, bool y, bool z, bool w>
inline bool BvVec::False() const
{
	unsigned int mask = ~((x ? 1 : 0)
		| (y ? 1 : 0) << 1
		| (z ? 1 : 0) << 2
		| (w ? 1 : 0) << 3);

	return (VectorGetMask(m128) & mask) == mask;
}

inline unsigned int BvVec::GetMask() const
{
	return VectorGetMask(m128);
}