#pragma once


#include "BDeV/RenderAPI/BvCommandPool.h"
#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Container/BvVector.h"


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

	void AllocateCommandBuffers(u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) override;
	void FreeCommandBuffers(u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) override;

	void Reset() override;

private:
	const BvRenderDeviceVk & m_Device;
	VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	BvVector<BvCommandBufferVk*> m_UsedCommandBuffers;
	BvVector<BvCommandBufferVk*> m_FreeCommandBuffers;
};