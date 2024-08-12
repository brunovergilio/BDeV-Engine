#pragma once


#include "BvRenderDevice.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BvGPUInfo.h"
#include "BvRenderObject.h"


class BvRenderEngine : public IBvRenderObject
{
	BV_NOCOPYMOVE(BvRenderEngine);

public:
	BV_IBVOBJECT_ID("00ed0392-e7d6-4609-99c1-df9fd66010c8");

	virtual u32 GetSupportedGPUCount() const = 0;
	virtual void GetGPUInfo(u32 index, BvGPUInfo& info) const = 0;
	virtual BvRenderDevice* CreateRenderDevice(u32 gpuIndex = kU32Max) = 0;

protected:
	BvRenderEngine(IBvMemoryArena* pArena)
		: IBvRenderObject(pArena) {}
	~BvRenderEngine() {}
};