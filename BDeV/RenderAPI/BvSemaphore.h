#pragma once


#include "BDeV/Utils/BvUtils.h"


class BvSemaphore
{
	BV_NOCOPYMOVE(BvSemaphore);

public:
	static constexpr u64 kMaxSemaphoreWaitTimeout = UINT64_MAX;

	enum class WaitStatus : u8
	{
		kSuccess,
		kTimeout
	};

	virtual void Signal(u64 value) = 0;
	virtual WaitStatus Wait(u64 value, u64 timeout = kMaxSemaphoreWaitTimeout) = 0;
	virtual u64 GetCompletedValue() = 0;

protected:
	BvSemaphore() {}
	virtual ~BvSemaphore() = 0 {}
};