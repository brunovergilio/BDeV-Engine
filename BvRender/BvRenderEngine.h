#pragma once


#include "BvRender/BvRenderDevice.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Container/BvVector.h"
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
	virtual ~BvRenderEngine() = 0 {};
	virtual void GetGPUInfo(const u32 index, BvGPUInfo & info) const = 0;
	virtual BvRenderDevice * const CreateRenderDevice(const DeviceCreateDesc& deviceDesc = DeviceCreateDesc(), const u32 gpuIndex = 0) = 0;
	virtual void DestroyRenderDevice(const u32 gpuIndex) = 0;
	virtual const char * const GetAPIName() const = 0;

	BV_INLINE u32 GetSupportedGPUCount() const { return m_Devices.Size(); }
	BV_INLINE BvRenderDevice * const GetRenderDevice(const u32 index = 0) const { return m_Devices[index]; }

protected:
	BvRenderEngine() {}

protected:
	BvVector<BvRenderDevice *> m_Devices;
};