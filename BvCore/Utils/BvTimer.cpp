#include "BvTimer.h"
#include "../Utils/BvDebug.h"


BvTimer::BvTimer()
{
	Reset();
}


BvTimer::~BvTimer()
{
}


void BvTimer::Reset()
{
	m_CurrTime = std::chrono::high_resolution_clock::now();
}


const f32 BvTimer::GetDt() const
{
	return std::chrono::duration<f32, std::milli>(std::chrono::high_resolution_clock::now() - m_CurrTime).count();
}


i64 GetCurrentTimestamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}