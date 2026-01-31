#pragma once


#include "BDeV/Core/RenderAPI/BvGPUFence.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvGPUFenceVk final : public IBvGPUFence, public IBvResourceVk
{
public:
	BvGPUFenceVk(BvRenderDeviceVk* pDevice, VkSemaphore semaphore);
	~BvGPUFenceVk();

	void Signal(u64 value) override;
	bool Wait(u64 value, u64 timeout = kU64Max) override;
	bool IsDone(u64 value) override;
	u64 GetCompletedValue() override;

	BV_INLINE VkSemaphore GetHandle() const { return m_Semaphore; }
	BV_INLINE bool IsValid() const { return m_Semaphore != VK_NULL_HANDLE; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkSemaphore m_Semaphore = VK_NULL_HANDLE;
};
BV_OBJECT_DEFINE_ID(BvGPUFenceVk, "96de8b96-bb06-4ae4-b96e-e6a3c5323232");
BV_CREATE_CAST_TO_VK(BvGPUFence)