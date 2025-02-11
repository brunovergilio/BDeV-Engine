#pragma once


#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvSamplerVk, "15824086-fe4a-46c3-9362-f8e43b8c3e67");
class BvSamplerVk final : public BvSampler
{
	BV_NOCOPYMOVE(BvSamplerVk);

public:
	BvSamplerVk(BvRenderDeviceVk* pDevice, const SamplerDesc & samplerDesc);
	~BvSamplerVk();
	BvRenderDevice* GetDevice() override;

	BV_INLINE VkSampler GetHandle() const { return m_Sampler; }
	BV_INLINE bool IsValid() const { return m_Sampler != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvSamplerVk, BvSampler, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkSampler m_Sampler = VK_NULL_HANDLE;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvSamplerVk);


BV_CREATE_CAST_TO_VK(BvSampler)