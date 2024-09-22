#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


class BvRenderDeviceVk;
class BvCommandBufferVk;


class BvCommandQueueVk final
{
	BV_NOCOPY(BvCommandQueueVk);

public:
	BvCommandQueueVk();
	BvCommandQueueVk(const BvRenderDeviceVk* pDevice, CommandType queueFamilyType, u32 queueFamilyIndex, u32 queueIndex);
	BvCommandQueueVk(BvCommandQueueVk&& rhs) noexcept;
	BvCommandQueueVk& operator=(BvCommandQueueVk&& rhs) noexcept;
	~BvCommandQueueVk();

	void AddWaitSemaphore(VkSemaphore waitSemaphore, u64 value);
	void Submit(const BvVector<BvCommandBufferVk*>& commandBuffers, VkSemaphore signalSemaphore, u64 value);
	void WaitIdle();

	BV_INLINE u32 GetFamilyIndex() const { return m_QueueFamilyIndex; }
	BV_INLINE u32 GetIndex() const { return m_QueueIndex; }
	BV_INLINE VkQueue GetHandle() const { return m_Queue; }
	
private:
	const BvRenderDeviceVk* m_pDevice = nullptr;
	VkQueue m_Queue = VK_NULL_HANDLE;
	u32 m_QueueFamilyIndex = 0;
	u32 m_QueueIndex = 0;
	BvVector<VkSemaphoreSubmitInfo> m_WaitSemaphores;
	BvVector<VkSemaphoreSubmitInfo> m_SignalSemaphores;
	BvVector<VkCommandBufferSubmitInfo> m_CommandBuffers;
};