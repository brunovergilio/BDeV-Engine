#pragma once


#include "BvRender/BvCommandPool.h"
#include "BvRenderVK/BvCommonVk.h"
#include "BvCore/Container/BvVector.h"


class BvRenderDeviceVk;
class BvCommandBufferVk;


class BvCommandPoolVk final : public BvCommandPool
{
	BV_NOCOPYMOVE(BvCommandPoolVk);

public:
	BvCommandPoolVk(const BvRenderDeviceVk & device, const CommandPoolDesc & commandPoolDesc);
	~BvCommandPoolVk();

	void Create();
	void Destroy();

	void AllocateCommandBuffers(const u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) override final;
	void FreeCommandBuffers(const u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) override final;

	void Reset() override final;

private:
	const BvRenderDeviceVk & m_Device;
	VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	BvVector<BvCommandBufferVk*> m_CommandBuffers;
};