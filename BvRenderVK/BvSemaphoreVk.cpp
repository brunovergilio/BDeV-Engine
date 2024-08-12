#include "BvSemaphoreVk.h"
#include "BvRenderVK/BvRenderDeviceVk.h"


BvSemaphoreVk::BvSemaphoreVk()
{
}


BvSemaphoreVk::BvSemaphoreVk(VkDevice device, u64 initialValue, bool isTimelineSemaphore)
	: m_Device(device)
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
		std::swap(m_Device, rhs.m_Device);
		std::swap(m_Semaphore, rhs.m_Semaphore);
	}

	return *this;
}


BvSemaphoreVk::~BvSemaphoreVk()
{
	Destroy();
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

	auto result = vkCreateSemaphore(m_Device, &createInfo, nullptr, &m_Semaphore);
}


void BvSemaphoreVk::Destroy()
{
	if (m_Semaphore)
	{
		vkDestroySemaphore(m_Device, m_Semaphore, nullptr);
		m_Semaphore = VK_NULL_HANDLE;
	}
}


void BvSemaphoreVk::Signal(u64 value)
{
	VkSemaphoreSignalInfo signalInfo{};
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	//signalInfo.pNext = nullptr;
	signalInfo.semaphore = m_Semaphore;
	signalInfo.value = value;

	vkSignalSemaphore(m_Device, &signalInfo);
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

	auto result = vkWaitSemaphores(m_Device, &waitInfo, timeout);

	return result == VK_SUCCESS ? true : false;
}


u64 BvSemaphoreVk::GetCompletedValue()
{
	u64 value{};
	auto result = vkGetSemaphoreCounterValue(m_Device, m_Semaphore, &value);

	return value;
}


BvSemaphorePoolVk::BvSemaphorePoolVk()
{
}


BvSemaphorePoolVk::BvSemaphorePoolVk(VkDevice device)
	: m_Device(device)
{
}

BvSemaphorePoolVk::BvSemaphorePoolVk(BvSemaphorePoolVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvSemaphorePoolVk& BvSemaphorePoolVk::operator=(BvSemaphorePoolVk&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_Semaphores, rhs.m_Semaphores);
		std::swap(m_ActiveSemaphoreCount, rhs.m_ActiveSemaphoreCount);
	}

	return *this;
}


BvSemaphorePoolVk::~BvSemaphorePoolVk()
{
}


BvSemaphoreVk* BvSemaphorePoolVk::GetSemaphore()
{
	if (m_ActiveSemaphoreCount < m_Semaphores.Size())
	{
		return &m_Semaphores[m_ActiveSemaphoreCount++];
	}

	auto pSemaphore = &m_Semaphores.EmplaceBack(m_Device, 0);
	++m_ActiveSemaphoreCount;

	return pSemaphore;
}


void BvSemaphorePoolVk::Reset()
{
	m_ActiveSemaphoreCount = 0;
}