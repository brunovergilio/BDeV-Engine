#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"


class IBvRenderDeviceVk;
class BvCommandBufferVk;
class BvFrameDataVk;


class BvCommandPoolVk final
{
	BV_NOCOPY(BvCommandPoolVk);

public:
	BvCommandPoolVk();
	BvCommandPoolVk(IBvRenderDeviceVk* pDevice, u32 queueFamilyIndex);
	BvCommandPoolVk(BvCommandPoolVk&& rhs) noexcept;
	BvCommandPoolVk& operator=(BvCommandPoolVk&& rhs) noexcept;
	~BvCommandPoolVk();

	void Create();
	void Destroy();

	BvCommandBufferVk* GetCommandBuffer(BvFrameDataVk* pFrameData);

	void Reset();

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	BvVector<BvCommandBufferVk*> m_CommandBuffers;
	u32 m_ActiveCommandBufferCount = 0;
	u32 m_QueueFamilyIndex = 0;
};