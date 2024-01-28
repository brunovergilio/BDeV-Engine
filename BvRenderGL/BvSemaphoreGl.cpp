#include "BvSemaphoreGl.h"


BvSemaphoreGl::BvSemaphoreGl(const BvRenderDeviceGl& device, u64 initialValue /*= 0*/)
	: BvSemaphore(), m_Device(device), m_CurrValue(initialValue)
{
}


BvSemaphoreGl::~BvSemaphoreGl()
{
}


void BvSemaphoreGl::Create(bool isTimelineSemaphore, u64 initialValue)
{
}


void BvSemaphoreGl::Destroy()
{
}


void BvSemaphoreGl::Signal(const u64 value)
{
}


BvSemaphore::WaitStatus BvSemaphoreGl::Wait(const u64 value, const u64 timeout)
{
	return BvSemaphore::WaitStatus::kSuccess;
}


u64 BvSemaphoreGl::GetCompletedValue()
{
	return m_CurrValue;
}