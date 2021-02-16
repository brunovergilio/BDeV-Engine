#pragma once


#include "../../BvFloatTypes.h"


class BvMat;
class BvQuat;


class BvVec
{
public:
	BvVec();

	BvVec(const BvVec & rhs);
	BvVec(BvVec && rhs);
	BvVec & operator=(const BvVec & rhs);
	BvVec & operator=(BvVec && rhs);

	explicit BvVec(const float x, const float y, const float z, const float w = 0.0f);
	explicit BvVec(const float s);
	explicit BvVec(const Float2 & v);
	explicit BvVec(const Float3 & v);
	explicit BvVec(const Float4 & v);
	explicit BvVec(const BvFastVec & m128);

	void Set(const float x, const float y, const float z, const float w = 0.0f);
	void Set(const float s);
	void Set(const Float2 & v);
	void Set(const Float3 & v);
	void Set(const Float4 & v);
	void Set(const BvFastVec & m128);
	void SetZero();

	float GetX() const;
	float GetY() const;
	float GetZ() const;
	float GetW() const;

	Float2 AsFloat2() const;
	Float3 AsFloat3() const;
	Float4 AsFloat4() const;

	BvQuat ToQuat() const;

	BvVec ReplicateX() const;
	BvVec ReplicateY() const;
	BvVec ReplicateZ() const;
	BvVec ReplicateW() const;
	template<unsigned int x, unsigned int y, unsigned int z, unsigned int w> BvVec Permute() const;

	BvVec Max(const BvVec & v) const;
	BvVec Min(const BvVec & v) const;
	BvVec Abs() const;
	BvVec Floor() const;
	BvVec Ceil() const;
	BvVec Round() const;
	BvVec Zero() const;

	BvVec IsNearlyEqual(const BvVec & v, const float eps = BvEpsilon) const;
	BvVec IsZero(const float eps = BvEpsilon) const;

	BvVec Add(const BvVec & v) const;
	BvVec Sub(const BvVec & v) const;
	BvVec Mul(const BvVec & v) const;
	BvVec Div(const BvVec & v) const;

	BvVec Add(const float val) const;
	BvVec Sub(const float val) const;
	BvVec Mul(const float val) const;
	BvVec Div(const float val) const;

	BvVec Mul(const BvMat & m) const;

	float Dot(const BvVec & v) const;
	BvVec Cross(const BvVec & v) const;

	float Length() const;
	float LengthSqr() const;

	void Normalize();
	BvVec Normal() const;

	BvVec Project(const BvVec & normal) const;
	BvVec Lerp(const BvVec & toVec, const float t) const;
	BvVec Reflect(const BvVec & normal);

	const float & operator[](const unsigned int index) const;
	float & operator[](const unsigned int index);

	friend BvVec operator+(const BvVec & lhs, const BvVec & rhs);
	friend BvVec operator-(const BvVec & lhs, const BvVec & rhs);
	friend BvVec operator*(const BvVec & lhs, const BvVec & rhs);
	friend BvVec operator/(const BvVec & lhs, const BvVec & rhs);

	friend BvVec operator+(const BvVec & lhs, const float val);
	friend BvVec operator-(const BvVec & lhs, const float val);
	friend BvVec operator*(const BvVec & lhs, const float val);
	friend BvVec operator/(const BvVec & lhs, const float val);

	friend BvVec operator+(const float val, const BvVec & rhs);
	friend BvVec operator-(const float val, const BvVec & rhs);
	friend BvVec operator*(const float val, const BvVec & rhs);
	friend BvVec operator/(const float val, const BvVec & rhs);

	friend BvVec operator*(const BvVec & lhs, const BvMat & rhs);

	BvVec operator+() const;
	BvVec operator-() const;

	BvVec & operator+=(const BvVec & v);
	BvVec & operator-=(const BvVec & v);
	BvVec & operator*=(const BvVec & v);
	BvVec & operator/=(const BvVec & v);

	BvVec & operator+=(const float val);
	BvVec & operator-=(const float val);
	BvVec & operator*=(const float val);
	BvVec & operator/=(const float val);

	BvVec & operator*=(const BvMat & m);

	friend BvVec operator==(const BvVec & lhs, const BvVec & rhs);
	friend BvVec operator< (const BvVec & lhs, const BvVec & rhs);
	friend BvVec operator<=(const BvVec & lhs, const BvVec & rhs);
	friend BvVec operator> (const BvVec & lhs, const BvVec & rhs);
	friend BvVec operator>=(const BvVec & lhs, const BvVec & rhs);

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
		BvFastVec m128;
		Float2 v2;
		Float3 v3;
		Float4 v4;
		float arr[4];
		struct
		{
			float x, y, z, w;
		};
	};
};


#include "BvVec.inl"