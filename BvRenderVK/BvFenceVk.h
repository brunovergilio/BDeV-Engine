#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Utils/BvUtils.h"
#include <atomic>
#include "BDeV/Container/BvVector.h"
#include "BDeV/System/Threading/Windows/BvSyncWindows.h"


class BvRenderDeviceVk;


class BvFenceVk final
{
	BV_NOCOPYMOVE(BvFenceVk);

public:
	BvFenceVk(const BvRenderDeviceVk& device, VkFenceCreateFlags createFlags = 0);
	~BvFenceVk();

	void Create(VkFenceCreateFlags createFlags);
	void Destroy();

	VkResult Wait(const u64 timeout) const;
	void Reset() const;
	VkResult GetStatus() const;

	bool Acquire();
	
	BV_INLINE VkFence GetHandle() const { return m_Fence; }
	
	void IncrementUsageCount(i32 usageCount = 1);
	void DecrementUsageCount();

protected:
	const BvRenderDeviceVk& m_Device;
	VkFence m_Fence = VK_NULL_HANDLE;
	std::atomic<i32> m_UsageCount = 0;
	std::atomic_flag m_FenceFlag;
};


class BvFenceManagerVk
{
public:
	BvFenceManagerVk();
	~BvFenceManagerVk();

	BvFenceVk* GetFence(const BvRenderDeviceVk& device);
	void Destroy();

private:
	BvVector<BvFenceVk*> m_Fences;
	BvSpinlock m_Lock;
};


BvFenceManagerVk* GetFenceManager();