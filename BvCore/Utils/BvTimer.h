#pragma once


#include "BvCore/BvPlatform.h"
#include <chrono>


class BvTimer
{
public:
	BvTimer();
	BvTimer(const BvTimer & rhs) = default;
	BvTimer(BvTimer && rhs) = default;
	BvTimer & operator=(const BvTimer & rhs) = default;
	BvTimer & operator=(BvTimer && rhs) = default;
	~BvTimer();

	void Tick();
	void Reset();

	BV_INLINE const f32 GetDt() const { return std::chrono::duration<double, std::milli>(m_CurrCycle - m_PrevCycle).count(); }

protected:
	std::chrono::high_resolution_clock::time_point m_PrevCycle;
	std::chrono::high_resolution_clock::time_point m_CurrCycle;
};