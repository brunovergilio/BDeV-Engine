#pragma once


#include "BvRenderGl/BvCommonGl.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/RenderAPI/BvSemaphore.h"


class BvRenderDeviceGl;


class BvSemaphoreGl final : public BvSemaphore
{
	BV_NOCOPYMOVE(BvSemaphoreGl);

public:
	BvSemaphoreGl(const BvRenderDeviceGl& device, u64 initialValue = 0);
	~BvSemaphoreGl();

	void Create(bool isTimelineSemaphore, u64 initialValue);
	void Destroy();

	void Signal(const u64 value) override final;
	WaitStatus Wait(const u64 value, const u64 timeout) override final;
	u64 GetCompletedValue() override final;

protected:
	const BvRenderDeviceGl& m_Device;
	std::atomic<u64> m_CurrValue = 0;
};