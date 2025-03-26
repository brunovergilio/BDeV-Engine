#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvObject.h"


//BV_OBJECT_DEFINE_ID(IBvTexture, "b3aad9aa-5a42-434c-87c8-cf744c8eeefa");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvTexture);
class IBvTexture : public BvRCObj
{
	BV_NOCOPYMOVE(IBvTexture);

public:
	virtual const TextureDesc& GetDesc() const = 0;

protected:
	IBvTexture() {}
	~IBvTexture() {}
};