#pragma once


#include "BvRenderDevice.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Container/BvVector.h"
#include "BvGPUInfo.h"


struct DeviceCreateDesc
{
	u32 m_GraphicsQueueCount = 1;
	u32 m_ComputeQueueCount = 1;
	u32 m_TransferQueueCount = 1;
};


class BvRenderEngine
{
	BV_NOCOPYMOVE(BvRenderEngine);

public:
	static constexpr u32 kAutoSelectGPU = kU32Max;

	virtual void GetGPUInfo(const u32 index, BvGPUInfo & info) const = 0;
	virtual BvRenderDevice * const CreateRenderDevice(const DeviceCreateDesc& deviceDesc = DeviceCreateDesc(), u32 gpuIndex = kAutoSelectGPU) = 0;
	virtual const char * const GetEngineName() const = 0;

	BV_INLINE u32 GetSupportedGPUCount() const { return m_Devices.Size(); }
	BV_INLINE BvRenderDevice * const GetRenderDevice(const u32 index = 0) const { return m_Devices[index]; }

protected:
	BvRenderEngine() {}
	virtual ~BvRenderEngine() = 0 {};

protected:
	BvVector<BvRenderDevice *> m_Devices;
};