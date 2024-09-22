#pragma once


#include "BDeV/RenderAPI/BvCommandQueue.h"
#include "BvCommonGl.h"
#include "BDeV/Container/BvVector.h"


class BvRenderDeviceGl;


class BvCommandQueueGl final : public BvCommandQueue
{
	BV_NOCOPYMOVE(BvCommandQueueGl);

public:
	BvCommandQueueGl(const BvRenderDeviceGl& device, const CommandType queueFamilyType, const u32 queueIndex);
	~BvCommandQueueGl();

	void Submit(const SubmitInfo& submitInfo) override;
	void Execute() override;
	void WaitIdle() override;

private:
	const BvRenderDeviceGl& m_Device;
};