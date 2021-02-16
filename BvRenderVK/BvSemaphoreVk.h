#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvRender/BvSemaphore.h"


class BvRenderDeviceVk;


class BvSemaphoreVk final : public BvSemaphore
{
	BV_NOCOPYMOVE(BvSemaphoreVk);

public:
	BvSemaphoreVk(const BvRenderDeviceVk & device);
	~BvSemaphoreVk();

	void Create(const bool isTimelineSemaphore = true, const u64 initialValue = 0);
	void Destroy();

	void Signal(const u64 value) override final;
	WaitStatus Wait(const u64 value, const u64 timeout) override final;
	u64 GetCompletedValue() const override final;

	BV_INLINE VkSemaphore GetHandle() const { return m_Semaphore; }

protected:
	const BvRenderDeviceVk & m_Device;
	VkSemaphore m_Semaphore = VK_NULL_HANDLE;
};