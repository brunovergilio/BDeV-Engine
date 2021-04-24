#pragma once


#include "BvRender/BvCommandQueue.h"
#include "BvCommonVk.h"
#include "BvCore/Container/BvVector.h"


class BvRenderDeviceVk;
class BvSwapChainVk;


class BvCommandQueueVk final : public BvCommandQueue
{
	BV_NOCOPYMOVE(BvCommandQueueVk);

public:
	BvCommandQueueVk(const BvRenderDeviceVk & device, const QueueFamilyType queueFamilyType, const u32 queueIndex);
	~BvCommandQueueVk();

	void Submit(const SubmitInfo & submitInfo) override final;
	void Execute() override;
	void WaitIdle() override final;

	BV_INLINE u32 GetFamilyIndex() const { return m_QueueFamilyIndex; }
	BV_INLINE VkQueue GetHandle() const { return m_Queue; }

private:
	const BvRenderDeviceVk & m_Device;
	VkQueue m_Queue = VK_NULL_HANDLE;
	u32 m_QueueFamilyIndex = 0;
	u32 m_QueueIndex = 0;

	struct SubmitInfoData
	{
		BvVector<VkSubmitInfo> m_SubmitInfos;
		BvVector<VkTimelineSemaphoreSubmitInfo> m_TimelineSemaphoreInfos;
		BvVector<VkSemaphore> m_WaitSemaphores;
		BvVector<u64> m_WaitSemaphoreValues;
		BvVector<VkPipelineStageFlags> m_WaitStageFlags;
		BvVector<VkSemaphore> m_SignalSemaphores;
		BvVector<u64> m_SignalSemaphoreValues;
		BvVector<VkCommandBuffer> m_CommandBuffers;
	} *m_pSubmitInfo = nullptr;
};