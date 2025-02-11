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
	virtual bool CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc, BvRenderDevice** ppObj) = 0;
	//virtual void SetVirtualMemory(void* pStart, void* pEnd, size_t growSize = 0) = 0;
	//virtual void SetVirtualMemory(size_t maxSize, size_t growSize = 0) = 0;

	BV_INLINE const auto& GetGPUs() const { return m_GPUs; }

protected:
	BvRenderEngine() {}
	~BvRenderEngine() {}

protected:
	BvVector<BvGPUInfo> m_GPUs;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderEngine);