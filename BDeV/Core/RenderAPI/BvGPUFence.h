#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BvRenderDeviceObject.h"


BV_OBJECT_DEFINE_ID(BvGPUFence, "ef829631-ea35-4e5d-a4ec-21f5fe9da455");
class BvGPUFence : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(BvGPUFence);

public:
	virtual bool IsDone(u64 value) = 0;
	virtual bool Wait(u64 value, u64 timeout = kU64Max) = 0;
	virtual u64 GetCompletedValue() = 0;
	
protected:
	BvGPUFence() = default;
	~BvGPUFence() = 0 {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvGPUFence);