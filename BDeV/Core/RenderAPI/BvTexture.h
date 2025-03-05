#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BvRenderDeviceObject.h"


BV_OBJECT_DEFINE_ID(IBvTexture, "b3aad9aa-5a42-434c-87c8-cf744c8eeefa");
class IBvTexture : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvTexture);

public:
	virtual const TextureDesc& GetDesc() const = 0;

protected:
	IBvTexture() {}
	~IBvTexture() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvTexture);