#include "BvGPUFenceVk.h"
#include "BvRenderDeviceVk.h"


BvGPUFenceVk::BvGPUFenceVk(BvRenderDeviceVk* pDevice, u64 initialValue)
	: m_Semaphore(pDevice, initialValue)
{
}


BvGPUFenceVk::~BvGPUFenceVk()
{
}


bool BvGPUFenceVk::IsDone(u64 value)
{
	return m_Semaphore.GetCompletedValue() >= value;
}


bool BvGPUFenceVk::Wait(u64 value, u64 timeout /*= kU64Max*/)
{
	return m_Semaphore.Wait(value, timeout);
}


u64 BvGPUFenceVk::GetCompletedValue()
{
	return m_Semaphore.GetCompletedValue();
}


IBvRenderDevice* BvGPUFenceVk::GetDevice()
{
	return m_Semaphore.GetDevice();
}