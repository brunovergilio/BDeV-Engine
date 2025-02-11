#include "BvSemaphoreVk.h"
#include "BvRenderDeviceVk.h"


BvSemaphoreVk::BvSemaphoreVk()
{
}


BvSemaphoreVk::BvSemaphoreVk(BvRenderDeviceVk* pDevice, u64 initialValue, bool isTimelineSemaphore)
	: m_pDevice(pDevice)
{
	Create(initialValue, isTimelineSemaphore);
}


BvSemaphoreVk::BvSemaphoreVk(BvSemaphoreVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvSemaphoreVk& BvSemaphoreVk::operator=(BvSemaphoreVk&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_pDevice = rhs.m_pDevice;
		std::swap(m_Semaphore, rhs.m_Semaphore);
	}

	return *this;
}


BvSemaphoreVk::~BvSemaphoreVk()
{
	Destroy();
}


void BvSemaphoreVk::Signal(u64 value)
{
	VkSemaphoreSignalInfo signalInfo{};
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	//signalInfo.pNext = nullptr;
	signalInfo.semaphore = m_Semaphore;
	signalInfo.value = value;

	vkSignalSemaphore(m_pDevice->GetHandle(), &signalInfo);
}


bool BvSemaphoreVk::Wait(u64 value, u64 timeout)
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


u64 BvSemaphoreVk::GetCompletedValue()
{
	u64 value{};
	auto result = vkGetSemaphoreCounterValue(m_pDevice->GetHandle(), m_Semaphore, &value);

	return value;
}


void BvSemaphoreVk::Create(u64 initialValue, bool isTimelineSemaphore)
{
	VkSemaphoreTypeCreateInfo timelineCreateInfo{};
	if (isTimelineSemaphore)
	{
		timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		//timelineCreateInfo.pNext = nullptr;
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = initialValue;
	}

	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = isTimelineSemaphore ? &timelineCreateInfo : nullptr;

	auto result = vkCreateSemaphore(m_pDevice->GetHandle(), &createInfo, nullptr, &m_Semaphore);
}


void BvSemaphoreVk::Destroy()
{
	if (m_Semaphore)
	{
		vkDestroySemaphore(m_pDevice->GetHandle(), m_Semaphore, nullptr);
		m_Semaphore = VK_NULL_HANDLE;
	}
}