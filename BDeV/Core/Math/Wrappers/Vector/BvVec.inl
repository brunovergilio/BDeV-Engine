#pragma once


#include "BDeV/Core/Math/BvFastVec.h"
#include "BDeV/Core/Math/Wrappers/Matrix/BvMat.h"
#include "BDeV/Core/Math/Wrappers/Quaternion/BvQuat.h"
#include "BDeV/Core/Math/BvLoaders.h"


inline BvVec::BvVec()
	: m128(VectorZero()) {}

inline BvVec::BvVec(const BvVec& rhs)
	: m128(rhs.m128) {}

inline BvVec::BvVec(BvVec&& rhs) noexcept
	: m128(rhs.m128) {}

inline BvVec& BvVec::operator=(const BvVec& rhs)
{
	if (this != &rhs)
	{
		m128 = rhs.m128;
	}

	return *this;
}

inline BvVec& BvVec::operator=(BvVec&& rhs) noexcept
{
	m128 = rhs.m128;

	return *this;
}

inline  BvVec::BvVec(f32 x, f32 y, f32 z, f32 w)
	: m128(VectorSet(x, y, z, w)) {}

inline BvVec::BvVec(f32 s)
	: m128(VectorReplicate(s)) {}

inline BvVec::BvVec(const Float2& v)
	: m128(Load2(v.v)) {}

inline BvVec::BvVec(const Float3& v)
	: m128(Load3(v.v)) {}

inline BvVec::BvVec(const Float4& v)
	: m128(Load4(v.v)) {}

inline BvVec::BvVec(crvf32 m128)
	: m128(m128) {}

inline void BvVec::Set(f32 x, f32 y, f32 z, f32 w)
{
	m128 = VectorSet(x, y, z, w);
}

inline void BvVec::Set(f32 s)
{
	m128 = VectorReplicate(s);
}

inline void BvVec::Set(const Float2& v)
{
	m128 = Load2(v.v);
}

inline void BvVec::Set(const Float3& v)
{
	m128 = Load3(v.v);
}

inline void BvVec::Set(const Float4& v)
{
	m128 = Load4(v.v);
}

inline void BvVec::Set(crvf32 v)
{
	m128 = v;
}

inline void BvVec::SetZero()
{
	m128 = VectorZero();
}

inline f32 BvVec::GetX() const
{
	return VectorGetX(m128);
}

inline f32 BvVec::GetY() const
{
	return VectorGetY(m128);
}

inline f32 BvVec::GetZ() const
{
	return VectorGetZ(m128);
}

inline f32 BvVec::GetW() const
{
	return VectorGetW(m128);
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

template<unsigned int X, unsigned int Y, unsigned int Z, unsigned int W>
inline BvVec BvVec::Permute() const
{
	return VectorPermute<X, Y, Z, W>(m128);
}

inline BvVec BvVec::Max(CRBvVec v) const
{
	return BvVec(VectorMax(m128, v.m128));
}

inline BvVec BvVec::Min(CRBvVec v) const
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

inline BvVec BvVec::IsNearlyEqual(CRBvVec v, f32 eps) const
{
	return BvVec(VectorNearlyEqual(m128, v.m128, eps));
}

inline BvVec BvVec::IsZero(f32 eps) const
{
	return BvVec(VectorNearlyEqual(m128, VectorZero(), eps));
}

inline BvVec BvVec::Add(CRBvVec v) const
{
	return BvVec(VectorAdd(m128, v.m128));
}

inline BvVec BvVec::Sub(CRBvVec v) const
{
	return BvVec(VectorSub(m128, v.m128));
}

inline BvVec BvVec::Mul(CRBvVec v) const
{
	return BvVec(VectorMul(m128, v.m128));
}

inline BvVec BvVec::Div(CRBvVec v) const
{
	return BvVec(VectorDiv(m128, v.m128));
}

inline BvVec BvVec::Add(f32 val) const
{
	return BvVec(VectorAdd(m128, val));
}

inline BvVec BvVec::Sub(f32 val) const
{
	return BvVec(VectorSub(m128, val));
}

inline BvVec BvVec::Mul(f32 val) const
{
	return BvVec(VectorMul(m128, val));
}

inline BvVec BvVec::Div(f32 val) const
{
	return BvVec(VectorDiv(m128, val));
}

inline BvVec BvVec::Mul(const BvMat& m) const
{
	return BvVec(VectorMul(m128, m.mat));
}

inline f32 BvVec::Dot(CRBvVec v) const
{
	return VectorDot(m128, v.m128);
}

inline BvVec BvVec::Cross(CRBvVec v) const
{
	return BvVec(VectorCross(m128, v.m128));
}

inline f32 BvVec::Length() const
{
	return VectorLength(m128);
}

inline f32 BvVec::LengthSqr() const
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

inline BvVec BvVec::Project(CRBvVec normal) const
{
	return BvVec(VectorProject(m128, normal.m128));
}

inline BvVec BvVec::Lerp(CRBvVec toVec, f32 t) const
{
	return BvVec(VectorLerp(m128, toVec.m128, t));
}

inline BvVec BvVec::Reflect(CRBvVec normal) const
{
	return BvVec(VectorReflection(m128, normal.m128));
}

inline BvVec BvVec::Refract(CRBvVec normal, f32 eta) const
{
	return BvVec(VectorRefraction(m128, normal.m128, eta));
}

inline const f32& BvVec::operator[](unsigned int index) const
{
	return arr[index];
}

inline f32& BvVec::operator[](unsigned int index)
{
	return arr[index];
}

inline BvVec operator+(CRBvVec lhs, CRBvVec rhs)
{
	return lhs.Add(rhs);
}

inline BvVec operator-(CRBvVec lhs, CRBvVec rhs)
{
	return lhs.Sub(rhs);
}

inline BvVec operator*(CRBvVec lhs, CRBvVec rhs)
{
	return lhs.Mul(rhs);
}

inline BvVec operator/(CRBvVec lhs, CRBvVec rhs)
{
	return lhs.Div(rhs);
}

inline BvVec operator+(CRBvVec lhs, f32 val)
{
	return lhs.Add(val);
}

inline BvVec operator-(CRBvVec lhs, f32 val)
{
	return lhs.Sub(val);
}

inline BvVec operator*(CRBvVec lhs, f32 val)
{
	return lhs.Mul(val);
}

inline BvVec operator/(CRBvVec lhs, f32 val)
{
	return lhs.Div(val);
}

inline BvVec operator+(f32 val, CRBvVec rhs)
{
	return rhs.Add(val);
}

inline BvVec operator-(f32 val, CRBvVec rhs)
{
	return BvVec(val).Sub(rhs);
}

inline BvVec operator*(f32 val, CRBvVec rhs)
{
	return rhs.Mul(val);
}

inline BvVec operator/(f32 val, CRBvVec rhs)
{
	return rhs.Div(val);
}

inline BvVec operator*(CRBvVec lhs, const BvMat& rhs)
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

inline BvVec& BvVec::operator+=(CRBvVec v)
{
	m128 = VectorAdd(m128, v.m128);

	return *this;
}

inline BvVec& BvVec::operator-=(CRBvVec v)
{
	m128 = VectorSub(m128, v.m128);

	return *this;
}

inline BvVec& BvVec::operator*=(CRBvVec v)
{
	m128 = VectorMul(m128, v.m128);

	return *this;
}

inline BvVec& BvVec::operator/=(CRBvVec v)
{
	m128 = VectorDiv(m128, v.m128);

	return *this;
}

inline BvVec& BvVec::operator+=(f32 val)
{
	m128 = VectorAdd(m128, val);

	return *this;
}

inline BvVec& BvVec::operator-=(f32 val)
{
	m128 = VectorSub(m128, val);

	return *this;
}

inline BvVec& BvVec::operator*=(f32 val)
{
	m128 = VectorMul(m128, val);

	return *this;
}

inline BvVec& BvVec::operator/=(f32 val)
{
	m128 = VectorDiv(m128, val);

	return *this;
}

inline BvVec& BvVec::operator*=(const BvMat& m)
{
	m128 = VectorMul(m128, m.mat);

	return *this;
}

inline BvVec operator==(CRBvVec lhs, CRBvVec rhs)
{
	return BvVec(VectorEqual(lhs.m128, rhs.m128));
}

inline BvVec operator<(CRBvVec lhs, CRBvVec rhs)
{
	return BvVec(VectorLess(lhs.m128, rhs.m128));
}

inline BvVec operator<=(CRBvVec lhs, CRBvVec rhs)
{
	return BvVec(VectorLessEqual(lhs.m128, rhs.m128));
}

inline BvVec operator>(CRBvVec lhs, CRBvVec rhs)
{
	return BvVec(VectorGreater(lhs.m128, rhs.m128));
}

inline BvVec operator>=(CRBvVec lhs, CRBvVec rhs)
{
	return BvVec(VectorGreaterEqual(lhs.m128, rhs.m128));
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