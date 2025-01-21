#pragma once


#include "BvRenderDevice.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BvGPUInfo.h"


struct BvRenderDeviceCreateDesc
{
	u32 m_GPUIndex = kU32Max;
	u32 m_GraphicsQueueCount = 1;
	u32 m_ComputeQueueCount = 0;
	u32 m_TransferQueueCount = 0;
	bool m_UseDebug = true;
};


class BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngine);

public:
	virtual BvRenderDevice* CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc = BvRenderDeviceCreateDesc()) = 0;
	//virtual void SetVirtualMemory(void* pStart, void* pEnd, size_t growSize = 0) = 0;
	//virtual void SetVirtualMemory(size_t maxSize, size_t growSize = 0) = 0;

	BV_INLINE const auto& GetGPUs() const { return m_GPUs; }

protected:
	BvRenderEngine() {}
	~BvRenderEngine() {}

protected:
	BvVector<BvGPUInfo> m_GPUs;
};