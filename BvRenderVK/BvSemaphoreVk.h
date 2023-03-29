#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Render/BvSemaphore.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/Container/BvQueue.h"
#include "BDeV/System/Threading/Windows/BvSyncWindows.h"


class BvRenderDeviceVk;
class BvFenceVk;


struct BvSyncPointVk
{
	BvSyncPointVk() = default;
	explicit BvSyncPointVk(BvFenceVk* pFence, u64 value, u32 queueFamilyIndex, u32 queueIndex)
		: pFence(pFence), value(value), queueFamilyIndex(queueFamilyIndex), queueIndex(queueIndex) {}

	BvFenceVk* pFence;
	u64 value;
	u32 queueFamilyIndex;
	u32 queueIndex;
};


class BvSemaphoreVk final : public BvSemaphore
{
	BV_NOCOPYMOVE(BvSemaphoreVk);

public:
	BvSemaphoreVk(const BvRenderDeviceVk & device, bool isTimelineSemaphore = true, u64 initialValue = 0);
	~BvSemaphoreVk();

	void Create(bool isTimelineSemaphore, u64 initialValue);
	void Destroy();

	void Signal(const u64 value) override final;
	WaitStatus Wait(const u64 value, const u64 timeout) override final;
	u64 GetCompletedValue() override final;

	void AddSyncPoint(BvFenceVk* pFence, u64 value, u32 queueFamilyIndex, u32 queueIndex);

	BV_INLINE VkSemaphore GetHandle() const { return m_Semaphore; }
	BV_INLINE bool IsTimeline() const { return m_SyncPoints.Capacity() == 0; }
	
private:
	u64 GetCompletedValueInternal();
	void SetCompletedValue(u64 value);

protected:
	const BvRenderDeviceVk & m_Device;
	VkSemaphore m_Semaphore = VK_NULL_HANDLE;
	
	// For a binary semaphore, in order to keep the same concept of waiting and
	// signaling a value, we need to store information about the submitted
	// semaphores, along with the fence that was used in that submission
	BvMutex m_SyncLock;
	BvQueue<BvSyncPointVk> m_SyncPoints{};
	std::atomic<u64> m_CurrValue = 0;
};