#pragma once


#include "Wrappers/Vector/BvVec.h"
#include "Wrappers/Matrix/BvMat.h"
#include "Wrappers/Quaternion/BvQuat.h"


struct BvRay
{
	BvRay() = default;
	BvRay(BvVec pos, BvVec dir)
		: m_Pos(pos), m_Dir(dir) {}

	BvVec m_Pos;
	BvVec m_Dir;
};


struct BvTriangle
{
	BvTriangle() = default;
	BvTriangle(BvVec v0, BvVec v1, BvVec v2)
		: m_V0(v0), m_V1(v1), m_V2(v2) {}

	BvVec m_V0;
	BvVec m_V1;
	BvVec m_V2;
};


struct BvSphere
{
	BvSphere() = default;
	BvSphere(BvVec center, float radius = 1.0f)
		: m_Center(center), m_Radius(radius) {}

	BvVec m_Center;
	float m_Radius;
};


struct BvPlane
{
	BvPlane() = default;
	BvPlane(BvVec normal, float distance)
		: m_Normal(normal), m_Distance(distance) {}
	BvPlane(BvVec normal, BvVec point)
		: m_Normal(normal), m_Distance(-(point.Dot(normal))) {}

	BvVec m_Normal;
	float m_Distance;
};


struct BvAABB
{
	BvAABB() = default;
	BvAABB(BvVec min, BvVec max)
		: m_Min(min), m_Max(max) {}

	BvVec m_Min;
	BvVec m_Max;
};


struct BvOBB
{
	BvOBB() = default;
	BvOBB(BvVec center, BvVec extents, BvQuat rotation = BvQuat())
		: m_Center(center), m_Extents(extents), m_Rotation(rotation) {}
	explicit BvOBB(const BvAABB& aabb)
		: m_Center((aabb.m_Min + aabb.m_Max) * 0.5f), m_Extents((aabb.m_Max - aabb.m_Min) * 0.5f) {}

	BvVec m_Center;
	BvVec m_Extents;
	BvQuat m_Rotation;
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
	BvCapsule(BvVec p0, BvVec p1, float radius = 1.0f)
		: m_P0(p0), m_P1(p1), m_Radius(radius) {}

	BvVec m_P0;
	BvVec m_P1;
	float m_Radius;
};