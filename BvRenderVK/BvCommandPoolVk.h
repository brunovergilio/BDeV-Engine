#pragma once


#include "BDeV/RenderAPI/BvCommandPool.h"
#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Container/BvVector.h"


class BvRenderDeviceVk;
class BvCommandBufferVk;


class BvCommandPoolVk
{
	BV_NOCOPYMOVE(BvCommandPoolVk);

public:
	BvCommandPoolVk(const BvRenderDeviceVk& device, u32 queueFamilyIndex);
	~BvCommandPoolVk();

	void Create();
	void Destroy();

	BvCommandBufferVk* GetCommandBuffer();

	void Reset();

private:
	const BvRenderDeviceVk & m_Device;
	VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	BvVector<BvCommandBufferVk*> m_CommandBuffers;
	u32 m_ActiveCommandBufferCount = 0;
	u32 m_QueueFamilyIndex = 0;
};