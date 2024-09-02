#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvVector.h"


class BvRenderDeviceVk;


class BvSemaphoreVk final
{
	BV_NOCOPY(BvSemaphoreVk);

public:
	BvSemaphoreVk();
	BvSemaphoreVk(VkDevice device, u64 initialValue = 0, bool isTimelineSemaphore = true);
	BvSemaphoreVk(BvSemaphoreVk&& rhs) noexcept;
	BvSemaphoreVk& operator=(BvSemaphoreVk&& rhs) noexcept;
	~BvSemaphoreVk();

	void Signal(u64 value);
	bool Wait(u64 value, u64 timeout = kU64Max);
	u64 GetCompletedValue();

	BV_INLINE VkSemaphore GetHandle() const { return m_Semaphore; }

private:
	void Create(u64 initialValue, bool isTimelineSemaphore);
	void Destroy();
	
private:
	VkDevice m_Device = VK_NULL_HANDLE;
	VkSemaphore m_Semaphore = VK_NULL_HANDLE;
};


class BvSemaphorePoolVk
{
	BV_NOCOPY(BvSemaphorePoolVk);

public:
	BvSemaphorePoolVk();
	BvSemaphorePoolVk(VkDevice device);
	BvSemaphorePoolVk(BvSemaphorePoolVk&& rhs) noexcept;
	BvSemaphorePoolVk& operator=(BvSemaphorePoolVk&& rhs) noexcept;
	~BvSemaphorePoolVk();

	BvSemaphoreVk* GetSemaphore();
	void Reset();

private:
	VkDevice m_Device = VK_NULL_HANDLE;
	BvVector<BvSemaphoreVk> m_Semaphores;
	u32 m_ActiveSemaphoreCount = 0;
};