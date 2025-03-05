#pragma once


#include "BDeV/RenderAPI/BvSampler.h"
#include "BvCommonGl.h"


class BvRenderDeviceGl;


class BvSamplerGl final : public IBvSampler
{
	BV_NOCOPYMOVE(BvSamplerGl);

public:
	BvSamplerGl(const BvRenderDeviceGl& device, const SamplerDesc& samplerDesc);
	~BvSamplerGl();

	void Create();
	void Destroy();

	BV_INLINE GLuint GetHandle() const { return m_Sampler; }

private:
	const BvRenderDeviceGl& m_Device;
	GLuint m_Sampler = 0;
};