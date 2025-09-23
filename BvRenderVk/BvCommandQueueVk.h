#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


class BvRenderDeviceVk;
class BvCommandBufferVk;


class BvCommandQueueVk final
{
	BV_NOCOPY(BvCommandQueueVk);

public:
	BvCommandQueueVk();
	BvCommandQueueVk(VkDevice device, u32 queueFamilyIndex, u32 queueIndex);
	BvCommandQueueVk(BvCommandQueueVk&& rhs) noexcept;
	BvCommandQueueVk& operator=(BvCommandQueueVk&& rhs) noexcept;
	~BvCommandQueueVk();

	void AddWaitSemaphore(VkSemaphore waitSemaphore, u64 value);
	void AddSignalSemaphore(VkSemaphore signalSemaphore, u64 value);
	void Submit(const BvVector<BvCommandBufferVk*>& commandBuffers, VkSemaphore signalSemaphore, u64 value);
	void WaitIdle();

	BV_INLINE VkQueue GetHandle() const { return m_Queue; }
	
private:
	VkDevice m_Device = VK_NULL_HANDLE;
	VkQueue m_Queue = VK_NULL_HANDLE;
	BvVector<VkSemaphoreSubmitInfo> m_WaitSemaphores;
	BvVector<VkSemaphoreSubmitInfo> m_SignalSemaphores;
	BvVector<VkCommandBufferSubmitInfo> m_CommandBuffers;
};