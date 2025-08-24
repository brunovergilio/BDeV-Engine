#pragma once


#include "BDeV/Core/RenderAPI/BvShaderResource.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvShaderResourceLayoutD3D12 : public IBvShaderResourceLayout, public IBvResourceD3D12
{
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvShaderResourceLayoutD3D12(BvRenderDeviceD3D12* pDevice, const ShaderResourceLayoutDesc& srlDesc);
	~BvShaderResourceLayoutD3D12();

	BV_INLINE ID3D12RootSignature* GetRootSignature() const { return m_RootSig.Get(); }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ShaderResourceLayoutDesc m_ShaderResourceLayoutDesc;
	ShaderResourceDesc* m_pShaderResources = nullptr;
	ShaderResourceConstantDesc* m_pShaderConstants = nullptr;
	ShaderResourceSetDesc* m_pShaderResourceSets = nullptr;
	IBvSampler** m_ppStaticSamplers = nullptr;
	ComPtr<ID3D12RootSignature> m_RootSig;
};


BV_CREATE_CAST_TO_D3D12(BvShaderResourceLayout)