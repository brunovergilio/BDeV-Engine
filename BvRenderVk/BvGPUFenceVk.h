#pragma once


#include "BDeV/Core/RenderAPI/BvGPUFence.h"
#include "BvSemaphoreVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvGPUFenceVk, "96de8b96-bb06-4ae4-b96e-e6a3c5323232");
class IBvGPUFenceVk : public IBvGPUFence
{
	BV_NOCOPYMOVE(IBvGPUFenceVk);

public:
	virtual const BvSemaphoreVk* GetSemaphore() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvGPUFenceVk() {}
	~IBvGPUFenceVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvGPUFenceVk);


class BvGPUFenceVk final : public IBvGPUFenceVk
{
	BV_NOCOPYMOVE(BvGPUFenceVk);

public:
	BvGPUFenceVk(BvRenderDeviceVk* pDevice, u64 initialValue = 0);
	~BvGPUFenceVk();

	bool IsDone(u64 value) override;
	bool Wait(u64 value, u64 timeout = kU64Max) override;
	u64 GetCompletedValue() override;
	IBvRenderDevice* GetDevice() override;

	BV_INLINE const BvSemaphoreVk* GetSemaphore() const override { return &m_Semaphore; }
	BV_INLINE bool IsValid() const override { return m_Semaphore.GetHandle() != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvGPUFenceVk, IBvGPUFence, IBvRenderDeviceObject);

private:
	BvSemaphoreVk m_Semaphore;
};


BV_CREATE_CAST_TO_VK(IBvGPUFence)