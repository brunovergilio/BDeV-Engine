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


BV_OBJECT_DEFINE_ID(BvRenderEngine, "1f046059-8e51-4fca-aac6-fd11aa3e1684");
class BvRenderEngine : public BvObjectBase
{
	BV_NOCOPYMOVE(BvRenderEngine);

public:
	virtual bool CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc, IBvRenderDevice** ppObj) = 0;
	virtual const BvVector<BvGPUInfo>& GetGPUs() const = 0;

protected:
	BvRenderEngine() {}
	~BvRenderEngine() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderEngine);