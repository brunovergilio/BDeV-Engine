#include "BvGPUFenceVk.h"
#include "BvRenderDeviceVk.h"


BV_VK_DEVICE_RES_DEF(BvGPUFenceVk)


BvGPUFenceVk::BvGPUFenceVk(BvRenderDeviceVk* pDevice, u64 initialValue, bool isTimeline)
	: m_pDevice(pDevice)
{
	Create(initialValue, isTimeline);
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

	vkSignalSemaphore(m_pDevice->GetHandle(), &signalInfo);
}


bool BvGPUFenceVk::Wait(u64 value, u64 timeout /*= kU64Max*/)
{
	VkSemaphoreWaitInfo waitInfo{};
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	//waitInfo.pNext = nullptr;
	//waitInfo.flags = 0;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &m_Semaphore;
	waitInfo.pValues = &value;

	auto result = vkWaitSemaphores(m_pDevice->GetHandle(), &waitInfo, timeout);

	return result == VK_SUCCESS ? true : false;
}


bool BvGPUFenceVk::IsDone(u64 value)
{
	return GetCompletedValue() >= value;
}


u64 BvGPUFenceVk::GetCompletedValue()
{
	u64 value{};
	auto result = vkGetSemaphoreCounterValue(m_pDevice->GetHandle(), m_Semaphore, &value);

	return value;
}


void BvGPUFenceVk::Create(u64 initialValue, bool isTimelineSemaphore)
{
	VkSemaphoreTypeCreateInfo timelineCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	if (isTimelineSemaphore) [[likely]]
	{
		//timelineCreateInfo.pNext = nullptr;
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = initialValue;
	}

	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = isTimelineSemaphore ? &timelineCreateInfo : nullptr;

	auto result = vkCreateSemaphore(m_pDevice->GetHandle(), &createInfo, nullptr, &m_Semaphore);
}


void BvGPUFenceVk::Destroy()
{
	if (m_Semaphore)
	{
		vkDestroySemaphore(m_pDevice->GetHandle(), m_Semaphore, nullptr);
		m_Semaphore = VK_NULL_HANDLE;
	}
}