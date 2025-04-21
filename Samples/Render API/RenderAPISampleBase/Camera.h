#pragma once


#include "BDeV/Core/Math/BvMath.h"


class Camera
{
public:
	Camera();
	BV_DEFAULTCOPYMOVE(Camera);

	void SetPos(BvVec3 pos);
	void SetPos(f32 x, f32 y, f32 z);

	void Walk(f32 val);
	void Strafe(f32 val);
	void Fly(f32 val);

	void Pitch(f32 rad);
	void Yaw(f32 rad);
	void Roll(f32 rad);

	void RotateX(f32 rad);
	void RotateY(f32 rad);
	void RotateZ(f32 rad);

	void SetPerspective(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY);

	void Update();

	BV_INLINE BvVec3 GetPos() const { return m_Pos; }
	BV_INLINE BvVec3 GetRight() const { return m_Right; }
	BV_INLINE BvVec3 GetUp() const { return m_Up; }
	BV_INLINE BvVec3 GetLook() const { return m_Look; }
	BV_INLINE BvMatrix GetView() const { return m_View; }
	BV_INLINE BvMatrix GetProj() const { return m_Proj; }
	BV_INLINE BvMatrix GetViewProj() const { return m_View * m_Proj; }

private:
	BvMatrix m_View;
	BvMatrix m_Proj;

	BvVec3 m_Pos = BvVec3(0.0f, 0.0f, 0.0f);
	BvVec3 m_Right = BvVec3(1.0f, 0.0f, 0.0f);
	BvVec3 m_Up = BvVec3(0.0f, 1.0f, 0.0f);
	BvVec3 m_Look = BvVec3(0.0f, 0.0f, 1.0f);

	f32 m_NearZ = 0.1f;
	f32 m_FarZ = 100.0f;
	f32 m_AspectRatio = 1.77777778f;
	f32 m_FovY = kPiDiv4;

	bool m_Dirty = true;
};