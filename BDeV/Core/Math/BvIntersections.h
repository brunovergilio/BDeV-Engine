#pragma once


#include "BvPrimitives.h"


enum class IntersectionType
{
	Outside,
	Intersecting,
	Inside
};


bool Intersects(const BvRay& ray, const BvTriangle& triangle, f32& t, f32& u, f32& v);
BV_INLINE bool Intersects(const BvRay& ray, const BvTriangle& triangle) { f32 t, u, v; return Intersects(ray, triangle, t, u, v); }
bool Intersects(const BvRay& ray, const BvSphere& sphere, f32& t);
BV_INLINE bool Intersects(const BvRay& ray, const BvSphere& sphere) { f32 t; return Intersects(ray, sphere, t); }
bool Intersects(const BvRay& ray, const BvPlane& plane, f32& t);
BV_INLINE bool Intersects(const BvRay& ray, const BvPlane& plane) { f32 t; return Intersects(ray, plane, t); }
bool Intersects(const BvRay& ray, const BvAABB& aabb, f32& tMin, f32& tMax);
BV_INLINE bool Intersects(const BvRay& ray, const BvAABB& aabb) { f32 min, max; return Intersects(ray, aabb, min, max); }
bool Intersects(const BvRay& ray, const BvOBB& obb, f32& tMin, f32& tMax);
BV_INLINE bool Intersects(const BvRay& ray, const BvOBB& obb) { f32 min, max; return Intersects(ray, obb, min, max); }
bool Intersects(const BvRay& ray, const BvFrustum& frustum, f32& tMin, f32& tMax);
BV_INLINE bool Intersects(const BvRay& ray, const BvFrustum& frustum) { f32 min, max; return Intersects(ray, frustum, min, max); }
bool Intersects(const BvRay& ray, const BvCapsule& capsule, f32& t);
BV_INLINE bool Intersects(const BvRay& ray, const BvCapsule& capsule) { f32 t; return Intersects(ray, capsule, t); }

BV_INLINE bool Intersects(const BvTriangle& triangle, const BvRay& ray) { return Intersects(ray, triangle); }
BV_INLINE bool Intersects(const BvSphere& sphere, const BvRay& ray) { return Intersects(ray, sphere); }
BV_INLINE bool Intersects(const BvPlane& plane, const BvRay& ray) { return Intersects(ray, plane); }
BV_INLINE bool Intersects(const BvAABB& aabb, const BvRay& ray) { return Intersects(ray, aabb); }
BV_INLINE bool Intersects(const BvOBB& obb, const BvRay& ray) { return Intersects(ray, obb); }
BV_INLINE bool Intersects(const BvFrustum& frustum, const BvRay& ray) { return Intersects(ray, frustum); }
BV_INLINE bool Intersects(const BvCapsule& capsule, const BvRay& ray) { return Intersects(ray, capsule); }

bool Intersects(const BvTriangle& triangle1, const BvTriangle& triangle2);
bool Intersects(const BvTriangle& triangle, const BvSphere& sphere);
bool Intersects(const BvTriangle& triangle, const BvPlane& plane);
bool Intersects(const BvTriangle& triangle, const BvAABB& aabb);
bool Intersects(const BvTriangle& triangle, const BvOBB& obb);
bool Intersects(const BvTriangle& triangle, const BvFrustum& frustum);
bool Intersects(const BvTriangle& triangle, const BvCapsule& capsule);

BV_INLINE bool Intersects(const BvSphere& sphere, const BvTriangle& triangle) { return Intersects(triangle, sphere); }
BV_INLINE bool Intersects(const BvPlane& plane, const BvTriangle& triangle) { return Intersects(triangle, plane); }
BV_INLINE bool Intersects(const BvAABB& aabb, const BvTriangle& triangle) { return Intersects(triangle, aabb); }
BV_INLINE bool Intersects(const BvOBB& obb, const BvTriangle& triangle) { return Intersects(triangle, obb); }
BV_INLINE bool Intersects(const BvFrustum& frustum, const BvTriangle& triangle) { return Intersects(triangle, frustum); }
BV_INLINE bool Intersects(const BvCapsule& capsule, const BvTriangle& triangle) { return Intersects(triangle, capsule); }

bool Intersects(const BvSphere& sphere, const BvSphere& sphere1);
bool Intersects(const BvSphere& sphere, const BvPlane& plane);
bool Intersects(const BvSphere& sphere, const BvAABB& aabb);
bool Intersects(const BvSphere& sphere, const BvOBB& obb);
bool Intersects(const BvSphere& sphere, const BvFrustum& frustum);
bool Intersects(const BvSphere& sphere, const BvCapsule& capsule);

BV_INLINE bool Intersects(const BvPlane& plane, const BvSphere& sphere) { return Intersects(sphere, plane); }
BV_INLINE bool Intersects(const BvAABB& aabb, const BvSphere& sphere) { return Intersects(sphere, aabb); }
BV_INLINE bool Intersects(const BvOBB& obb, const BvSphere& sphere) { return Intersects(sphere, obb); }
BV_INLINE bool Intersects(const BvFrustum& frustum, const BvSphere& sphere) { return Intersects(sphere, frustum); }
BV_INLINE bool Intersects(const BvCapsule& capsule, const BvSphere& sphere) { return Intersects(sphere, capsule); }

bool Intersects(const BvPlane& plane, const BvAABB& aabb);
bool Intersects(const BvPlane& plane, const BvOBB& obb);
bool Intersects(const BvPlane& plane, const BvCapsule& capsule);

BV_INLINE bool Intersects(const BvAABB& aabb, const BvPlane& plane) { return Intersects(plane, aabb); }
BV_INLINE bool Intersects(const BvOBB& obb, const BvPlane& plane) { return Intersects(plane, obb); }
BV_INLINE bool Intersects(const BvCapsule& capsule, const BvPlane& plane) { return Intersects(plane, capsule); }

bool Intersects(const BvAABB& aabb1, const BvAABB& aabb2);
bool Intersects(const BvAABB& aabb, const BvOBB& obb);
bool Intersects(const BvAABB& aabb, const BvFrustum& frustum);
bool Intersects(const BvAABB& aabb, const BvCapsule& capsule);

BV_INLINE bool Intersects(const BvOBB& obb, const BvAABB& aabb) { return Intersects(aabb, obb); }
BV_INLINE bool Intersects(const BvCapsule& capsule, const BvAABB& aabb) { return Intersects(aabb, capsule); }

bool Intersects(const BvOBB& obb1, const BvOBB& obb2);
bool Intersects(const BvOBB& obb, const BvFrustum& frustum);
bool Intersects(const BvOBB& obb, const BvCapsule& capsule);

BV_INLINE bool Intersects(const BvCapsule& capsule, const BvOBB& obb) { return Intersects(obb, capsule); }

IntersectionType TestIntersection(const BvFrustum& frustum, const BvTriangle& triangle);
IntersectionType TestIntersection(const BvFrustum& frustum, const BvSphere& sphere);
IntersectionType TestIntersection(const BvFrustum& frustum, const BvAABB& aabb);
IntersectionType TestIntersection(const BvFrustum& frustum, const BvOBB& obb);
IntersectionType TestIntersection(const BvFrustum& frustum, const BvCapsule& capsule);