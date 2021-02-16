#pragma once


#include "BvRender/BvFence.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvFenceVk final : public BvFence
{
	BV_NOCOPYMOVE(BvFenceVk);

public:
	BvFenceVk(const BvRenderDeviceVk & device, const State initialState);
	~BvFenceVk();

	void Create();
	void Destroy();

	void Reset();
	bool Wait(const u64 timeout);

	BV_INLINE VkFence GetHandle() const { return m_Fence; }

private:
	const BvRenderDeviceVk & m_Device;
	VkFence m_Fence = VK_NULL_HANDLE;
	State m_InitialState = State::kUnsignaled;
};