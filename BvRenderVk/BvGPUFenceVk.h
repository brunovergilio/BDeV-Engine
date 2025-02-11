#pragma once


#include "BDeV/Core/RenderAPI/BvGPUFence.h"
#include "BvSemaphoreVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvGPUFenceVk, "96de8b96-bb06-4ae4-b96e-e6a3c5323232");
class BvGPUFenceVk final : public BvGPUFence
{
	BV_NOCOPYMOVE(BvGPUFenceVk);

public:
	BvGPUFenceVk(BvRenderDeviceVk* pDevice, u64 initialValue = 0);
	~BvGPUFenceVk();

	bool IsDone(u64 value) override;
	bool Wait(u64 value, u64 timeout = kU64Max) override;
	u64 GetCompletedValue() override;
	BvRenderDevice* GetDevice() override;

	BV_INLINE const BvSemaphoreVk* GetSemaphore() const { return &m_Semaphore; }
	BV_INLINE bool IsValid() const { return m_Semaphore.GetHandle() != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvGPUFenceVk, BvGPUFence, IBvRenderDeviceObject);

private:
	BvSemaphoreVk m_Semaphore;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvGPUFenceVk);


BV_CREATE_CAST_TO_VK(BvGPUFence)