#pragma once


#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvSamplerVk, "15824086-fe4a-46c3-9362-f8e43b8c3e67");
class IBvSamplerVk : public IBvSampler
{
	BV_NOCOPYMOVE(IBvSamplerVk);

public:
	virtual VkSampler GetHandle() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvSamplerVk() {}
	~IBvSamplerVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvSamplerVk);


class BvSamplerVk final : public IBvSamplerVk
{
	BV_NOCOPYMOVE(BvSamplerVk);

public:
	BvSamplerVk(BvRenderDeviceVk* pDevice, const SamplerDesc& samplerDesc);
	~BvSamplerVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const SamplerDesc& GetDesc() const override { return m_SamplerDesc; }
	BV_INLINE VkSampler GetHandle() const override { return m_Sampler; }
	BV_INLINE bool IsValid() const override { return m_Sampler != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvSamplerVk, IBvSampler, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkSampler m_Sampler = VK_NULL_HANDLE;
	SamplerDesc m_SamplerDesc;
};


BV_CREATE_CAST_TO_VK(IBvSampler)