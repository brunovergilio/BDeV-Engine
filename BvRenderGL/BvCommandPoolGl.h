#pragma once


#include "BDeV/RenderAPI/BvCommandPool.h"
#include "BvRenderGl/BvCommonGl.h"
#include "BDeV/Container/BvVector.h"


class BvRenderDeviceGl;
class BvCommandBufferGl;


class BvCommandPoolGl final : public BvCommandPool
{
	BV_NOCOPYMOVE(BvCommandPoolGl);

public:
	BvCommandPoolGl(const BvRenderDeviceGl& device, const CommandPoolDesc& commandPoolDesc);
	~BvCommandPoolGl();

	void AllocateCommandBuffers(u32 commandBufferCount, BvCommandBuffer** ppCommandBuffers) override;
	void FreeCommandBuffers(u32 commandBufferCount, BvCommandBuffer** ppCommandBuffers) override;

	void Reset() override;

private:
	const BvRenderDeviceGl& m_Device;
	BvVector<BvCommandBufferGl*> m_UsedCommandBuffers;
	BvVector<BvCommandBufferGl*> m_FreeCommandBuffers;
};