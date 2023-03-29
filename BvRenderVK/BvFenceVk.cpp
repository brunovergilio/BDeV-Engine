#include "BvFenceVk.h"
#include "BvRenderDeviceVk.h"


BvFenceVk::BvFenceVk(const BvRenderDeviceVk& device, VkFenceCreateFlags createFlags)
	: m_Device(device)
{
	Create(createFlags);
}


BvFenceVk::~BvFenceVk()
{
	Destroy();
}


void BvFenceVk::Create(VkFenceCreateFlags createFlags)
{
	VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceCI.flags = createFlags;

	vkCreateFence(m_Device.GetHandle(), &fenceCI, nullptr, &m_Fence);
}


void BvFenceVk::Destroy()
{
	if (m_Fence)
	{
		vkDestroyFence(m_Device.GetHandle(), m_Fence, nullptr);
		m_Fence = nullptr;
	}
}


VkResult BvFenceVk::Wait(const u64 timeout) const
{
	return vkWaitForFences(m_Device.GetHandle(), 1, &m_Fence, VK_TRUE, timeout);
}


void BvFenceVk::Reset() const
{
	vkResetFences(m_Device.GetHandle(), 1, &m_Fence);
}


VkResult BvFenceVk::GetStatus() const
{
	return vkGetFenceStatus(m_Device.GetHandle(), m_Fence);
}


bool BvFenceVk::Acquire()
{
	return !m_FenceFlag.test_and_set();
}


void BvFenceVk::IncrementUsageCount(i32 usageCount)
{
	m_UsageCount += usageCount;
}


void BvFenceVk::DecrementUsageCount()
{
	if (m_UsageCount.fetch_sub(1) == 1)
	{
		m_FenceFlag.clear();
	}
}



BvFenceManagerVk::BvFenceManagerVk()
{
}


BvFenceManagerVk::~BvFenceManagerVk()
{
}


BvFenceVk* BvFenceManagerVk::GetFence(const BvRenderDeviceVk& device)
{
	BvScopedLock lock(m_Lock);

	BvFenceVk* pAcquiredFence = nullptr;
	for (auto&& pFence : m_Fences)
	{
		if (pFence->Acquire())
		{
			pAcquiredFence = pFence;
			if (pAcquiredFence->GetStatus() == VK_SUCCESS)
			{
				pAcquiredFence->Wait(kU64Max);
			}
			break;
		}
	}

	if (pAcquiredFence == nullptr)
	{
		pAcquiredFence = m_Fences.EmplaceBack(new BvFenceVk(device));
		pAcquiredFence->Acquire();
	}
	pAcquiredFence->Reset();

	return pAcquiredFence;
}


void BvFenceManagerVk::Destroy()
{
	for (auto&& pFence : m_Fences)
	{
		delete pFence;
	}
	m_Fences.Clear();
}


BvFenceManagerVk* GetFenceManager()
{
	static BvFenceManagerVk instance;
	return &instance;
}