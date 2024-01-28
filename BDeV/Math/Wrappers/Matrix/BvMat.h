#pragma once


#include "../../BvFloatTypes.h"


class BvVec;
class BvQuat;


class BvMat
{
public:
	BvMat();

	BvMat(const BvMat& rhs);
	BvMat(BvMat&& rhs) noexcept;
	BvMat& operator=(const BvMat& rhs);
	BvMat& operator=(BvMat&& rhs) noexcept;

	explicit BvMat(const Float2& r1, const Float2& r2);
	explicit BvMat(const Float3& r1, const Float3& r2, const Float3& r3, const Float3& r4 = Float3());
	explicit BvMat(const Float4& r1, const Float4& r2, const Float4& r3, const Float4& r4);
	explicit BvMat(const Float22& m);
	explicit BvMat(const Float33& m);
	explicit BvMat(const Float43& m);
	explicit BvMat(const Float44& m);
	explicit BvMat(CRBvVec r1, CRBvVec r2, CRBvVec r3, CRBvVec r4 = BvVec(0.0f, 0.0f, 0.0f, 1.0f));
	explicit BvMat(crvf32 r1, crvf32 r2, crvf32 r3, crvf32 r4);
	explicit BvMat(const mf32& m);
	explicit BvMat(const BvQuat& q);

	void Set(const Float2& r1, const Float2& r2);
	void Set(const Float3& r1, const Float3& r2, const Float3& r3, const Float3& r4 = Float3());
	void Set(const Float4& r1, const Float4& r2, const Float4& r3, const Float4& r4);
	void Set(CRBvVec r1, CRBvVec r2, CRBvVec r3, CRBvVec r4);
	void Set(crvf32 r1, crvf32 r2, crvf32 r3, crvf32 r4);
	void Set(const BvQuat& q);

	BvVec Get(unsigned int row) const;

	Float22 AsFloat22() const;
	Float33 AsFloat33() const;
	Float43 AsFloat43() const;

	BvQuat ToQuaternion() const;
	BvMat FromQuaternion(const BvQuat& q) const;

	BvMat Add(const BvMat& m) const;
	BvMat Sub(const BvMat& m) const;
	BvMat Mul(const BvMat& m) const;

	BvMat Add(f32 val) const;
	BvMat Sub(f32 val) const;
	BvMat Mul(f32 val) const;
	BvMat Div(f32 val) const;
	
	BvMat Mul(const BvQuat& q) const;

	BvMat Identity() const;
	BvMat Transpose() const;
	f32 Determinant() const;
	BvMat Inverse() const;

	BvMat Scale(f32 x, f32 y, f32 z) const;

	BvMat RotX(f32 rad) const;
	BvMat RotY(f32 rad) const;
	BvMat RotZ(f32 rad) const;
	BvMat RotAxis(CRBvVec axis, f32 rad) const;

	BvMat Translate(f32 x, f32 y, f32 z) const;

	BvMat LookAt(CRBvVec eyePos, CRBvVec dirVec, CRBvVec upVec) const;

	BvMat PerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const;
	BvMat PerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const;

	BvMat PerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const;
	BvMat PerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const;

	BvMat PerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const;
	BvMat PerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY) const;

	BvMat OrthoLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) const;
	BvMat OrthoRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) const;
	BvMat OrthoLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) const;
	BvMat OrthoRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) const;
	BvMat OrthoLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) const;
	BvMat OrthoRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32 nearZ, f32 farZ) const;

	const BvVec& operator[](unsigned int index) const;
	BvVec& operator[](unsigned int index);

	friend BvMat operator+(const BvMat& lhs, const BvMat& rhs);
	friend BvMat operator-(const BvMat& lhs, const BvMat& rhs);
	friend BvMat operator*(const BvMat& lhs, const BvMat& rhs);

	friend BvMat operator+(const BvMat& lhs, f32 val);
	friend BvMat operator-(const BvMat& lhs, f32 val);
	friend BvMat operator*(const BvMat& lhs, f32 val);
	friend BvMat operator/(const BvMat& lhs, f32 val);

	friend BvMat operator+(f32 val, const BvMat& rhs);
	friend BvMat operator-(f32 val, const BvMat& rhs);
	friend BvMat operator*(f32 val, const BvMat& rhs);

	BvMat operator+();
	BvMat operator-();

	BvMat& operator+=(const BvMat& m);
	BvMat& operator-=(const BvMat& m);
	BvMat& operator*=(const BvMat& m);

	BvMat& operator+=(f32 val);
	BvMat& operator-=(f32 val);
	BvMat& operator*=(f32 val);
	BvMat& operator/=(f32 val);

	friend BvMat operator*(const BvQuat& lhs, const BvMat& rhs);
	friend BvMat operator*(const BvMat& lhs, const BvQuat& rhs);

public:
	union
	{
		mf32 mat;
		Float44 m44;
		BvVec arr[4];
		struct
		{
			f32 m00, m01, m02, m03,
				m10, m11, m12, m13,
				m20, m21, m22, m23,
				m30, m31, m32, m33;
		};
	};
};


#include "BvMat.inl"