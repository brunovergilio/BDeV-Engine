#include "BvTimer.h"
#include "../BvDebug.h"


BvTimer::BvTimer()
{
	Reset();
}


BvTimer::~BvTimer()
{
}


void BvTimer::Tick()
{
	m_PrevCycle = m_CurrCycle;
	m_CurrCycle = std::chrono::high_resolution_clock::now();
}


void BvTimer::Reset()
{
	m_PrevCycle = m_CurrCycle = std::chrono::high_resolution_clock::now();
}