#pragma once


#include "BvRenderDevice.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvObject.h"
#include "BvGPUInfo.h"


struct BvRenderDeviceCreateDesc
{
	u32 m_GPUIndex = kU32Max;
	u32 m_GraphicsQueueCount = 1;
	u32 m_ComputeQueueCount = 0;
	u32 m_TransferQueueCount = 0;
	bool m_UseDebug = true;
};


//BV_OBJECT_DEFINE_ID(IBvRenderEngine, "1f046059-8e51-4fca-aac6-fd11aa3e1684");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderEngine);
class IBvRenderEngine : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRenderEngine);

public:
	virtual const BvVector<BvGPUInfo>& GetGPUs() const = 0;
	template<typename T = IBvRenderDevice> BV_INLINE BvRCRaw<T> CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc) { return static_cast<T*>(CreateRenderDeviceImpl(deviceCreateDesc)); }

protected:
	IBvRenderEngine() {}
	~IBvRenderEngine() {}

	virtual IBvRenderDevice* CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc) = 0;
};