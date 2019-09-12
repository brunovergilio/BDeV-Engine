#pragma once


#include "../../BvFloatTypes.h"


class BvVec;
class BvQuat;


class BvMat
{
public:
	BvMat();

	BvMat(const BvMat & rhs);
	BvMat(BvMat && rhs);
	BvMat & operator=(const BvMat & rhs);
	BvMat & operator=(BvMat && rhs);

	explicit BvMat(const Float2 & r1, const Float2 & r2);
	explicit BvMat(const Float3 & r1, const Float3 & r2, const Float3 & r3, const Float3 & r4 = Float3());
	explicit BvMat(const Float4 & r1, const Float4 & r2, const Float4 & r3, const Float4 & r4);
	explicit BvMat(const BvVec & r1, const BvVec & r2, const BvVec & r3, const BvVec & r4 = BvVec(0.0f, 0.0f, 0.0f, 1.0f));
	explicit BvMat(BvFastVec r1, BvFastVec r2, BvFastVec r3, BvFastVec r4);
	explicit BvMat(BvFastMat m);
	explicit BvMat(const BvQuat & q);

	void Set(const Float2 & r1, const Float2 & r2);
	void Set(const Float3 & r1, const Float3 & r2, const Float3 & r3, const Float3 & r4 = Float3());
	void Set(const Float4 & r1, const Float4 & r2, const Float4 & r3, const Float4 & r4);
	void Set(const BvVec & r1, const BvVec & r2, const BvVec & r3, const BvVec & r4);
	void Set(BvFastVec r1, BvFastVec r2, BvFastVec r3, BvFastVec r4);
	void Set(const BvQuat & q);

	BvVec Get(const unsigned int row) const;

	Float22 AsFloat22() const;
	Float33 AsFloat33() const;
	Float43 AsFloat43() const;
	Float44 AsFloat44() const;

	BvQuat ToQuaternion() const;
	BvMat FromQuaternion(const BvQuat & q) const;

	BvMat Add(const BvMat & m) const;
	BvMat Sub(const BvMat & m) const;
	BvMat Mul(const BvMat & m) const;

	BvMat Add(const float val) const;
	BvMat Sub(const float val) const;
	BvMat Mul(const float val) const;
	BvMat Div(const float val) const;
	
	BvMat Mul(const BvQuat & q) const;

	BvMat Identity() const;
	BvMat Transpose() const;
	float Determinant() const;
	BvMat Inverse() const;

	BvMat Scale(const float x, const float y, const float z) const;

	BvMat RotX(const float rad) const;
	BvMat RotY(const float rad) const;
	BvMat RotZ(const float rad) const;
	BvMat RotAxis(const BvVec & axis, const float rad) const;

	BvMat Translate(const float x, const float y, const float z) const;

	BvMat LookAt(const BvVec & eyePos, const BvVec & dirVec, const BvVec & upVec) const;

	BvMat PerspectiveLH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const;
	BvMat PerspectiveRH_DX(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const;

	BvMat PerspectiveLH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const;
	BvMat PerspectiveRH_GL(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const;

	BvMat PerspectiveLH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const;
	BvMat PerspectiveRH_VK(const float nearZ, const float farZ, const float aspectRatio, const float fovY) const;

	BvMat OrthoLH_DX(const float right, const float left, const float top, const float bottom, const float nearZ, const float farZ) const;
	BvMat OrthoRH_DX(const float right, const float left, const float top, const float bottom, const float nearZ, const float farZ) const;
	BvMat OrthoLH_GL(const float right, const float left, const float top, const float bottom, const float nearZ, const float farZ) const;
	BvMat OrthoRH_GL(const float right, const float left, const float top, const float bottom, const float nearZ, const float farZ) const;
	BvMat OrthoLH_VK(const float right, const float left, const float top, const float bottom, const float nearZ, const float farZ) const;
	BvMat OrthoRH_VK(const float right, const float left, const float top, const float bottom, const float nearZ, const float farZ) const;

	const BvVec & operator[](const unsigned int index) const;
	BvVec & operator[](const unsigned int index);

	friend BvMat operator+(const BvMat & lhs, const BvMat & rhs);
	friend BvMat operator-(const BvMat & lhs, const BvMat & rhs);
	friend BvMat operator*(const BvMat & lhs, const BvMat & rhs);

	friend BvMat operator+(const BvMat & lhs, const float val);
	friend BvMat operator-(const BvMat & lhs, const float val);
	friend BvMat operator*(const BvMat & lhs, const float val);
	friend BvMat operator/(const BvMat & lhs, const float val);

	friend BvMat operator+(const float val, const BvMat & rhs);
	friend BvMat operator-(const float val, const BvMat & rhs);
	friend BvMat operator*(const float val, const BvMat & rhs);

	BvMat operator+();
	BvMat operator-();

	BvMat & operator+=(const BvMat & m);
	BvMat & operator-=(const BvMat & m);
	BvMat & operator*=(const BvMat & m);

	BvMat & operator+=(const float val);
	BvMat & operator-=(const float val);
	BvMat & operator*=(const float val);
	BvMat & operator/=(const float val);

	friend BvMat operator*(const BvQuat & lhs, const BvMat & rhs);
	friend BvMat operator*(const BvMat & lhs, const BvQuat & rhs);

public:
	union
	{
		BvFastMat mat;
		Float44 m44;
		BvVec arr[4];
		struct
		{
			float m00, m01, m02, m03,
				m10, m11, m12, m13,
				m20, m21, m22, m23,
				m30, m31, m32, m33;
		};
	};
};


#include "BvMat.inl"