#pragma once


#include "../../BvFloatTypes.h"


class BvVec;
class BvMat;


class BvQuat
{
public:
	BvQuat();

	BvQuat(const BvQuat & rhs);
	BvQuat(BvQuat && rhs);
	BvQuat & operator=(const BvQuat & rhs);
	BvQuat & operator=(BvQuat && rhs);

	explicit BvQuat(const float x, const float y, const float z, const float w);
	explicit BvQuat(const Float4 & v);
	explicit BvQuat(const BvFastQuat & m128);
	explicit BvQuat(const BvVec & v);
	explicit BvQuat(const BvVec & axis, const float rad);
	explicit BvQuat(const BvMat & m);

	void Set(const float x, const float y, const float z, const float w);
	void Set(const Float4 & v);
	void Set(const BvFastQuat & q);
	void Set(const BvVec & v);
	void Set(const BvVec & axis, const float rad);
	void Set(const BvMat & m);
	void SetZero();

	float GetX() const;
	float GetY() const;
	float GetZ() const;
	float GetW() const;

	Float4 AsFloat4() const;

	BvQuat Identity() const;

	BvQuat Mul(const BvVec & v) const;
	BvQuat Mul(const BvMat & m) const;
	BvQuat Mul(const BvQuat & q) const;

	float Dot(const BvQuat & q) const;

	float Length() const;
	float LengthSqr() const;

	void Normalize();
	BvQuat Normal() const;

	BvQuat Conjugate() const;
	BvQuat Inverse() const;

	BvQuat RotAxis(const BvVec & axis, const float rad) const;

	float Angle() const;

	BvQuat Slerp(const BvQuat & toQuat, const float t) const;

	BvMat ToMatrix() const;
	BvQuat FromMatrix(const BvMat & m) const;

	BvQuat QVQC(const BvVec & v) const;
	BvQuat QCVQ(const BvVec & v) const;

	const float & operator[](const unsigned int index) const;
	float & operator[](const unsigned int index);

	friend BvQuat operator*(const BvQuat & lhs, const BvVec & rhs);
	friend BvQuat operator*(const BvVec & lhs, const BvQuat & rhs);
	friend BvQuat operator*(const BvQuat & lhs, const BvQuat & rhs);
	BvQuat & operator*=(const BvQuat & q);
	BvQuat & operator*=(const BvVec & v);
	BvQuat & operator*=(const BvMat & m);

public:
	union
	{
		BvFastQuat m128;
		Float4 v4;
		float arr[4];
		struct
		{
			float x, y, z, w;
		};
	};
};


#include "BvQuat.inl"