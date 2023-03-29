#include "BvSemaphoreVk.h"
#include "BvRenderVK/BvRenderDeviceVk.h"
#include "BvRenderVK/BvFenceVk.h"


constexpr u32 kMaxActiveSyncPoints = 8;


BvSemaphoreVk::BvSemaphoreVk(const BvRenderDeviceVk & device, bool isTimelineSemaphore, u64 initialValue)
	: m_Device(device)
{
	Create(isTimelineSemaphore, initialValue);
}


BvSemaphoreVk::~BvSemaphoreVk()
{
	Destroy();
}


void BvSemaphoreVk::Create(bool isTimelineSemaphore, u64 initialValue)
{
	VkSemaphoreTypeCreateInfoKHR timelineCreateInfo{};
	timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
	//timelineCreateInfo.pNext = nullptr;
	if (isTimelineSemaphore)
	{
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = initialValue;
	}
	else
	{
		timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
		m_SyncPoints.Reserve(kMaxActiveSyncPoints);
	}

	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = &timelineCreateInfo;

	auto result = vkCreateSemaphore(m_Device.GetHandle(), &createInfo, nullptr, &m_Semaphore);
}


void BvSemaphoreVk::Destroy()
{
	if (m_Semaphore)
	{
		vkDestroySemaphore(m_Device.GetHandle(), m_Semaphore, nullptr);
		m_Semaphore = VK_NULL_HANDLE;
	}
}


void BvSemaphoreVk::Signal(const u64 value)
{
	BvAssert(IsTimeline(), "Can't set a value in a binary semaphore!");

	VkSemaphoreSignalInfoKHR signalInfo{};
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR;
	//signalInfo.pNext = nullptr;
	signalInfo.semaphore = m_Semaphore;
	signalInfo.value = value;

	vkSignalSemaphoreKHR(m_Device.GetHandle(), &signalInfo);
}


BvSemaphore::WaitStatus BvSemaphoreVk::Wait(const u64 value, const u64 timeout)
{
	if (IsTimeline())
	{
		VkSemaphoreWaitInfoKHR waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR;
		//waitInfo.pNext = nullptr;
		//waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &m_Semaphore;
		waitInfo.pValues = &value;

		auto result = vkWaitSemaphoresKHR(m_Device.GetHandle(), &waitInfo, timeout);

		return result == VK_SUCCESS ? WaitStatus::kSuccess : WaitStatus::kTimeout;
	}
	else
	{
		BvScopedLock lock(m_SyncLock);

		while (!m_SyncPoints.IsEmpty())
		{
			auto& syncPoint = m_SyncPoints.Front();
			if (syncPoint.value > value)
			{
				break;
			}

			if (syncPoint.pFence->GetStatus() == VkResult::VK_NOT_READY)
			{
				if (syncPoint.pFence->Wait(timeout) == VkResult::VK_TIMEOUT)
				{
					return WaitStatus::kTimeout;
				}
			}
			
			SetCompletedValue(syncPoint.value);
			syncPoint.pFence->DecrementUsageCount();
			m_SyncPoints.PopFront();
		}

		return WaitStatus::kSuccess;
	}
}


u64 BvSemaphoreVk::GetCompletedValue()
{
	if (IsTimeline())
	{
		u64 value{};
		auto result = vkGetSemaphoreCounterValueKHR(m_Device.GetHandle(), m_Semaphore, &value);

		return value;
	}
	else
	{
		BvScopedLock lock(m_SyncLock);
		return GetCompletedValueInternal();
	}
}


void BvSemaphoreVk::AddSyncPoint(BvFenceVk* pFence, u64 value, u32 queueFamilyIndex, u32 queueIndex)
{
	BvAssert(!IsTimeline(), "Timeline semaphores should never call this function");

	BvScopedLock lock(m_SyncLock);

	BvAssert(value > (m_SyncPoints.IsEmpty() ? m_CurrValue.load() : m_SyncPoints.Back().value), "The new signal value must be greater than the last value");
	if (!m_SyncPoints.IsEmpty())
	{
		BvAssert(m_SyncPoints.Back().queueFamilyIndex == queueFamilyIndex && m_SyncPoints.Back().queueIndex == queueIndex,
			"This binary semaphore is being used in multiple queues, and that may cause a deadlock");
	}

	if (m_SyncPoints.Size() >= kMaxActiveSyncPoints)
	{
		GetCompletedValueInternal();
	}

	m_SyncPoints.EmplaceBack(pFence, value, queueFamilyIndex, queueIndex);
}


u64 BvSemaphoreVk::GetCompletedValueInternal()
{
	while (!m_SyncPoints.IsEmpty())
	{
		auto& syncPoint = m_SyncPoints.Front();
		if (syncPoint.pFence->GetStatus() == VkResult::VK_SUCCESS)
		{
			SetCompletedValue(syncPoint.value);
			syncPoint.pFence->DecrementUsageCount();
			m_SyncPoints.PopFront();
		}
		else
		{
			break;
		}
	}

	return m_CurrValue;
}


void BvSemaphoreVk::SetCompletedValue(u64 value)
{
	BvAssert(!IsTimeline(), "Timeline semaphores should never call this function");

	m_CurrValue = value;
}