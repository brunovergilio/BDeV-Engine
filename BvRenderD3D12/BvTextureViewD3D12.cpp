#include "BvTextureViewD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvDescriptorHeapD3D12.h"
#include "BvTextureD3D12.h"


BvTextureViewD3D12::BvTextureViewD3D12(BvRenderDeviceD3D12* pDevice, const TextureViewDesc& textureViewDesc)
	: m_TextureViewDesc(textureViewDesc), m_pDevice(pDevice)
{
	Create();
}


BvTextureViewD3D12::~BvTextureViewD3D12()
{
	Destroy();
}


void BvTextureViewD3D12::Create()
{
	BV_ASSERT(m_TextureViewDesc.m_pTexture != nullptr, "Invalid texture handle");

	auto pRes = TO_D3D12(m_TextureViewDesc.m_pTexture)->GetHandle();

	auto pDevice = m_pDevice->GetHandle();

	auto desc = pRes->GetDesc();
	auto pShaderHeap = m_pDevice->GetCPUShaderHeap();
	if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
	{
		auto srv = GetD3D12SRVDesc(m_TextureViewDesc);
		m_SRV = pShaderHeap->Allocate();
		pDevice->CreateShaderResourceView(pRes, &srv, m_SRV);
	}

	if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
	{
		auto uav = GetD3D12UAVDesc(m_TextureViewDesc);
		m_UAV = pShaderHeap->Allocate();
		pDevice->CreateUnorderedAccessView(pRes, nullptr, &uav, m_UAV);
	}

	if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
	{
		auto pRTVHeap = m_pDevice->GetCPURTVHeap();
		m_RTV = pRTVHeap->Allocate();
		auto rtv = GetD3D12RTVDesc(m_TextureViewDesc);
		pDevice->CreateRenderTargetView(pRes, &rtv, m_RTV);
	}

	if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		auto pDSVHeap = m_pDevice->GetCPUDSVHeap();
		auto dsv = GetD3D12DSVDesc(m_TextureViewDesc);
		m_DSV = pDSVHeap->Allocate();
		pDevice->CreateDepthStencilView(pRes, &dsv, m_DSV);
	}
}


void BvTextureViewD3D12::Destroy()
{
}