#pragma once


#include "BDeV/RenderAPI/BvSampler.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvSamplerVk final : public BvSampler
{
	BV_NOCOPYMOVE(BvSamplerVk);

public:
	BvSamplerVk(const BvRenderDeviceVk & device, const SamplerDesc & samplerDesc);
	~BvSamplerVk();

	void Create();
	void Destroy();

	BV_INLINE VkSampler GetHandle() const { return m_Sampler; }

private:
	const BvRenderDeviceVk & m_Device;
	VkSampler m_Sampler = VK_NULL_HANDLE;
};