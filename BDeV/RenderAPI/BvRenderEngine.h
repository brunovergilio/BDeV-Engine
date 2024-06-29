#pragma once


#include "BvRenderDevice.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Container/BvVector.h"
#include "BvGPUInfo.h"


class BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngine);

public:
	virtual bool Initialize() = 0;
	virtual void Shutdown() = 0;

	virtual u32 GetSupportedGPUCount() const = 0;
	virtual void GetGPUInfo(u32 index, BvGPUInfo& info) const = 0;

	virtual BvRenderDevice* CreateRenderDevice(u32 gpuIndex = kU32Max) = 0;
	
	virtual const char* GetEngineName() const = 0;

protected:
	BvRenderEngine() {}
	virtual ~BvRenderEngine() = 0 {};
};