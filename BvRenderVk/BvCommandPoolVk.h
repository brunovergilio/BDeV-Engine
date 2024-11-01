#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Container/BvVector.h"


class BvRenderDeviceVk;
class BvCommandBufferVk;
class BvFrameDataVk;


class BvCommandPoolVk final
{
	BV_NOCOPY(BvCommandPoolVk);

public:
	BvCommandPoolVk();
	BvCommandPoolVk(const BvRenderDeviceVk* pDevice, u32 queueFamilyIndex);
	BvCommandPoolVk(BvCommandPoolVk&& rhs) noexcept;
	BvCommandPoolVk& operator=(BvCommandPoolVk&& rhs) noexcept;
	~BvCommandPoolVk();

	void Create();
	void Destroy();

	BvCommandBufferVk* GetCommandBuffer(BvFrameDataVk* pFrameData);

	void Reset();

private:
	const BvRenderDeviceVk* m_pDevice = nullptr;
	VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	BvVector<BvCommandBufferVk*> m_CommandBuffers;
	u32 m_ActiveCommandBufferCount = 0;
	u32 m_QueueFamilyIndex = 0;
};