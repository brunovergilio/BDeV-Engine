#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvVector.h"


class IBvRenderDeviceVk;


class BvSemaphoreVk final
{
	BV_NOCOPY(BvSemaphoreVk);

public:
	BvSemaphoreVk();
	BvSemaphoreVk(IBvRenderDeviceVk* pDevice, u64 initialValue = 0, bool isTimelineSemaphore = true);
	BvSemaphoreVk(BvSemaphoreVk&& rhs) noexcept;
	BvSemaphoreVk& operator=(BvSemaphoreVk&& rhs) noexcept;
	~BvSemaphoreVk();

	void Signal(u64 value);
	bool Wait(u64 value, u64 timeout = kU64Max);
	u64 GetCompletedValue();

	BV_INLINE VkSemaphore GetHandle() const { return m_Semaphore; }
	BV_INLINE IBvRenderDeviceVk* GetDevice() const { return m_pDevice; }

private:
	void Create(u64 initialValue, bool isTimelineSemaphore);
	void Destroy();
	
private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	VkSemaphore m_Semaphore = VK_NULL_HANDLE;
};