#pragma once


#include "BDeV/Core/Utils/BvUtils.h"


class BvGPUFence
{
	BV_NOCOPYMOVE(BvGPUFence);

public:
	virtual bool IsDone(u64 value) = 0;
	virtual bool Wait(u64 value, u64 timeout = kU64Max) = 0;
	virtual u64 GetCompletedValue() = 0;
	
protected:
	BvGPUFence() = default;
	virtual ~BvGPUFence() = 0 {}
};