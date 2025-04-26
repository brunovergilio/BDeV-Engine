#include "Camera.h"


Camera::Camera()
{
	SetPerspective(m_NearZ, m_FarZ, m_AspectRatio, m_FovY);
	Update();
}


void Camera::SetPos(BvVec3 pos)
{
	m_Pos = pos;
	m_Dirty = true;
}


void Camera::SetPos(f32 x, f32 y, f32 z)
{
	SetPos(BvVec3(x, y, z));
}


void Camera::Walk(f32 val)
{
	m_Pos = (m_Look * val) + m_Pos;
	m_Dirty = true;
}


void Camera::Strafe(f32 val)
{
	m_Pos = (m_Right * val) + m_Pos;
	m_Dirty = true;
}


void Camera::Fly(f32 val)
{
	m_Pos = (m_Up * val) + m_Pos;
	m_Dirty = true;
}


void Camera::Pitch(f32 rad)
{
	BvQuat r(m_Right, rad);

	m_Up *= r;
	m_Look *= r;

	m_Dirty = true;
}


void Camera::Yaw(f32 rad)
{
	BvQuat r(m_Up, rad);

	m_Right *= r;
	m_Look *= r;

	m_Dirty = true;
}


void Camera::Roll(f32 rad)
{
	BvQuat r(m_Look, rad);

	m_Right *= r;
	m_Up *= r;

	m_Dirty = true;
}


void Camera::RotateX(f32 rad)
{
	BvQuat r(BvVec3(VectorUnitX()), rad);

	m_Right *= r;
	m_Up *= r;
	m_Look *= r;

	m_Dirty = true;
}


void Camera::RotateY(f32 rad)
{
	BvQuat r(BvVec3(VectorUnitY()), rad);

	m_Right *= r;
	m_Up *= r;
	m_Look *= r;

	m_Dirty = true;
}


void Camera::RotateZ(f32 rad)
{
	BvQuat r(BvVec3(VectorUnitZ()), rad);

	m_Right *= r;
	m_Up *= r;
	m_Look *= r;

	m_Dirty = true;
}


void Camera::SetPerspective(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	m_NearZ = nearZ;
	m_FarZ = farZ;
	m_AspectRatio = aspectRatio;
	m_FovY = fovY;

	m_Proj = m_FlipViewportY ? BvMatrix::PerspectiveLH_VK(m_NearZ, m_FarZ, m_AspectRatio, m_FovY)
		: BvMatrix::PerspectiveLH_DX(m_NearZ, m_FarZ, m_AspectRatio, m_FovY);
}


void Camera::SetFlipViewportY(bool flip)
{
	m_FlipViewportY = flip;
}


void Camera::Update()
{
	if (!m_Dirty)
	{
		return;
	}
	BvVec3 l(m_Look.Normalize());
	BvVec3 u(l.Cross(m_Right).Normalize());
	BvVec3 r(u.Cross(l));

	f32 x = -(m_Pos.Dot(r));
	f32 y = -(m_Pos.Dot(u));
	f32 z = -(m_Pos.Dot(l));

	m_Right = r;
	m_Up = u;
	m_Look = l;

	m_View.SetX(BvVec4(r, 0.0f));
	m_View.SetY(BvVec4(u, 0.0f));
	m_View.SetZ(BvVec4(l, 0.0f));
	m_View.SetW(BvVec4(VectorZero()));
	m_View.SetTranspose();

	m_View.SetW(BvVec4(x, y, z, 1.0f));

	m_Dirty = false;
}