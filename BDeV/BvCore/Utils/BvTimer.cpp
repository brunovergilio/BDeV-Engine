#include "BvTimer.h"
#include "../BvDebug.h"


BvTimer::BvTimer()
{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&m_CyclesPerSec));
	BvAssert(m_CyclesPerSec != 0);

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_CurrCycle));
	m_PrevCycle = m_CurrCycle;
#else
#error "Platform not yet supported"
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)
}


BvTimer::~BvTimer()
{
}


void BvTimer::Tick()
{
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	m_PrevCycle = m_CurrCycle;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_CurrCycle));
#else
#error "Platform not yet supported"
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)
	m_Time = (static_cast<float>(m_CurrCycle - m_PrevCycle) / static_cast<float>(m_CyclesPerSec));
}


void BvTimer::Reset()
{
	m_PrevCycle = 0;
	m_CurrCycle = 0;
	m_Time = 0.0f;
}