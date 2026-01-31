#include "BvGPUFenceVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"


BvGPUFenceVk::BvGPUFenceVk(BvRenderDeviceVk* pDevice, VkSemaphore semaphore)
	: m_pDevice(pDevice), m_Semaphore(semaphore)
{
}


BvGPUFenceVk::~BvGPUFenceVk()
{
	Destroy();
}


void BvGPUFenceVk::Signal(u64 value)
{
	VkSemaphoreSignalInfo signalInfo{};
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	//signalInfo.pNext = nullptr;
	signalInfo.semaphore = m_Semaphore;
	signalInfo.value = value;

	vkSignalSemaphore(*m_pDevice, &signalInfo);
}


bool BvGPUFenceVk::Wait(u64 value, u64 timeout /*= kU64Max*/)
{
	if (IsDone(value))
	{
		return true;
	}

	VkSemaphoreWaitInfo waitInfo{};
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	//waitInfo.pNext = nullptr;
	//waitInfo.flags = 0;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &m_Semaphore;
	waitInfo.pValues = &value;

	auto result = vkWaitSemaphores(*m_pDevice, &waitInfo, timeout);

	return result == VK_SUCCESS ? true : false;
}


bool BvGPUFenceVk::IsDone(u64 value)
{
	return GetCompletedValue() >= value;
}


u64 BvGPUFenceVk::GetCompletedValue()
{
	u64 value{};
	auto result = vkGetSemaphoreCounterValue(*m_pDevice, m_Semaphore, &value);

	return value;
}


void BvGPUFenceVk::Destroy()
{
	if (m_Semaphore)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Semaphore);
		m_Semaphore = VK_NULL_HANDLE;
	}
}