#pragma once


#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvSamplerVk final : public BvSampler
{
	BV_NOCOPYMOVE(BvSamplerVk);

public:
	BvSamplerVk(BvRenderDeviceVk* pDevice, const SamplerDesc & samplerDesc);
	~BvSamplerVk();
	BvRenderDevice* GetDevice() override;

	BV_INLINE VkSampler GetHandle() const { return m_Sampler; }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkSampler m_Sampler = VK_NULL_HANDLE;
};


BV_CREATE_CAST_TO_VK(BvSampler)