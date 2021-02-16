#include "BvSemaphoreVk.h"
#include "BvRenderVK/BvRenderDeviceVk.h"


BvSemaphoreVk::BvSemaphoreVk(const BvRenderDeviceVk & device)
	: m_Device(device)
{
}


BvSemaphoreVk::~BvSemaphoreVk()
{
	Destroy();
}


void BvSemaphoreVk::Create(const bool isTimelineSemaphore, const u64 initialValue)
{
	VkSemaphoreTypeCreateInfo timelineCreateInfo{};
	timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	//timelineCreateInfo.pNext = nullptr;
	if (isTimelineSemaphore)
	{
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = initialValue;
	}
	else
	{
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
		//timelineCreateInfo.initialValue = 0; // Binary semaphores should have this set to 0
	}

	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = &timelineCreateInfo;
	//createInfo.flags = 0;

	auto result = m_Device.GetDeviceFunctions().vkCreateSemaphore(m_Device.GetHandle(), &createInfo, nullptr, &m_Semaphore);
}


void BvSemaphoreVk::Destroy()
{
	if (m_Semaphore)
	{
		m_Device.GetDeviceFunctions().vkDestroySemaphore(m_Device.GetHandle(), m_Semaphore, nullptr);
		m_Semaphore = VK_NULL_HANDLE;
	}
}


void BvSemaphoreVk::Signal(const u64 value)
{
	VkSemaphoreSignalInfo signalInfo{};
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	//signalInfo.pNext = nullptr;
	signalInfo.semaphore = m_Semaphore;
	signalInfo.value = value;

	m_Device.GetDeviceFunctions().vkSignalSemaphore(m_Device.GetHandle(), &signalInfo);
}


BvSemaphore::WaitStatus BvSemaphoreVk::Wait(const u64 value, const u64 timeout)
{
	VkSemaphoreWaitInfo waitInfo{};
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	//waitInfo.pNext = nullptr;
	//waitInfo.flags = 0;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &m_Semaphore;
	waitInfo.pValues = &value;

	auto result = m_Device.GetDeviceFunctions().vkWaitSemaphores(m_Device.GetHandle(), &waitInfo, timeout * 1000000ull);
	return result == VK_SUCCESS ? WaitStatus::kSuccess : WaitStatus::kTimeout;
}


u64 BvSemaphoreVk::GetCompletedValue() const
{
	u64 value = 0;
	auto result = m_Device.GetDeviceFunctions().vkGetSemaphoreCounterValue(m_Device.GetHandle(), m_Semaphore, &value);

	return value;
}