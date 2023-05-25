#pragma once


#include "BDeV/Utils/BvUtils.h"


class BvSemaphore
{
	BV_NOCOPYMOVE(BvSemaphore);

public:
	static constexpr u64 kMaxSemaphoreWaitTimeout = UINT64_MAX;

	enum class Type : u8
	{
		kBinary,
		kTimeline
	};

	enum class WaitStatus : u8
	{
		kSuccess,
		kTimeout
	};

	virtual void Signal(const u64 value) = 0;
	virtual WaitStatus Wait(const u64 value, const u64 timeout = kMaxSemaphoreWaitTimeout) = 0;
	virtual u64 GetCompletedValue() = 0;
	
	virtual ~BvSemaphore() = 0 {}

protected:
	BvSemaphore() {}
};