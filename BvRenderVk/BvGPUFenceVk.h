#pragma once


#include "BDeV/Core/RenderAPI/BvGPUFence.h"
#include "BvSemaphoreVk.h"


class BvRenderDeviceVk;


class BvGPUFenceVk final : public BvGPUFence
{
	BV_NOCOPYMOVE(BvGPUFenceVk);

public:
	BvGPUFenceVk(const BvRenderDeviceVk* pDevice, u64 initialValue = 0);
	~BvGPUFenceVk();

	bool IsDone(u64 value) override;
	bool Wait(u64 value, u64 timeout = kU64Max) override;
	u64 GetCompletedValue() override;

	BV_INLINE const BvSemaphoreVk* GetSemaphore() const { return &m_Semaphore; }

private:
	BvSemaphoreVk m_Semaphore;
};


BV_CREATE_CAST_TO_VK(BvGPUFence)