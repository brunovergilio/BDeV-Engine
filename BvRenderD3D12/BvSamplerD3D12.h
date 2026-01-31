#pragma once


#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvSamplerD3D12 final : public IBvSampler, public IBvResourceD3D12
{
public:
	BvSamplerD3D12(BvRenderDeviceD3D12* pDevice, const SamplerDesc& samplerDesc);
	~BvSamplerD3D12();

	BV_INLINE const SamplerDesc& GetDesc() const override { return m_SamplerDesc; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetHandle() const { return m_Sampler; }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Sampler;
	SamplerDesc m_SamplerDesc;
};
BV_OBJECT_DEFINE_ID(BvSamplerD3D12, "15824086-fe4a-46c3-9362-f8e43b8c3e67");
BV_CREATE_CAST_TO_D3D12(BvSampler)