#include "BvIntersections.h"

f32 SignedDistanceToPlane(const BvPlane& plane, BvVec point)
{
	return plane.m_Normal.Dot(point) + plane.m_Distance;
}

void ProjectOnAxis(BvVec axis, const BvVec* pPoints, u32 pointCount, f32& min, f32& max)
{
	min = axis.Dot(pPoints[0]);
	max = min;
	for (auto i = 1; i < pointCount; ++i)
	{
		auto proj = axis.Dot(pPoints[i]);
		if (proj < min)
		{
			min = proj;
		}
		if (proj > max)
		{
			max = proj;
		}
	}
}

bool OverlapOnAxis(BvVec axis, const BvVec* pPointsA, u32 pointCountA, const BvVec* pPointsB, u32 pointCountB)
{
	f32 minA, maxA, minB, maxB;
	ProjectOnAxis(axis, pPointsA, pointCountA, minA, maxA);
	ProjectOnAxis(axis, pPointsB, pointCountB, minB, maxB);

	return (maxA >= minB) && (maxB >= minA);
}


bool IsPointInSphere(const BvSphere& sphere, BvVec p)
{
	return (sphere.m_Center - p).LengthSqr() <= sphere.m_Radius * sphere.m_Radius;
}


f32 SqrDistanceFromPointToLineSegment(BvVec p, BvVec v0, BvVec v1)
{
	auto line = v1 - v0;
	auto toPoint = p - v0;
	auto t = toPoint.Dot(line) / line.Dot(line);
	if (t < 0.0f)
	{
		t = 0.0f;
	}
	else if (t < 1.0f)
	{
		t = 1.0f;
	}

	return (v0 + line * t).LengthSqr();
}


bool Intersects(const BvRay& ray, const BvTriangle& triangle, f32& t, f32& u, f32& v)
{
	auto e1 = triangle.m_V1 - triangle.m_V0;
	auto e2 = triangle.m_V2 - triangle.m_V0;

	auto p = ray.m_Dir.Cross(e2);
	auto det = e1.Dot(p);

	if (det > -kEpsilon && det < kEpsilon)
	{
		return false;
	}

	auto tv = ray.m_Pos - triangle.m_V0;

	auto invDet = 1.0f / det;
	u = p.Dot(tv) * invDet;
	if (u < 0.0f || u > 1.0f)
	{
		return false;
	}

	auto q = tv.Cross(e1);
	v = ray.m_Dir.Dot(q) * invDet;
	if (v < 0.0f || v > 1.0f)
	{
		return false;
	}

	t = q.Dot(e2) * invDet;

	return t > kEpsilon;
}

bool Intersects(const BvRay& ray, const BvSphere& sphere, f32& t)
{
	auto rayToCenter = sphere.m_Center - ray.m_Pos;
	auto tca = rayToCenter.Dot(ray.m_Dir);
	if (tca < 0.0f)
	{
		return false;
	}

	auto d2 = rayToCenter.Dot(rayToCenter) - tca * tca;
	auto rad2 = sphere.m_Radius * sphere.m_Radius;
	if (d2 > rad2)
	{
		return false;
	}

	auto thc = sqrtf(rad2 - d2);
	t = tca - thc;

	return true;
}

bool Intersects(const BvRay& ray, const BvPlane& plane, f32& t)
{
	auto denom = ray.m_Dir.Dot(plane.m_Normal);
	auto num = ray.m_Pos.Dot(plane.m_Normal) + plane.m_Distance;
	if (fabs(denom) > kEpsilon)
	{
		t = -num / denom;
	}
	else if (fabs(num) <= kEpsilon)
	{
		t = 0.0f;
	}

	return t >= 0.0f;
}

bool Intersects(const BvRay& ray, const BvAABB& aabb, f32& tMin, f32& tMax)
{
	tMin = kF32Min;
	tMax = kF32Max;

	for (auto i = 0; i < 3; ++i)
	{
		f32 invD = 1.0f / ray.m_Dir[i];

		auto t1 = (aabb.m_Min[i] - ray.m_Pos[i]) * invD;
		auto t2 = (aabb.m_Max[i] - ray.m_Pos[i]) * invD;

		if (invD < 0.0f)
		{
			std::swap(t1, t2);
		}

		tMin = std::max(tMin, t1);
		tMax = std::min(tMax, t2);

		if (tMin > tMax)
		{
			return false;
		}
	}

	return true;
}

bool Intersects(const BvRay& ray, const BvOBB& obb, f32& tMin, f32& tMax)
{
	tMin = kF32Min;
	tMax = kF32Max;

	BvVec obbToRay = ray.m_Pos - obb.m_Center;
	BvMat rot(obb.m_Rotation);

	for (auto i = 0; i < 3; ++i)
	{
		f32 e = rot[i].Dot(obbToRay);
		f32 f = ray.m_Dir.Dot(rot[i]);

		if (fabs(f) > kEpsilon)
		{
			auto invF = 1.0f / f;
			auto t1 = (e + obb.m_Extents[i]) * invF;
			auto t2 = (e - obb.m_Extents[i]) * invF;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax)
			{
				return false;
			}
		}
		else if (-e - obb.m_Extents[i] > 0.0f || -e + obb.m_Extents[i] < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool Intersects(const BvRay& ray, const BvFrustum& frustum, f32& tMin, f32& tMax)
{
	tMin = kF32Min;
	tMax = kF32Max;

	for (auto& plane : frustum.m_Planes)
	{
		auto denom = ray.m_Dir.Dot(plane.m_Normal);
		auto num = ray.m_Pos.Dot(plane.m_Normal) + plane.m_Distance;
		if (fabs(denom) > kEpsilon)
		{
			auto t = -num / denom;

			if (denom < 0.0f)
			{
				tMin = std::max(tMin, t);
			}
			else
			{
				tMax = std::min(tMax, t);
			}

			if (tMin > tMax)
			{
				return false;
			}
		}
		else if (num > 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool Intersects(const BvRay& ray, const BvCapsule& capsule, f32& t)
{
	auto capDir = (capsule.m_P1 - capsule.m_P0);
	capDir.Normalize();

	auto capToRay = ray.m_Pos - capsule.m_P0;

	auto f = capDir.Dot(ray.m_Dir);
	if (1.0f - fabs(f) < kEpsilon && 1.0f - fabs(f) > -kEpsilon)
	{
		auto tClosest = capToRay.Dot(capDir) / f;
		auto closestPoint = ray.m_Pos + tClosest * ray.m_Dir;

		auto toCap = closestPoint - capsule.m_P1;
		if (toCap.LengthSqr() <= capsule.m_Radius * capsule.m_Radius)
		{
			return true;
		}
	}

	BvSphere s0(capsule.m_P0, capsule.m_Radius); f32 ts0;
	BvSphere s1(capsule.m_P1, capsule.m_Radius); f32 ts1;
	bool inS0 = Intersects(ray, s0, ts0);
	bool inS1 = Intersects(ray, s1, ts1);

	if (inS0 && inS1)
	{
		t = std::min(ts0, ts1);
		return true;
	}
	else if (inS0)
	{
		t = ts0;
		return true;
	}
	else if (inS1)
	{
		t = ts1;
		return true;
	}

	return false;
}

bool Intersects(const BvTriangle& triangle1, const BvTriangle& triangle2)
{
	BvVec t0[] = { triangle1.m_V0, triangle1.m_V1, triangle1.m_V2 };
	BvVec t1[] = { triangle2.m_V0, triangle2.m_V1, triangle2.m_V2 };

	auto n0 = (triangle1.m_V1 - triangle1.m_V0).Cross(triangle1.m_V2 - triangle1.m_V0);
	if (!OverlapOnAxis(n0, t0, 3, t1, 3))
	{
		return false;
	}

	auto n1 = (triangle2.m_V1 - triangle2.m_V0).Cross(triangle2.m_V2 - triangle2.m_V0);
	if (!OverlapOnAxis(n1, t0, 3, t1, 3))
	{
		return false;
	}

	BvVec e0[] = { (triangle1.m_V1 - triangle1.m_V0), (triangle1.m_V2 - triangle1.m_V1), (triangle1.m_V0 - triangle1.m_V2) };
	BvVec e1[] = { (triangle2.m_V1 - triangle2.m_V0), (triangle2.m_V2 - triangle2.m_V1), (triangle2.m_V0 - triangle2.m_V2) };
	for (auto i = 0; i < 3; ++i)
	{
		for (auto j = 0; j < 3; ++j)
		{
			auto n = e0[i].Cross(e1[j]);
			if (!OverlapOnAxis(n, t0, 3, t1, 3))
			{
				return false;
			}
		}
	}

	return true;
}

bool Intersects(const BvTriangle& triangle, const BvSphere& sphere)
{
	// Check the points
	if (IsPointInSphere(sphere, triangle.m_V0) || IsPointInSphere(sphere, triangle.m_V1) || IsPointInSphere(sphere, triangle.m_V2))
	{
		return true;
	}

	// Check the line segments
	auto r2 = sphere.m_Radius * sphere.m_Radius;
	if (SqrDistanceFromPointToLineSegment(sphere.m_Center, triangle.m_V0, triangle.m_V1) <= r2
		|| SqrDistanceFromPointToLineSegment(sphere.m_Center, triangle.m_V1, triangle.m_V2) <= r2
		|| SqrDistanceFromPointToLineSegment(sphere.m_Center, triangle.m_V2, triangle.m_V0) <= r2)
	{
		return true;
	}

	// Check the triangle's plane distance to the sphere center
	auto v0 = (triangle.m_V1 - triangle.m_V0);
	auto v1 = (triangle.m_V2 - triangle.m_V0);
	auto n = v0.Cross(v1).Normal();
	if (fabsf(n.Dot(sphere.m_Center - triangle.m_V0)) <= sphere.m_Radius)
	{
		return true;
	}

	return false;
}

bool Intersects(const BvTriangle& triangle, const BvPlane& plane)
{
	auto t0 = SignedDistanceToPlane(plane, triangle.m_V0);
	auto t1 = SignedDistanceToPlane(plane, triangle.m_V1);
	auto t2 = SignedDistanceToPlane(plane, triangle.m_V2);

	bool anyPos = t0 > 0.0f || t1 > 0.0f || t2 > 0.0f;
	bool anyNeg = t0 < 0.0f || t1 < 0.0f || t2 < 0.0f;

	return anyPos && anyNeg;
}

bool Intersects(const BvSphere& sphere, const BvFrustum& frustum)
{
	for (auto& plane : frustum.m_Planes)
	{
		auto d = SignedDistanceToPlane(plane, sphere.m_Center);

		if (d < -sphere.m_Radius)
		{
			return false;
		}
	}

	return true;
}

bool Intersects(const BvPlane& plane, const BvAABB& aabb)
{
	BvVec zero;
	BvVec min = aabb.m_Min;
	BvVec max = aabb.m_Max;

	BvVec p0 = plane.m_Normal >= zero;
	max = max.And(p0);
	min = p0.AndNot(min);

	p0 = min.Or(max);

	if (SignedDistanceToPlane(plane, p0) < 0.0f)
	{
		return false;
	}

	return true;
}

bool Intersects(const BvPlane& plane, const BvOBB& obb)
{
	BvMat m(obb.m_Rotation);
	auto e = obb.m_Extents * m;

	BvVec verts[8];
	for (auto i = 0; i < 8; ++i)
	{
		auto dir = BvVec(i & 1 ? 1.0f : -1.0f, i & 2 ? 1.0f : -1.0f, i & 4 ? 1.0f : -1.0f);
		verts[i] = obb.m_Center + dir * e;
	}

	u32 outside = 0;
	for (auto& vert : verts)
	{
		if (SignedDistanceToPlane(plane, vert) < 0.0f)
		{
			++outside;
		}
		else
		{
			break;
		}
	}

	// For a more precise test, continue with SATs for the OBB's axes,
	// and their cross products with the frustum planes

	return outside < 8;
}

bool Intersects(const BvPlane& plane, const BvCapsule& capsule)
{
	if (Intersects(plane, BvSphere(capsule.m_P0, capsule.m_Radius)) || Intersects(plane, BvSphere(capsule.m_P1, capsule.m_Radius)))
	{
		return true;
	}

	auto d0 = SignedDistanceToPlane(plane, capsule.m_P0);
	auto d1 = SignedDistanceToPlane(plane, capsule.m_P1);
	if (d0 < -capsule.m_Radius && d1 < capsule.m_Radius)
	{
		return false;
	}

	return true;
}

bool Intersects(const BvAABB& aabb1, const BvAABB& aabb2)
{
	return (aabb1.m_Min <= aabb2.m_Max).AllTrue() && (aabb1.m_Max >= aabb2.m_Min).AllTrue();
}

bool Intersects(const BvAABB& aabb, const BvOBB& obb)
{
	BvOBB obb2(aabb);
	return Intersects(obb, obb2);
}

bool Intersects(const BvAABB& aabb, const BvFrustum& frustum)
{
	BvVec zero;
	for (auto& plane : frustum.m_Planes)
	{
		BvVec min = aabb.m_Min;
		BvVec max = aabb.m_Max;

		BvVec p0 = plane.m_Normal >= zero;
		max = max.And(p0);
		min = p0.AndNot(min);

		p0 = min.Or(max);

		if (SignedDistanceToPlane(plane, p0) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool Intersects(const BvAABB& aabb, const BvCapsule& capsule)
{
	auto closestP0 = capsule.m_P0.Max(aabb.m_Min).Min(aabb.m_Max);
	auto closestP1 = capsule.m_P1.Max(aabb.m_Min).Min(aabb.m_Max);

	auto d0Sqr = (closestP0 - capsule.m_P0).LengthSqr();
	auto d1Sqr = (closestP1 - capsule.m_P1).LengthSqr();

	auto r2 = capsule.m_Radius * capsule.m_Radius;

	return d0Sqr <= r2 || d1Sqr <= r2;
}

bool Intersects(const BvOBB& obb1, const BvOBB& obb2)
{
	BvVec vertsA[8];
	BvVec vertsB[8];

	auto m1 = BvMat(obb1.m_Rotation);
	auto m2 = BvMat(obb2.m_Rotation);

	auto e1 = obb1.m_Extents * m1;
	auto e2 = obb2.m_Extents * m2;

	for (auto i = 0; i < 8; ++i)
	{
		auto dir = BvVec(i & 1 ? 1.0f : -1.0f, i & 2 ? 1.0f : -1.0f, i & 4 ? 1.0f : -1.0f);
		vertsA[i] = obb1.m_Center + dir * e1;
		vertsB[i] = obb2.m_Center + dir * e2;
	}

	for (auto i = 0; i < 3; ++i)
	{
		if (!OverlapOnAxis(m1[i], vertsA, 8, vertsB, 8))
		{
			return false;
		}
		if (!OverlapOnAxis(m2[i], vertsA, 8, vertsB, 8))
		{
			return false;
		}
		for (auto j = 0; j < 3; ++j)
		{
			auto cAxis = m1[i].Cross(m2[j]);
			if (cAxis.LengthSqr() > kEpsilon)
			{
				if (!OverlapOnAxis(cAxis, vertsA, 8, vertsB, 8))
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool Intersects(const BvOBB& obb, const BvFrustum& frustum)
{
	BvMat m(obb.m_Rotation);
	auto e = obb.m_Extents * m;

	BvVec verts[8];
	for (auto i = 0; i < 8; ++i)
	{
		auto dir = BvVec(i & 1 ? 1.0f : -1.0f, i & 2 ? 1.0f : -1.0f, i & 4 ? 1.0f : -1.0f);
		verts[i] = obb.m_Center + dir * e;
	}

	for (auto& plane : frustum.m_Planes)
	{
		u32 outside = 0;
		for (auto& vert : verts)
		{
			if (SignedDistanceToPlane(plane, vert) < 0.0f)
			{
				++outside;
			}
			else
			{
				break;
			}
		}

		if (outside == 8)
		{
			return false;
		}
	}

	// For a more precise test, continue with SATs for the OBB's axes,
	// and their cross products with the frustum planes

	return true;
}

bool Intersects(const BvOBB& obb, const BvCapsule& capsule)
{
	auto l0 = capsule.m_P0 - obb.m_Center;
	auto l1 = capsule.m_P1 - obb.m_Center;
	auto q = obb.m_Rotation.Conjugate();
	l0 = q.QCVQ(l0).ToVector();
	l1 = q.QCVQ(l1).ToVector();

	BvAABB aabb(-obb.m_Extents, obb.m_Extents);
	BvCapsule lcap(l0, l1, capsule.m_Radius);

	return Intersects(aabb, lcap);
}

IntersectionType TestIntersection(const BvFrustum& frustum, const BvTriangle& triangle)
{
	u32 fullyInside = 0;
	for (auto& plane : frustum.m_Planes)
	{
		auto t0 = SignedDistanceToPlane(plane, triangle.m_V0);
		auto t1 = SignedDistanceToPlane(plane, triangle.m_V1);
		auto t2 = SignedDistanceToPlane(plane, triangle.m_V2);

		if (t0 < 0.0f && t1 < 0.0f && t2 < 0.0f)
		{
			return IntersectionType::Outside;
		}

		if (t0 >= 0.0f && t1 >= 0.0f && t2 >= 0.0f)
		{
			++fullyInside;
		}
	}

	// SATs for more precision?

	return fullyInside == 6 ? IntersectionType::Inside : IntersectionType::Intersecting;
}

IntersectionType TestIntersection(const BvFrustum& frustum, const BvSphere& sphere)
{
	u32 inside = 0;
	for (auto& plane : frustum.m_Planes)
	{
		auto d = SignedDistanceToPlane(plane, sphere.m_Center);
		if (d < -sphere.m_Radius)
		{
			return IntersectionType::Outside;
		}
		else if (d > sphere.m_Radius)
		{
			++inside;
		}
	}

	if (inside == 6)
	{
		return IntersectionType::Inside;
	}

	return IntersectionType::Intersecting;
}

IntersectionType TestIntersection(const BvFrustum& frustum, const BvAABB& aabb)
{
	BvOBB obb(aabb);
	return TestIntersection(frustum, obb);
}

IntersectionType TestIntersection(const BvFrustum& frustum, const BvOBB& obb)
{
	BvMat m(obb.m_Rotation);
	auto e = obb.m_Extents * m;

	BvVec verts[8];
	for (auto i = 0; i < 8; ++i)
	{
		auto dir = BvVec(i & 1 ? 1.0f : -1.0f, i & 2 ? 1.0f : -1.0f, i & 4 ? 1.0f : -1.0f);
		verts[i] = obb.m_Center + dir * e;
	}

	u32 fullyInside = 0;
	for (auto& plane : frustum.m_Planes)
	{
		u32 inside = 0;
		for (auto& vert : verts)
		{
			if (SignedDistanceToPlane(plane, vert) >= 0.0f)
			{
				++inside;
			}
		}

		if (inside == 0)
		{
			return IntersectionType::Outside;
		}
		else if (inside == 8)
		{
			++fullyInside;
		}
	}

	return fullyInside == 6 ? IntersectionType::Inside : IntersectionType::Intersecting;
}

IntersectionType TestIntersection(const BvFrustum& frustum, const BvCapsule& capsule)
{
	auto s0 = TestIntersection(frustum, BvSphere(capsule.m_P0, capsule.m_Radius));
	auto s1 = TestIntersection(frustum, BvSphere(capsule.m_P1, capsule.m_Radius));
	if (s0 == IntersectionType::Inside && s1 == IntersectionType::Inside)
	{
		return IntersectionType::Inside;
	}
	else if (s0 == IntersectionType::Intersecting || s1 == IntersectionType::Intersecting)
	{
		return IntersectionType::Intersecting;
	}

	for (auto& plane : frustum.m_Planes)
	{
		auto d0 = SignedDistanceToPlane(plane, capsule.m_P0);
		auto d1 = SignedDistanceToPlane(plane, capsule.m_P1);
		if (d0 < -capsule.m_Radius && d1 < capsule.m_Radius)
		{
			return IntersectionType::Outside;
		}
	}

	return IntersectionType::Intersecting;
}