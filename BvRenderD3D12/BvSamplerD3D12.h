#pragma once


#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


//BV_OBJECT_DEFINE_ID(IBvSamplerD3D12, "15824086-fe4a-46c3-9362-f8e43b8c3e67");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvSamplerD3D12);


class BvSamplerD3D12 final : public IBvSampler, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvSamplerD3D12);
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvSamplerD3D12(BvRenderDeviceD3D12* pDevice, const SamplerDesc& samplerDesc);
	~BvSamplerD3D12();

	BV_INLINE const SamplerDesc& GetDesc() const override { return m_SamplerDesc; }

	//BV_OBJECT_IMPL_INTERFACE(IBvSamplerD3D12, IBvSampler, IBvRenderDeviceObject);

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Sampler;
	SamplerDesc m_SamplerDesc;
};


BV_CREATE_CAST_TO_D3D12(BvSampler)