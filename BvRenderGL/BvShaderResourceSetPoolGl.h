#pragma once


#include "BvCommonGl.h"
#include "BDeV/RenderAPI/BvShaderResource.h"


class BvRenderDeviceGl;
class BvShaderResourceParamsGl;


class BvShaderResourceSetPoolGl final : public BvShaderResourceSetPool
{
public:
	BvShaderResourceSetPoolGl(const BvRenderDeviceGl& device, const ShaderResourceSetPoolDesc& desc);
	~BvShaderResourceSetPoolGl();

private:
	const BvRenderDeviceGl& m_Device;
	BvRobinMap<const BvShaderResourceLayout*, BvVector<BvShaderResourceParamsGl*>> m_UsedSets;
	BvRobinMap<const BvShaderResourceLayout*, BvVector<BvShaderResourceParamsGl*>> m_FreeSets;
};