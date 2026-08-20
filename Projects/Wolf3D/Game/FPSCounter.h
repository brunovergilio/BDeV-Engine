#pragma once


#include "BDeV/Core/BvCore.h"


class FPSCounter
{
public:
	FPSCounter() = default;
	~FPSCounter() = default;

	inline void Update(f32 dt)
	{
		m_Counter++;
		m_Dt += dt;
		if (m_Dt >= 1.0f)
		{
			m_FPS = m_Counter;
			m_Counter = 0;
			m_Dt = 0.0f;
		}
	}

	inline u32 GetFPS() const { return m_FPS; }

private:
	u32 m_FPS = 0;
	u32 m_Counter = 0;
	f32 m_Dt = 0;
};