#pragma once


#include "BvRenderCommon.h"


class IBvRenderEngine : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRenderEngine);

public:
	virtual const GPUList& GetGPUs() const = 0;
	template<BvRCType T> BV_INLINE bool CreateRenderDevice(const RenderDeviceDesc& renderDeviceDesc, T** ppObj) { return CreateRenderDeviceImpl(renderDeviceDesc, reinterpret_cast<void**>(ppObj)); }

protected:
	IBvRenderEngine() {}
	~IBvRenderEngine() {}

	virtual bool CreateRenderDeviceImpl(const RenderDeviceDesc& renderDeviceDesc, void** ppObj) = 0;
};
BV_OBJECT_DEFINE_ID(IBvRenderEngine, "1f046059-8e51-4fca-aac6-fd11aa3e1684");