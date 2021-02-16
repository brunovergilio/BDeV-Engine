#pragma once


#include "BvCore/Utils/BvUtils.h"


constexpr u64 kMaxFenceWaitTimeout = UINT64_MAX;


class BvFence
{
	BV_NOCOPYMOVE(BvFence);

public:
	enum class State
	{
		kUnsignaled,
		kSignaled
	};

	virtual void Reset() = 0;
	virtual bool Wait(const u64 timeout = kMaxFenceWaitTimeout) = 0;

protected:
	BvFence(const State initialState = State::kUnsignaled) {}
	virtual ~BvFence() = 0 {}
};