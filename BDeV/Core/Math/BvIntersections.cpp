#include "BvIntersections.h"
#include <utility>


vf32 SignedDistanceToPlane(const BvPlane& plane, cvf32 point)
{
	return Vector4Dot(Load4(plane.m_Normal.v), VectorSetW(point, 1.0f));
}

vf32 ClosestPointToLine(cvf32 p, cvf32 l1, cvf32 l2)
{
	vf32 d = l2 - l1;
	vf32 toPoint = p - l1;

	vf32 t = Vector3Dot(toPoint, d);
	vf32 denom = VectorRcp(Vector3Dot(d, d));

	t *= denom;
	t = VectorClamp(t, VectorZero(), VectorOne());

	return p + t * d;
}

void ProjectOnAxis(cvf32 axis, const cvf32* pPoints, u32 pointCount, vf32& min, vf32& max)
{
	min = Vector3Dot(axis, pPoints[0]);
	max = min;
	for (auto i = 1; i < pointCount; ++i)
	{
		vf32 proj = Vector3Dot(axis, pPoints[i]);
		min = VectorMin(min, proj);
		max = VectorMax(max, proj);
	}
}

bool OverlapOnAxis(cvf32 axis, const cvf32* pPointsA, u32 pointCountA, const cvf32* pPointsB, u32 pointCountB)
{
	vf32 minA, maxA, minB, maxB;
	ProjectOnAxis(axis, pPointsA, pointCountA, minA, maxA);
	ProjectOnAxis(axis, pPointsB, pointCountB, minB, maxB);

	vf32 r1 = Vector4GreaterEqual(maxA, minB);
	vf32 r2 = Vector4GreaterEqual(maxB, minA);
	return VectorAllTrue(r1) && VectorAllTrue(r2);
}


bool IsPointInSphere(const BvSphere& sphere, cvf32 p)
{
	vf32 toCenter = VectorSub(Load(sphere.m_Center), p);
	f32 lenSqr = VectorGetX(Vector3LengthSqr(toCenter));
	return lenSqr <= sphere.m_Radius * sphere.m_Radius;
}

vf32 SqrDistanceFromPointToLineSegment(cvf32 p, cvf32 v0, cvf32 v1)
{
	vf32 line = VectorSub(v1, v0);
	vf32 toPoint = VectorSub(p, v0);
	vf32 t = VectorDiv(Vector3Dot(toPoint, line), Vector3Dot(line, line));
	t = VectorClamp(t, VectorZero(), VectorOne());

	vf32 closest = VectorMAdd(line, t, v0);

	return Vector3LengthSqr(closest);
}

void GetVertices(const BvAABB& aabb, vf32* pVertices)
{
	vf32 e = Load(aabb.m_Extents);

	vf32 x = VectorUnitX();
	vf32 y = VectorUnitY();
	vf32 z = VectorUnitZ();

	vf32 c = Load(aabb.m_Center);
	pVertices[0] = c - x - y - z;
	pVertices[1] = c + x - y - z;
	pVertices[2] = c - x + y - z;
	pVertices[3] = c + x + y - z;
	pVertices[4] = c - x - y + z;
	pVertices[5] = c + x - y + z;
	pVertices[6] = c - x + y + z;
	pVertices[7] = c + x + y + z;
}

void GetVertices(const BvOBB& obb, vf32* pVertices)
{
	vf32 e = Load(obb.m_Extents);
	mf32 rot = MatrixFromQuaternion(Load(obb.m_Rotation));

	vf32 x = rot.r[0] * e;
	vf32 y = rot.r[1] * e;
	vf32 z = rot.r[2] * e;

	vf32 c = Load(obb.m_Center);
	pVertices[0] = c - x - y - z;
	pVertices[1] = c + x - y - z;
	pVertices[2] = c - x + y - z;
	pVertices[3] = c + x + y - z;
	pVertices[4] = c - x - y + z;
	pVertices[5] = c + x - y + z;
	pVertices[6] = c - x + y + z;
	pVertices[7] = c + x + y + z;
}

void GetMinMax(const BvAABB& aabb, vf32& min, vf32& max)
{
	vf32 c = Load(aabb.m_Center);
	vf32 e = Load(aabb.m_Extents);

	min = c - e;
	max = c + e;
}

vf32 ClosestPoint(const BvSphere& sphere, cvf32 p)
{
	vf32 c = Load(sphere.m_Center);
	vf32 toPoint = Vector3Normalize(p - c);
	vf32 r = VectorSet(sphere.m_Radius);
	toPoint *= r;

	return c + toPoint;
}

vf32 ClosestPoint(const BvAABB& aabb, cvf32 p)
{
	vf32 min, max;
	GetMinMax(aabb, min, max);

	return VectorClamp(p, min, max);
}

vf32 ClosestPoint(const BvOBB& obb, cvf32 p)
{
	vf32 c = Load(obb.m_Center);
	vf32 toPoint = p - c;

	mf32 rot = MatrixFromQuaternion(Load(obb.m_Rotation));
	vf32 e = Load(obb.m_Extents);

	vf32 d = Vector3Dot(toPoint, rot.r[0]);
	d = VectorClamp(d, -e, e);
	c += rot.r[0] * d;

	d = Vector3Dot(toPoint, rot.r[1]);
	d = VectorClamp(d, -e, e);
	c += rot.r[1] * d;

	d = Vector3Dot(toPoint, rot.r[2]);
	d = VectorClamp(d, -e, e);
	c += rot.r[2] * d;

	return c;
}

vf32 LineLineDistanceSqr(cvf32 p1, cvf32 p2, cvf32 e1, cvf32 e2)
{
	vf32 u = p2 - p1;
	vf32 v = e2 - e1;
	vf32 w = p1 - e1;

	vf32 a = Vector3Dot(u, u); // Squared length of u
	vf32 b = Vector3Dot(u, v);
	vf32 c = Vector3Dot(v, v); // Squared length of v
	vf32 d = Vector3Dot(u, w);
	vf32 e = Vector3Dot(v, w);

	vf32 denom = a * c - b * b;
	vf32 c0;
	{
		// If denom is zero, the lines are parallel
		vf32 eps = VectorSet(kEpsilon);
		c0 = Vector4Less(denom, eps);
	}

	vf32 sc, tc;
	{
		vf32 v0 = VectorZero();
		vf32 v1 = VectorOne();
		
		vf32 sc0 = v0;
		vf32 tc0 = VectorBlend(d / b, e / c, Vector4Greater(b, c));

		sc = VectorAnd(c0, sc0);
		tc = VectorAnd(c0, tc0);

		vf32 invD = VectorRcp(denom);
		sc0 = (b * e - c * d) * invD;
		tc0 = (a * e - b * d) * invD;

		sc = VectorOr(sc, VectorAndNot(c0, sc0));
		tc = VectorOr(tc, VectorAndNot(c0, tc0));

		sc = VectorClamp(sc, v0, v1);
		tc = VectorClamp(tc, v0, v1);
	}

	// Closest vector between the two segments
	vf32 dP = w + (sc * u) - (tc * v);

	// Shortest distance between the segments
	return Vector3LengthSqr(dP);
}

bool Intersects(const BvRay& ray, const BvTriangle& triangle, f32& t, f32& u, f32& v)
{
	vf32 v0 = Load(triangle.m_V0);
	vf32 e1 = Load(triangle.m_V1) - v0;
	vf32 e2 = Load(triangle.m_V2) - v0;
	
	vf32 d = Load(ray.m_Dir);
	vf32 p = Vector3Cross(d, e2);
	vf32 det = Vector3Dot(e1, p);

	vf32 eps = VectorSet(kEpsilon);
	vf32 c0 = Vector4Greater(det, -eps);
	c0 = VectorAnd(c0, Vector4Less(det, eps));
	if (VectorAllTrue(c0))
	{
		return false;
	}

	vf32 pos = Load(ray.m_Pos);
	vf32 tv = pos - v0;

	det = VectorRcp(det);
	vf32 q = Vector3Cross(tv, e1);

	vf32 r0 = VectorZero();
	vf32 r1 = VectorOne();

	vf32 uvt = Vector3Dot(p, tv) * det;
	c0 = Vector4Less(uvt, r0);
	c0 = VectorOr(c0, Vector4Greater(uvt, r1));

	{
		vf32 tmp = Vector3Dot(d, q) * det;
		c0 = VectorOr(c0, Vector4Less(tmp, r0));
		c0 = VectorOr(c0, Vector4Greater(tmp, r1));
		uvt = VectorBlend(uvt, tmp, VectorMaskInvY());
	}

	{
		vf32 tmp = Vector3Dot(q, e2) * det;
		c0 = VectorOr(c0, Vector4LessEqual(tmp, eps));
		uvt = VectorBlend(uvt, tmp, VectorMaskInvZ());
	}

	if (VectorAllTrue(c0))
	{
		return false;
	}
	else
	{
		float r[3];
		Store3(uvt, r);
		u = r[0];
		v = r[1];
		t = r[2];

		return true;
	}
}

bool Intersects(const BvRay& ray, const BvSphere& sphere, f32& t)
{
	vf32 rayToCenter = VectorSub(Load(sphere.m_Center), Load(ray.m_Pos));
	vf32 tca = Vector3Dot(rayToCenter, Load(ray.m_Dir));
	
	// if (tca < 0.0f) then ray goes to opposite direction of the sphere
	vf32 c0 = Vector4GreaterEqual(tca, VectorZero());

	vf32 d2 = Vector3Dot(rayToCenter, rayToCenter) - tca * tca;
	vf32 rad2 = VectorSet(sphere.m_Radius * sphere.m_Radius);

	// if (d2 > rad2) then ray misses the sphere
	c0 = VectorAnd(c0, Vector4LessEqual(d2, rad2));

	vf32 thc = VectorSqrt(rad2 - d2);
	t = VectorGetX(tca - thc);

	return VectorAllTrue(c0);
}

bool Intersects(const BvRay& ray, const BvPlane& plane, f32& t)
{
	vf32 n = Load(plane.m_Normal);
	vf32 d = Load(ray.m_Dir);
	vf32 p = Load(ray.m_Pos);

	vf32 denom = Vector3Dot(d, n);
	vf32 num = VectorNegate(SignedDistanceToPlane(plane, p));

	vf32 r = VectorDiv(VectorNegate(num), denom);

	vf32 zero = VectorZero();
	vf32 c0 = Vector4Less(denom, zero);
	c0 = VectorAnd(c0, Vector4GreaterEqual(r, zero));

	t = VectorGetX(r);

	return VectorAllTrue(c0);
}

bool Intersects(const BvRay& ray, const BvAABB& aabb, f32& t)
{
	vf32 e = Load(ray.m_Pos) - Load(aabb.m_Center);
	vf32 f = Load(ray.m_Dir);
	vf32 ext = Load(aabb.m_Extents);

	vf32 eps = VectorSet(kEpsilon);
	vf32 vfMin = VectorSet(kF32Min);
	vf32 vfMax = VectorSet(kF32Max);
	
	// Check if it's parallel
	vf32 c0 = Vector3LessEqual(VectorAbs(f), eps);

	f = VectorRcp(f);
	
	vf32 t1 = (e + ext) * f;
	vf32 t2 = (e - ext) * f;
	
	vf32 vt1 = VectorBlend(vfMin, VectorMin(t1, t2), c0);
	vf32 vt2 = VectorBlend(vfMax, VectorMax(t1, t2), c0);

	vt1 = VectorMax(vt1, VectorReplicateY(vt1));
	vt1 = VectorReplicateX(VectorMax(vt1, VectorReplicateZ(vt1)));

	vt2 = VectorMin(vt2, VectorReplicateY(vt2));
	vt2 = VectorReplicateX(VectorMin(vt2, VectorReplicateZ(vt2)));

	vf32 z = VectorZero();
	// if min > max || max < 0.0f then no intersection
	vf32 c1 = Vector3Greater(vt1, vt2);
	c1 = VectorOr(c1, Vector3Less(vt2, z));

	{
		vf32 ee1 = -e;
		vf32 ee2 = ee1 + ext;
		ee1 -= ext;

		ee1 = Vector3Greater(ee1, z);
		ee2 = VectorOr(ee1, Vector3Less(ee2, z));

		// if is parallel (c0) and (-e - ext > 0.0f || -e + ext < 0.0f) then no intersection
		c1 = VectorOr(c1, VectorAnd(c0, ee2));
	}

	if (Vector3AllFalse(c1))
	{
		t = VectorGetX(vt1);
		return true;
	}

	return false;
}

bool Intersects(const BvRay& ray, const BvOBB& obb, f32& t)
{
	vf32 e;
	vf32 f;
	{
		mf32 rot = MatrixFromQuaternion(Load(obb.m_Rotation));
		{
			vf32 obbToRay = Load(ray.m_Pos) - Load(obb.m_Center);
			e = Vector3Dot(obbToRay, rot.r[0]);
			e = VectorBlend(e, Vector3Dot(obbToRay, rot.r[1]), 0x0D);
			e = VectorBlend(e, Vector3Dot(obbToRay, rot.r[2]), 0x0B);
		}

		{
			vf32 d = Load(ray.m_Dir);
			f = Vector3Dot(d, rot.r[0]);
			f = VectorBlend(f, Vector3Dot(d, rot.r[1]), 0x0D);
			f = VectorBlend(f, Vector3Dot(d, rot.r[2]), 0x0B);
		}
	}
	vf32 ext = Load(obb.m_Extents);

	vf32 eps = VectorSet(kEpsilon);
	vf32 vfMin = VectorSet(kF32Min);
	vf32 vfMax = VectorSet(kF32Max);

	// Check if it's parallel
	vf32 c0 = Vector3LessEqual(VectorAbs(f), eps);

	f = VectorRcp(f);

	vf32 t1 = (e + ext) * f;
	vf32 t2 = (e - ext) * f;

	vf32 vt1 = VectorBlend(vfMin, VectorMin(t1, t2), c0);
	vf32 vt2 = VectorBlend(vfMax, VectorMax(t1, t2), c0);

	vt1 = VectorMax(vt1, VectorReplicateY(vt1));
	vt1 = VectorReplicateX(VectorMax(vt1, VectorReplicateZ(vt1)));

	vt2 = VectorMin(vt2, VectorReplicateY(vt2));
	vt2 = VectorReplicateX(VectorMin(vt2, VectorReplicateZ(vt2)));

	vf32 z = VectorZero();
	// if min > max || max < 0.0f then no intersection
	vf32 c1 = Vector3Greater(vt1, vt2);
	c1 = VectorOr(c1, Vector3Less(vt2, z));

	{
		vf32 ee1 = -e;
		vf32 ee2 = ee1 + ext;
		ee1 -= ext;

		ee1 = Vector3Greater(ee1, z);
		ee2 = VectorOr(ee1, Vector3Less(ee2, z));

		// if is parallel (c0) and (-e - ext > 0.0f || -e + ext < 0.0f) then no intersection
		c1 = VectorOr(c1, VectorAnd(c0, ee2));
	}

	if (Vector3AllFalse(c1))
	{
		t = VectorGetX(vt1);
		return true;
	}

	return false;
}

bool Intersects(const BvRay& ray, const BvCapsule& capsule, f32& t)
{
	vf32 p1 = Load(capsule.m_P1);
	vf32 p2 = Load(capsule.m_P2);
	vf32 d = Load(ray.m_Dir);
	vf32 o = Load(ray.m_Pos);

	vf32 capDir = p2 - p1;
	capDir = Vector3Normalize(capDir);

	vf32 capToRay = o - p1;

	bool notParallel = false;
	{
		vf32 c0;
		vf32 proj = Vector3Cross(d, capDir);
		vf32 lenSqr = Vector3LengthSqr(proj);
		vf32 epsSqr = VectorSet(kEpsilon * kEpsilon);
		c0 = Vector4Greater(lenSqr, epsSqr);
		notParallel = VectorAnyTrue(c0);
	}

	if (notParallel)
	{
		vf32 tClosest = Vector3Dot(capToRay, capDir) / Vector3Dot(d, capDir);
		vf32 closestPoint = o + d * tClosest;

		vf32 capsDist = closestPoint - p1;
		vf32 lenSqr = Vector3LengthSqr(capsDist);

		vf32 radSqr = VectorSet(capsule.m_Radius * capsule.m_Radius);
		vf32 c0 = Vector3LessEqual(lenSqr, radSqr);

		if (Vector3AnyTrue(c0))
		{
			return true;
		}
	}

	f32 t1, t2;
	BvSphere s1(capsule.m_P1, capsule.m_Radius);
	BvSphere s2(capsule.m_P2, capsule.m_Radius);
	bool inS1 = Intersects(ray, s1, t1);
	bool inS2 = Intersects(ray, s2, t2);

	if (inS1 && inS2)
	{
		t = std::min(t1, t2);
		return true;
	}
	else if (inS1)
	{
		t = t1;
		return true;
	}
	else if (inS2)
	{
		t = t2;
		return true;
	}

	return false;
}

bool Intersects(const BvTriangle& triangle1, const BvTriangle& triangle2)
{
	vf32 t1[] = { Load(triangle1.m_V0), Load(triangle1.m_V1), Load(triangle1.m_V2) };
	vf32 t2[] = { Load(triangle2.m_V0), Load(triangle2.m_V1), Load(triangle2.m_V2) };

	vf32 fn = Vector3Cross(t1[1] - t1[0], t1[2] - t1[0]);
	if (!OverlapOnAxis(fn, t1, 3, t2, 3))
	{
		return false;
	}

	fn = Vector3Cross(t2[1] - t2[0], t2[2] - t2[0]);
	if (!OverlapOnAxis(fn, t1, 3, t2, 3))
	{
		return false;
	}

	vf32 e1[] = { (t1[1] - t1[0]), (t1[2] - t1[1]), (t1[0] - t1[2]) };
	vf32 e2[] = { (t2[1] - t2[0]), (t2[2] - t2[1]), (t2[0] - t2[2]) };
	vf32 eps = VectorSet(kEpsilon);
	for (auto i = 0; i < 3; ++i)
	{
		for (auto j = 0; j < 3; ++j)
		{
			fn = Vector3Cross(e1[i], e2[j]);
			vf32 c0 = Vector3Greater(Vector3LengthSqr(fn), eps);
			if (VectorAllTrue(c0))
			{
				if (!OverlapOnAxis(fn, t1, 3, t2, 3))
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool Intersects(const BvTriangle& triangle, const BvSphere& sphere)
{
	vf32 t[] = { Load(triangle.m_V0), Load(triangle.m_V1), Load(triangle.m_V2) };

	// Check the points
	if (IsPointInSphere(sphere, t[0]) || IsPointInSphere(sphere, t[1]) || IsPointInSphere(sphere, t[2]))
	{
		return true;
	}

	// Check the line segments
	vf32 c = Load(sphere.m_Center);
	vf32 r2 = VectorSet(sphere.m_Radius * sphere.m_Radius);

	vf32 c0 = Vector4LessEqual(SqrDistanceFromPointToLineSegment(c, t[0], t[1]), r2);
	c0 = VectorOr(c0, Vector4LessEqual(SqrDistanceFromPointToLineSegment(c, t[1], t[2]), r2));
	c0 = VectorOr(c0, Vector4LessEqual(SqrDistanceFromPointToLineSegment(c, t[2], t[0]), r2));

	if (VectorAnyTrue(c0))
	{
		return true;
	}

	// Check the triangle's plane distance to the sphere center
	vf32 n = Vector3Cross(t[1] - t[0], t[2] - t[0]);
	n = Vector3Normalize(n);
	c0 = VectorAbs(Vector3Dot(c - t[0], n));
	c0 = Vector4LessEqual(c0, r2);

	if (VectorAnyTrue(c0))
	{
		return true;
	}

	return false;
}

bool Intersects(const BvTriangle& triangle, const BvPlane& plane)
{
	vf32 t0 = SignedDistanceToPlane(plane, Load(triangle.m_V0));
	vf32 t1 = SignedDistanceToPlane(plane, Load(triangle.m_V1));
	vf32 t2 = SignedDistanceToPlane(plane, Load(triangle.m_V2));

	vf32 z = VectorZero();
	
	vf32 c0 = Vector4Greater(t0, z);
	c0 = VectorOr(c0, Vector4Greater(t1, z));
	c0 = VectorOr(c0, Vector4Greater(t2, z));

	vf32 c1 = Vector4Less(t0, z);
	c1 = VectorOr(c1, Vector4Less(t1, z));
	c1 = VectorOr(c1, Vector4Less(t2, z));

	return VectorAnyTrue(c0) && VectorAnyTrue(c1);
}

bool Intersects(const BvTriangle& triangle, const BvAABB& aabb)
{
	vf32 t[] = { Load(triangle.m_V0), Load(triangle.m_V1), Load(triangle.m_V2) };

	vf32 verts[8];
	GetVertices(aabb, verts);

	vf32 fn = Vector3Cross(t[1] - t[0], t[2] - t[0]);
	if (!OverlapOnAxis(fn, t, 3, verts, 8))
	{
		return false;
	}

	vf32 an[] = { VectorUnitX(), VectorUnitY(), VectorUnitZ() };
	if (!OverlapOnAxis(an[0], t, 3, verts, 8)
		|| !OverlapOnAxis(an[1], t, 3, verts, 8)
		|| !OverlapOnAxis(an[2], t, 3, verts, 8))
	{
		return false;
	}

	vf32 e[] = { (t[1] - t[0]), (t[2] - t[1]), (t[0] - t[2]) };
	vf32 eps = VectorSet(kEpsilon);
	for (auto i = 0; i < 3; ++i)
	{
		for (auto j = 0; j < 3; ++j)
		{
			vf32 cAxis = Vector3Cross(e[i], an[j]);
			vf32 c0 = Vector3Greater(Vector3LengthSqr(cAxis), eps);
			if (VectorAllTrue(c0))
			{
				if (!OverlapOnAxis(cAxis, t, 3, verts, 8))
				{
					return false;
				}
			}
		}
	}

	return false;
}

bool Intersects(const BvTriangle& triangle, const BvOBB& obb)
{
	vf32 t[] = { Load(triangle.m_V0), Load(triangle.m_V1), Load(triangle.m_V2) };

	vf32 verts[8];
	GetVertices(obb, verts);

	vf32 fn = Vector3Cross(t[1] - t[0], t[2] - t[0]);
	if (!OverlapOnAxis(fn, t, 3, verts, 8))
	{
		return false;
	}

	mf32 on = MatrixFromQuaternion(Load(obb.m_Rotation));
	if (!OverlapOnAxis(on.r[0], t, 3, verts, 8)
		|| !OverlapOnAxis(on.r[1], t, 3, verts, 8)
		|| !OverlapOnAxis(on.r[2], t, 3, verts, 8))
	{
		return false;
	}

	vf32 e[] = { (t[1] - t[0]), (t[2] - t[1]), (t[0] - t[2]) };
	vf32 eps = VectorSet(kEpsilon);
	for (auto i = 0; i < 3; ++i)
	{
		for (auto j = 0; j < 3; ++j)
		{
			vf32 cAxis = Vector3Cross(e[i], on.r[j]);
			vf32 c0 = Vector3Greater(Vector3LengthSqr(cAxis), eps);
			if (VectorAllTrue(c0))
			{
				if (!OverlapOnAxis(cAxis, t, 3, verts, 8))
				{
					return false;
				}
			}
		}
	}

	return false;
}

bool Intersects(const BvTriangle& triangle, const BvCapsule& capsule)
{
	if (Intersects(triangle, BvSphere(capsule.m_P1, capsule.m_Radius))
		|| Intersects(triangle, BvSphere(capsule.m_P2, capsule.m_Radius)))
	{
		return true;
	}

	vf32 t[] = { Load(triangle.m_V0), Load(triangle.m_V1), Load(triangle.m_V2) };
	vf32 p1 = Load(capsule.m_P1);
	vf32 p2 = Load(capsule.m_P2);

	vf32 capsDir = p2 - p1;
	vf32 capsLen = Vector3Length(capsDir);
	capsDir /= capsLen;

	vf32 z = VectorZero();
	vf32 r = VectorSet(capsule.m_Radius);
	for (auto i = 0; i < 3; ++i)
	{
		vf32 proj = VectorClamp(Vector3Dot(t[i] - p1, capsDir), z, capsLen);

		vf32 closestPoint = p1 + proj * capsDir;

		vf32 dist = Vector3Length(t[i] - closestPoint);
		vf32 c0 = Vector4LessEqual(dist, r);
		if (VectorAnyTrue(c0))
		{
			return true;
		}
	}

	r *= r;

	vf32 c0 = Vector4LessEqual(LineLineDistanceSqr(p1, p2, t[0], t[1]), r);
	c0 = VectorOr(c0, Vector4LessEqual(LineLineDistanceSqr(p1, p2, t[1], t[2]), r));
	c0 = VectorOr(c0, Vector4LessEqual(LineLineDistanceSqr(p1, p2, t[2], t[0]), r));

	return VectorAnyTrue(c0);
}

bool Intersects(const BvSphere& sphere1, const BvSphere& sphere2)
{
	vf32 d = Load(sphere1.m_Center) - Load(sphere2.m_Center);
	d = Vector3LengthSqr(d);

	vf32 radSum = Load(sphere1.m_Radius) + Load(sphere2.m_Radius);
	radSum *= radSum;

	return VectorAnyTrue(Vector4Less(d, radSum));
}

bool Intersects(const BvSphere& sphere, const BvPlane& plane)
{
	vf32 planeDist = SignedDistanceToPlane(plane, Load(sphere.m_Center));
	planeDist *= planeDist;

	vf32 rad = VectorSet(sphere.m_Radius * sphere.m_Radius);

	return VectorAnyTrue(Vector4Less(planeDist, rad));
}

bool Intersects(const BvSphere& sphere, const BvAABB& aabb)
{
	vf32 c = Load(sphere.m_Center);
	vf32 cp = ClosestPoint(aabb, c);

	vf32 lenSqr = Vector3LengthSqr(c - cp);
	vf32 r2 = VectorSet(sphere.m_Radius * sphere.m_Radius);

	return VectorAllTrue(Vector4Less(lenSqr, r2));
}

bool Intersects(const BvSphere& sphere, const BvOBB& obb)
{
	vf32 c = Load(sphere.m_Center);
	vf32 cp = ClosestPoint(obb, c);

	vf32 lenSqr = Vector3LengthSqr(c - cp);
	vf32 r2 = VectorSet(sphere.m_Radius * sphere.m_Radius);

	return VectorAllTrue(Vector4Less(lenSqr, r2));
}

bool Intersects(const BvSphere& sphere, const BvCapsule& capsule)
{
	vf32 c = Load(sphere.m_Center);
	vf32 p1 = Load(capsule.m_P1);
	vf32 p2 = Load(capsule.m_P2);
	
	vf32 cp = ClosestPointToLine(c, p1, p2);
	vf32 lenSqr = Vector3LengthSqr(cp - c);
	vf32 r2 = VectorSet(capsule.m_Radius * capsule.m_Radius + sphere.m_Radius + capsule.m_Radius);

	return VectorAllTrue(Vector4Less(lenSqr, r2));
}

bool Intersects(const BvPlane& plane, const BvAABB& aabb)
{
	vf32 e = Load(aabb.m_Extents);
	vf32 n = Load(plane.m_Normal);

	vf32 len = Vector3Dot(e, n);

	vf32 dist = VectorAbs(SignedDistanceToPlane(plane, Load(aabb.m_Center)));

	vf32 r = Vector4LessEqual(dist, len);

	return VectorAnyTrue(r);
}

bool Intersects(const BvPlane& plane, const BvOBB& obb)
{
	vf32 e = Load(obb.m_Extents);
	vf32 n = Load(plane.m_Normal);
	{
		mf32 rot = MatrixFromQuaternion(Load(obb.m_Rotation));
		vf32 v0 = Vector3Dot(n, rot.r[0]);
		v0 = VectorBlend(v0, Vector3Dot(n, rot.r[1]), 0x0D);
		v0 = VectorBlend(v0, Vector3Dot(n, rot.r[2]), 0x0B);
		n = VectorAbs(v0);
	}

	vf32 len = Vector3Dot(e, n);
	vf32 dist = VectorAbs(SignedDistanceToPlane(plane, Load(obb.m_Center)));
	vf32 r = Vector4LessEqual(dist, len);

	return VectorAnyTrue(r);
}

bool Intersects(const BvPlane& plane, const BvCapsule& capsule)
{
	if (Intersects(plane, BvSphere(capsule.m_P1, capsule.m_Radius)) || Intersects(plane, BvSphere(capsule.m_P2, capsule.m_Radius)))
	{
		return true;
	}

	vf32 r = VectorSet(capsule.m_Radius);

	vf32 d1 = SignedDistanceToPlane(plane, Load(capsule.m_P1));
	vf32 d2 = SignedDistanceToPlane(plane, Load(capsule.m_P2));

	vf32 c0 = Vector4Less(d1, VectorNegate(r));
	c0 = VectorAnd(c0, Vector4Less(d2, r));

	return VectorAllTrue(c0);
}

bool Intersects(const BvAABB& aabb1, const BvAABB& aabb2)
{
	vf32 minA, maxA, minB, maxB;
	GetMinMax(aabb1, minA, maxA);
	GetMinMax(aabb2, minB, maxB);

	vf32 c0 = Vector3LessEqual(minA, maxB);
	c0 = VectorAnd(c0, Vector3GreaterEqual(maxA, minB));

	return VectorAllTrue(c0);
}

bool Intersects(const BvAABB& aabb, const BvOBB& obb)
{
	BvOBB obb2(aabb);
	return Intersects(obb, obb2);
}

bool Intersects(const BvAABB& aabb, const BvCapsule& capsule)
{
	vf32 min, max;
	GetMinMax(aabb, min, max);

	vf32 p1 = Load(capsule.m_P1);
	vf32 p2 = Load(capsule.m_P2);

	vf32 closestP1 = VectorClamp(p1, min, max);
	vf32 closestP2 = VectorClamp(p2, min, max);

	vf32 d1Sqr = SqrDistanceFromPointToLineSegment(closestP1, p1, p2);
	vf32 d2Sqr = SqrDistanceFromPointToLineSegment(closestP2, p1, p2);

	vf32 r2 = VectorSet(capsule.m_Radius * capsule.m_Radius);
	
	vf32 c0 = Vector4LessEqual(d1Sqr, r2);
	c0 = VectorOr(c0, Vector4LessEqual(d2Sqr, r2));

	return VectorAnyTrue(c0);
}

bool Intersects(const BvOBB& obb1, const BvOBB& obb2)
{
	vf32 vertsA[8];
	vf32 vertsB[8];

	GetVertices(obb1, vertsA);
	GetVertices(obb2, vertsB);

	mf32 m1 = MatrixFromQuaternion(Load(obb1.m_Rotation));
	mf32 m2 = MatrixFromQuaternion(Load(obb2.m_Rotation));

	vf32 eps = VectorSet(kEpsilon);
	vf32 c0;
	for (auto i = 0; i < 3; ++i)
	{
		if (!OverlapOnAxis(m1.r[i], vertsA, 8, vertsB, 8))
		{
			return false;
		}
		if (!OverlapOnAxis(m2.r[i], vertsA, 8, vertsB, 8))
		{
			return false;
		}
		for (auto j = 0; j < 3; ++j)
		{
			vf32 cAxis = Vector3Cross(m1.r[i], m2.r[j]);
			c0 = Vector3Greater(Vector3LengthSqr(cAxis), eps);
			if (VectorAllTrue(c0))
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

bool Intersects(const BvOBB& obb, const BvCapsule& capsule)
{
	vf32 c = Load(obb.m_Center);
	vf32 l1 = Load(capsule.m_P1) - c;
	vf32 l2 = Load(capsule.m_P2) - c;

	vf32 r = Load(obb.m_Rotation);
	l1 = QuaternionQVQC(r, l1);
	l2 = QuaternionQVQC(r, l2);

	BvAABB aabb(Float3(), obb.m_Extents);
	BvCapsule lcap;
	lcap.m_Radius = capsule.m_Radius;
	Store(l1, lcap.m_P1);
	Store(l2, lcap.m_P2);

	return Intersects(aabb, lcap);
}

//bool Intersects(const BvRay& ray, const BvFrustum& frustum, f32& tMin, f32& tMax)
//{
//	tMin = kF32Min;
//	tMax = kF32Max;
//
//	for (auto& plane : frustum.m_Planes)
//	{
//		auto denom = ray.m_Dir.Dot(plane.m_Normal);
//		auto num = ray.m_Pos.Dot(plane.m_Normal) + plane.m_Distance;
//		if (fabs(denom) > kEpsilon)
//		{
//			auto t = -num / denom;
//
//			if (denom < 0.0f)
//			{
//				tMin = std::max(tMin, t);
//			}
//			else
//			{
//				tMax = std::min(tMax, t);
//			}
//
//			if (tMin > tMax)
//			{
//				return false;
//			}
//		}
//		else if (num > 0.0f)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool Intersects(const BvSphere& sphere, const BvFrustum& frustum)
//{
//	for (auto& plane : frustum.m_Planes)
//	{
//		auto d = SignedDistanceToPlane(plane, sphere.m_Center);
//
//		if (d < -sphere.m_Radius)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool Intersects(const BvAABB& aabb, const BvFrustum& frustum)
//{
//	BvVec zero;
//	for (auto& plane : frustum.m_Planes)
//	{
//		BvVec min = aabb.m_Min;
//		BvVec max = aabb.m_Max;
//
//		BvVec p0 = plane.m_Normal >= zero;
//		max = max.And(p0);
//		min = p0.AndNot(min);
//
//		p0 = min.Or(max);
//
//		if (SignedDistanceToPlane(plane, p0) < 0.0f)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool Intersects(const BvOBB& obb, const BvFrustum& frustum)
//{
//	BvMat m(obb.m_Rotation);
//	auto e = obb.m_Extents * m;
//
//	BvVec verts[8];
//	for (auto i = 0; i < 8; ++i)
//	{
//		auto dir = BvVec(i & 1 ? 1.0f : -1.0f, i & 2 ? 1.0f : -1.0f, i & 4 ? 1.0f : -1.0f);
//		verts[i] = obb.m_Center + dir * e;
//	}
//
//	for (auto& plane : frustum.m_Planes)
//	{
//		u32 outside = 0;
//		for (auto& vert : verts)
//		{
//			if (SignedDistanceToPlane(plane, vert) < 0.0f)
//			{
//				++outside;
//			}
//			else
//			{
//				break;
//			}
//		}
//
//		if (outside == 8)
//		{
//			return false;
//		}
//	}
//
//	// For a more precise test, continue with SATs for the OBB's axes,
//	// and their cross products with the frustum planes
//	// One horizontal edge from the near plane.
//	// One vertical edge from the near plane.
//	// One horizontal edge from the far plane.
//	// One vertical edge from the far plane.
//	// One diagonal edge along the left side connecting the near and far planes.
//	// One diagonal edge along the right side connecting the near and far planes.
//
//	return true;
//}
//
//IntersectionType TestIntersection(const BvFrustum& frustum, const BvTriangle& triangle)
//{
//	u32 fullyInside = 0;
//	for (auto& plane : frustum.m_Planes)
//	{
//		auto t0 = SignedDistanceToPlane(plane, triangle.m_V0);
//		auto t1 = SignedDistanceToPlane(plane, triangle.m_V1);
//		auto t2 = SignedDistanceToPlane(plane, triangle.m_V2);
//
//		if (t0 < 0.0f && t1 < 0.0f && t2 < 0.0f)
//		{
//			return IntersectionType::Outside;
//		}
//
//		if (t0 >= 0.0f && t1 >= 0.0f && t2 >= 0.0f)
//		{
//			++fullyInside;
//		}
//	}
//
//	// SATs for more precision?
//
//	return fullyInside == 6 ? IntersectionType::Inside : IntersectionType::Intersecting;
//}
//
//IntersectionType TestIntersection(const BvFrustum& frustum, const BvSphere& sphere)
//{
//	u32 inside = 0;
//	for (auto& plane : frustum.m_Planes)
//	{
//		auto d = SignedDistanceToPlane(plane, sphere.m_Center);
//		if (d < -sphere.m_Radius)
//		{
//			return IntersectionType::Outside;
//		}
//		else if (d > sphere.m_Radius)
//		{
//			++inside;
//		}
//	}
//
//	if (inside == 6)
//	{
//		return IntersectionType::Inside;
//	}
//
//	return IntersectionType::Intersecting;
//}
//
//IntersectionType TestIntersection(const BvFrustum& frustum, const BvAABB& aabb)
//{
//	BvOBB obb(aabb);
//	return TestIntersection(frustum, obb);
//}
//
//IntersectionType TestIntersection(const BvFrustum& frustum, const BvOBB& obb)
//{
//	BvMat m(obb.m_Rotation);
//	auto e = obb.m_Extents * m;
//
//	BvVec verts[8];
//	for (auto i = 0; i < 8; ++i)
//	{
//		auto dir = BvVec(i & 1 ? 1.0f : -1.0f, i & 2 ? 1.0f : -1.0f, i & 4 ? 1.0f : -1.0f);
//		verts[i] = obb.m_Center + dir * e;
//	}
//
//	u32 fullyInside = 0;
//	for (auto& plane : frustum.m_Planes)
//	{
//		u32 inside = 0;
//		for (auto& vert : verts)
//		{
//			if (SignedDistanceToPlane(plane, vert) >= 0.0f)
//			{
//				++inside;
//			}
//		}
//
//		if (inside == 0)
//		{
//			return IntersectionType::Outside;
//		}
//		else if (inside == 8)
//		{
//			++fullyInside;
//		}
//	}
//
//	// For a more precise test, continue with SATs for the OBB's axes,
//	// and their cross products with the frustum planes
//	// One horizontal edge from the near plane.
//	// One vertical edge from the near plane.
//	// One horizontal edge from the far plane.
//	// One vertical edge from the far plane.
//	// One diagonal edge along the left side connecting the near and far planes.
//	// One diagonal edge along the right side connecting the near and far planes.
//
//	return fullyInside == 6 ? IntersectionType::Inside : IntersectionType::Intersecting;
//}
//
//IntersectionType TestIntersection(const BvFrustum& frustum, const BvCapsule& capsule)
//{
//	auto s0 = TestIntersection(frustum, BvSphere(capsule.m_P1, capsule.m_Radius));
//	auto s1 = TestIntersection(frustum, BvSphere(capsule.m_P2, capsule.m_Radius));
//	if (s0 == IntersectionType::Inside && s1 == IntersectionType::Inside)
//	{
//		return IntersectionType::Inside;
//	}
//	else if (s0 == IntersectionType::Intersecting || s1 == IntersectionType::Intersecting)
//	{
//		return IntersectionType::Intersecting;
//	}
//
//	for (auto& plane : frustum.m_Planes)
//	{
//		auto d0 = SignedDistanceToPlane(plane, capsule.m_P1);
//		auto d1 = SignedDistanceToPlane(plane, capsule.m_P2);
//		if (d0 < -capsule.m_Radius && d1 < capsule.m_Radius)
//		{
//			return IntersectionType::Outside;
//		}
//	}
//
//	return IntersectionType::Intersecting;
//}