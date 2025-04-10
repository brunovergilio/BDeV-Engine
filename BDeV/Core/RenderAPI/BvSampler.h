#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvObject.h"


//BV_OBJECT_DEFINE_ID(IBvSampler, "2110750d-806b-4758-b844-a66f75ac383a");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvSampler);
class IBvSampler : public BvRCObj
{
	BV_NOCOPYMOVE(IBvSampler);

public:
	virtual const SamplerDesc& GetDesc() const = 0;

protected:
	IBvSampler() {}
	~IBvSampler() {}
};