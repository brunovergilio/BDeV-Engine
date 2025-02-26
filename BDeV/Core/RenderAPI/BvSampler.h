#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BvRenderDeviceObject.h"


BV_OBJECT_DEFINE_ID(BvSampler, "2110750d-806b-4758-b844-a66f75ac383a");
class BvSampler : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(BvSampler);

public:
	BV_INLINE const SamplerDesc & GetDesc() const { return m_SamplerDesc; }

protected:
	BvSampler(const SamplerDesc & samplerDesc)
		: m_SamplerDesc(samplerDesc) {}
	~BvSampler() {}

protected:
	SamplerDesc m_SamplerDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvSampler);