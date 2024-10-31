#pragma once


#include "BvMath.h"


struct BvRay
{
	BvRay() = default;
	BvRay(const Float3& pos, const Float3& dir)
		: m_Pos(pos), m_Dir(dir) {}

	Float3 m_Pos;
	Float3 m_Dir;
};


struct BvTriangle
{
	BvTriangle() = default;
	BvTriangle(const Float3& v0, const Float3& v1, const Float3& v2)
		: m_V0(v0), m_V1(v1), m_V2(v2) {}

	Float3 m_V0;
	Float3 m_V1;
	Float3 m_V2;
};


struct BvSphere
{
	BvSphere() = default;
	BvSphere(const Float3& center, float radius = 1.0f)
		: m_Center(center), m_Radius(radius) {}

	Float3 m_Center;
	float m_Radius;
};


struct BvPlane
{
	BvPlane() = default;
	BvPlane(const Float3& normal, float distance)
		: m_Normal(normal), m_Distance(distance) {}
	BvPlane(const Float3& normal, const Float3& point)
		: m_Normal(normal), m_Distance(-VectorGetX(Vector3Dot(Load(point), Load(normal)))) {}

	Float3 m_Normal;
	float m_Distance;
};


struct BvAABB
{
	BvAABB() = default;
	BvAABB(const Float3& center, const Float3& extents)
		: m_Center(center), m_Extents(extents) {}

	Float3 m_Center;
	Float3 m_Extents;
};


struct BvOBB
{
	BvOBB() = default;
	BvOBB(const Float3& center, const Float3& extents, const Float4& rotation)
		: m_Center(center), m_Extents(extents), m_Rotation(rotation) {}
	explicit BvOBB(const BvAABB& aabb)
		: m_Center(aabb.m_Center), m_Extents(aabb.m_Extents), m_Rotation(0.0f, 0.0f, 0.0f, 1.0f) {}

	Float3 m_Center;
	Float3 m_Extents;
	Float4 m_Rotation;
};


struct BvFrustum
{
	BvFrustum() = default;
	BvFrustum(const BvPlane& p0, const BvPlane& p1, const BvPlane& p2,
		const BvPlane& p3, const BvPlane& p4, const BvPlane& p5)
	{
		m_Planes[0] = p0;
		m_Planes[1] = p1;
		m_Planes[2] = p2;
		m_Planes[3] = p3;
		m_Planes[4] = p4;
		m_Planes[5] = p5;
	}

	BvPlane m_Planes[6];
};


struct BvCapsule
{
	BvCapsule() = default;
	BvCapsule(const Float3& p0, const Float3& p1, float radius = 1.0f)
		: m_P1(p0), m_P2(p1), m_Radius(radius) {}

	Float3 m_P1;
	Float3 m_P2;
	float m_Radius;
};