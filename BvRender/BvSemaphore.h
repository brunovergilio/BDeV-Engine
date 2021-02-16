#pragma once


#include "BvCore/Utils/BvUtils.h"


class BvSemaphore
{
	BV_NOCOPYMOVE(BvSemaphore);

public:
	enum class WaitStatus : u8
	{
		kSuccess,
		kTimeout
	};

	virtual void Signal(const u64 value) = 0;
	virtual WaitStatus Wait(const u64 value, const u64 timeout) = 0;
	virtual u64 GetCompletedValue() const = 0;
	
	virtual ~BvSemaphore() = 0 {}

protected:
	BvSemaphore() {}
};