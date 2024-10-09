#pragma once


#include "BDeV/Core/Math/BvFloatTypes.h"


class BvVec;
class BvMat;


class BvQuat
{
public:
	BvQuat();

	BvQuat(const BvQuat& rhs);
	BvQuat(BvQuat&& rhs) noexcept;
	BvQuat& operator=(const BvQuat& rhs);
	BvQuat& operator=(BvQuat&& rhs) noexcept;

	explicit BvQuat(f32 x, f32 y, f32 z, f32 w);
	explicit BvQuat(const Float4& v);
	explicit BvQuat(crqf32 m128);
	explicit BvQuat(CRBvVec v);
	explicit BvQuat(CRBvVec axis, f32 rad);
	explicit BvQuat(const BvMat& m);

	void Set(f32 x, f32 y, f32 z, f32 w);
	void Set(const Float4& v);
	void Set(crqf32 q);
	void Set(CRBvVec v);
	void Set(CRBvVec axis, f32 rad);
	void Set(const BvMat& m);
	void SetZero();

	f32 GetX() const;
	f32 GetY() const;
	f32 GetZ() const;
	f32 GetW() const;

	BvQuat Identity() const;

	BvQuat Mul(CRBvVec v) const;
	BvQuat Mul(const BvMat& m) const;
	BvQuat Mul(CRBvQuat q) const;

	f32 Dot(CRBvQuat q) const;

	f32 Length() const;
	f32 LengthSqr() const;

	void Normalize();
	BvQuat Normal() const;

	BvQuat Conjugate() const;
	BvQuat Inverse() const;

	BvQuat RotAxis(CRBvVec axis, f32 rad) const;

	f32 Angle() const;

	BvQuat Slerp(CRBvQuat toQuat, f32 t) const;

	BvMat ToMatrix() const;
	BvQuat FromMatrix(const BvMat& m) const;

	BvVec ToVector() const;

	BvQuat QVQC(CRBvVec v) const;
	BvQuat QCVQ(CRBvVec v) const;

	const f32& operator[](unsigned int index) const;
	f32& operator[](unsigned int index);

	friend BvQuat operator*(CRBvQuat lhs, CRBvVec rhs);
	friend BvQuat operator*(CRBvVec lhs, CRBvQuat rhs);
	friend BvQuat operator*(CRBvQuat lhs, CRBvQuat rhs);
	BvQuat& operator*=(CRBvQuat q);
	BvQuat& operator*=(CRBvVec v);
	BvQuat& operator*=(const BvMat& m);

public:
	union
	{
		qf32 m128;
		Float4 v4;
		f32 arr[4];
		struct
		{
			f32 x, y, z, w;
		};
	};
};


#include "BvQuat.inl"