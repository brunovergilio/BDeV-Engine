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
	void WaitIdle() override final;
	void AddSwapChain(BvSwapChainVk * pSwapChain);

	BV_INLINE u32 GetFamilyIndex() const { return m_QueueFamilyIndex; }
	BV_INLINE VkQueue GetHandle() const { return m_Queue; }

private:
	const BvRenderDeviceVk & m_Device;
	VkQueue m_Queue = VK_NULL_HANDLE;
	BvVector<VkSubmitInfo> m_SubmitInfos;
	BvVector<BvSwapChainVk *> m_SwapChains;
	u32 m_QueueFamilyIndex = 0;
	u32 m_QueueIndex = 0;
};