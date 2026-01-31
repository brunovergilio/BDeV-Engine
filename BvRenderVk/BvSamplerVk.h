#pragma once


#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvSamplerVk final : public IBvSampler, public IBvResourceVk
{
public:
	BvSamplerVk(BvRenderDeviceVk* pDevice, const SamplerDesc& samplerDesc, VkSampler sampler);
	~BvSamplerVk();

	BV_INLINE const SamplerDesc& GetDesc() const override { return m_SamplerDesc; }
	BV_INLINE VkSampler GetHandle() const { return m_Sampler; }
	BV_INLINE bool IsValid() const { return m_Sampler != VK_NULL_HANDLE; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkSampler m_Sampler = VK_NULL_HANDLE;
	SamplerDesc m_SamplerDesc;
};
BV_OBJECT_DEFINE_ID(BvSamplerVk, "15824086-fe4a-46c3-9362-f8e43b8c3e67");
BV_CREATE_CAST_TO_VK(BvSampler)