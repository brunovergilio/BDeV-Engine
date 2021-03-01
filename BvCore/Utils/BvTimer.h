#pragma once


#include "BvCore/BvCore.h"
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

	void Reset();

	const f32 GetDt() const;

protected:
	std::chrono::high_resolution_clock::time_point m_CurrTime;
};

i64 GetCurrentTimestamp();