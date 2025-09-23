#pragma once


#include "BvRenderCommon.h"


BV_OBJECT_DEFINE_ID(IBvRenderEngine, "1f046059-8e51-4fca-aac6-fd11aa3e1684");
class IBvRenderEngine : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRenderEngine);

public:
	virtual const GPUList& GetGPUs() const = 0;
	template<BvRCType T> BV_INLINE bool CreateRenderDevice(const BvRenderDeviceCreateDesc& deviceCreateDesc, T** ppObj) { return CreateRenderDeviceImpl(deviceCreateDesc, BV_OBJ_ARGS(ppObj)); }

	template<BvRCType T>
	BV_INLINE static bool Create(BvRCCreateFn pCreateFn, T** ppObj)
	{
		return pCreateFn(BV_OBJECT_ID(T), reinterpret_cast<void**>(ppObj));
	}

protected:
	IBvRenderEngine() {}
	~IBvRenderEngine() {}

	virtual bool CreateRenderDeviceImpl(const BvRenderDeviceCreateDesc& deviceCreateDesc, const BvUUID& objId, void** ppObj) = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderEngine);