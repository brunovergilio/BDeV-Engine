#pragma once


#include "BDeV/Core/Utils/BvObject.h"


class IBvRenderDevice;


BV_OBJECT_DEFINE_ID(IBvRenderDeviceObject, "81fa81ab-fb53-4fe9-af11-5bc853799c79");
class IBvRenderDeviceObject : public BvObjectBase
{
public:
	virtual IBvRenderDevice* GetDevice() = 0;
	~IBvRenderDeviceObject() {}

protected:
	IBvRenderDeviceObject() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderDeviceObject);