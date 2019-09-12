#pragma once


#include "../BvDefines.h"


class BvTimer
{
public:
	BvTimer();
	BvTimer(const BvTimer & rhs) = default;
	BvTimer(BvTimer && rhs) = default;
	BvTimer & operator =(const BvTimer & rhs) = default;
	BvTimer & operator =(BvTimer && rhs) = default;
	~BvTimer();

	void Tick();
	void Reset();

	BV_INLINE const i64 GetClockCycles() const { return m_CurrCycle - m_PrevCycle; }
	BV_INLINE const f32 GetDt() const { return m_Time; }

protected:
	i64 m_CyclesPerSec = 0;
	i64 m_PrevCycle = 0;
	i64 m_CurrCycle = 0;
	f32 m_Time = 0.0f;
};