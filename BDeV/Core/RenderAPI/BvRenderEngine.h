#pragma once


#include "BvRenderDevice.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BvGPUInfo.h"


struct BvRenderDeviceCreateDesc
{
	u32 m_GPUIndex = kU32Max;
};


class BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngine);

public:
	virtual u32 GetSupportedGPUCount() const = 0;
	virtual void GetGPUInfo(u32 index, BvGPUInfo& info) const = 0;
	virtual BvRenderDevice* CreateRenderDevice(const BvRenderDeviceCreateDesc* pDeviceCreateDesc = nullptr) = 0;

protected:
	BvRenderEngine() {}
	~BvRenderEngine() {}
};