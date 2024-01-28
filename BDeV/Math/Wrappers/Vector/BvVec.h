#pragma once


#include "../../BvFloatTypes.h"


class BvMat;
class BvQuat;


class BvVec
{
public:
	BvVec();

	BvVec(const BvVec& rhs);
	BvVec(BvVec&& rhs) noexcept;
	BvVec& operator=(const BvVec& rhs);
	BvVec& operator=(BvVec&& rhs) noexcept;

	explicit BvVec(f32 x, f32 y, f32 z, f32 w = 0.0f);
	explicit BvVec(f32 s);
	explicit BvVec(const Float2& v);
	explicit BvVec(const Float3& v);
	explicit BvVec(const Float4& v);
	explicit BvVec(crvf32 v);

	void Set(f32 x, f32 y, f32 z, f32 w = 0.0f);
	void Set(f32 s);
	void Set(const Float2& v);
	void Set(const Float3& v);
	void Set(const Float4& v);
	void Set(crvf32 v);
	void SetZero();

	f32 GetX() const;
	f32 GetY() const;
	f32 GetZ() const;
	f32 GetW() const;

	BvQuat ToQuat() const;

	BvVec ReplicateX() const;
	BvVec ReplicateY() const;
	BvVec ReplicateZ() const;
	BvVec ReplicateW() const;
	template<unsigned int X, unsigned int Y, unsigned int Z, unsigned int W> BvVec Permute() const;

	BvVec Max(CRBvVec v) const;
	BvVec Min(CRBvVec v) const;
	BvVec Abs() const;
	BvVec Floor() const;
	BvVec Ceil() const;
	BvVec Round() const;
	BvVec Zero() const;

	BvVec IsNearlyEqual(CRBvVec v, f32 eps = kEpsilon) const;
	BvVec IsZero(f32 eps = kEpsilon) const;

	BvVec Add(CRBvVec v) const;
	BvVec Sub(CRBvVec v) const;
	BvVec Mul(CRBvVec v) const;
	BvVec Div(CRBvVec v) const;

	BvVec Add(f32 val) const;
	BvVec Sub(f32 val) const;
	BvVec Mul(f32 val) const;
	BvVec Div(f32 val) const;

	BvVec Mul(const BvMat& m) const;

	f32 Dot(CRBvVec v) const;
	BvVec Cross(CRBvVec v) const;

	f32 Length() const;
	f32 LengthSqr() const;

	void Normalize();
	BvVec Normal() const;

	BvVec Project(CRBvVec normal) const;
	BvVec Lerp(CRBvVec toVec, f32 t) const;
	BvVec Reflect(CRBvVec normal) const;
	BvVec Refract(CRBvVec normal, f32 eta) const;

	const f32& operator[](unsigned int index) const;
	f32& operator[](unsigned int index);

	friend BvVec operator+(CRBvVec lhs, CRBvVec rhs);
	friend BvVec operator-(CRBvVec lhs, CRBvVec rhs);
	friend BvVec operator*(CRBvVec lhs, CRBvVec rhs);
	friend BvVec operator/(CRBvVec lhs, CRBvVec rhs);

	friend BvVec operator+(CRBvVec lhs, f32 val);
	friend BvVec operator-(CRBvVec lhs, f32 val);
	friend BvVec operator*(CRBvVec lhs, f32 val);
	friend BvVec operator/(CRBvVec lhs, f32 val);

	friend BvVec operator+(f32 val, CRBvVec rhs);
	friend BvVec operator-(f32 val, CRBvVec rhs);
	friend BvVec operator*(f32 val, CRBvVec rhs);
	friend BvVec operator/(f32 val, CRBvVec rhs);

	friend BvVec operator*(CRBvVec lhs, const BvMat& rhs);

	BvVec operator+() const;
	BvVec operator-() const;

	BvVec& operator+=(CRBvVec v);
	BvVec& operator-=(CRBvVec v);
	BvVec& operator*=(CRBvVec v);
	BvVec& operator/=(CRBvVec v);

	BvVec& operator+=(f32 val);
	BvVec& operator-=(f32 val);
	BvVec& operator*=(f32 val);
	BvVec& operator/=(f32 val);

	BvVec& operator*=(const BvMat& m);

	friend BvVec operator==(CRBvVec lhs, CRBvVec rhs);
	friend BvVec operator< (CRBvVec lhs, CRBvVec rhs);
	friend BvVec operator<=(CRBvVec lhs, CRBvVec rhs);
	friend BvVec operator> (CRBvVec lhs, CRBvVec rhs);
	friend BvVec operator>=(CRBvVec lhs, CRBvVec rhs);

	bool AllTrue() const;
	bool AllFalse() const;
	bool AnyTrue() const;
	bool AnyFalse() const;
	template<bool x = true, bool y = true, bool z = true, bool w = false> bool True() const;
	template<bool x = true, bool y = true, bool z = true, bool w = false> bool False() const;
	unsigned int GetMask() const;

public:
	union
	{
		vf32 m128;
		Float2 v2;
		Float3 v3;
		Float4 v4;
		f32 arr[4];
		struct
		{
			f32 x, y, z, w;
		};
	};
};


#include "BvVec.inl"