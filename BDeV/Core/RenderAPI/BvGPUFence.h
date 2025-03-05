#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BvRenderDeviceObject.h"


BV_OBJECT_DEFINE_ID(IBvGPUFence, "ef829631-ea35-4e5d-a4ec-21f5fe9da455");
class IBvGPUFence : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvGPUFence);

public:
	virtual bool IsDone(u64 value) = 0;
	virtual bool Wait(u64 value, u64 timeout = kU64Max) = 0;
	virtual u64 GetCompletedValue() = 0;
	
protected:
	IBvGPUFence() {};
	~IBvGPUFence() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvGPUFence);