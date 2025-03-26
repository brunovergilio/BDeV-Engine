#pragma once


#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


//BV_OBJECT_DEFINE_ID(IBvSamplerVk, "15824086-fe4a-46c3-9362-f8e43b8c3e67");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvSamplerVk);


class BvSamplerVk final : public IBvSampler, public IBvResourceVk
{
	BV_NOCOPYMOVE(BvSamplerVk);
	BV_VK_DEVICE_RES_DECL;

public:
	BvSamplerVk(BvRenderDeviceVk* pDevice, const SamplerDesc& samplerDesc);
	~BvSamplerVk();

	BV_INLINE const SamplerDesc& GetDesc() const override { return m_SamplerDesc; }
	BV_INLINE VkSampler GetHandle() const { return m_Sampler; }
	BV_INLINE bool IsValid() const { return m_Sampler != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvSamplerVk, IBvSampler, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkSampler m_Sampler = VK_NULL_HANDLE;
	SamplerDesc m_SamplerDesc;
};


BV_CREATE_CAST_TO_VK(BvSampler)