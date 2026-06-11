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
	auto pShaderHeap = m_pDevice->GetCPUShaderHeap();
	auto pDevice = m_pDevice->GetHandle();
	auto flags = m_TextureViewDesc.m_pTexture->GetDesc().m_UsageFlags;

	if (EHasAnyFlags(flags, TextureUsage::kShaderResource | TextureUsage::kInputAttachment))
	{
		auto srv = GetD3D12SRVDesc(m_TextureViewDesc);
		m_SRV = pShaderHeap->Allocate();
		pDevice->CreateShaderResourceView(pRes, &srv, m_SRV);
	}

	if (EHasFlag(flags, TextureUsage::kUnorderedAccess))
	{
		auto uav = GetD3D12UAVDesc(m_TextureViewDesc);
		m_UAV = pShaderHeap->Allocate();
		pDevice->CreateUnorderedAccessView(pRes, nullptr, &uav, m_UAV);
	}

	if (EHasFlag(flags, TextureUsage::kRenderTarget))
	{
		auto pRTVHeap = m_pDevice->GetCPURTVHeap();
		m_RTV = pRTVHeap->Allocate();
		auto rtv = GetD3D12RTVDesc(m_TextureViewDesc);
		pDevice->CreateRenderTargetView(pRes, &rtv, m_RTV);
	}

	if (EHasFlag(flags, TextureUsage::kDepthStencilTarget))
	{
		auto pDSVHeap = m_pDevice->GetCPUDSVHeap();
		auto dsv = GetD3D12DSVDesc(m_TextureViewDesc);
		m_DSV = pDSVHeap->Allocate();
		pDevice->CreateDepthStencilView(pRes, &dsv, m_DSV);
	}
}


void BvTextureViewD3D12::Destroy()
{
	u32 numHandles = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE handles[2];
	if (m_SRV.ptr)
	{
		handles[numHandles++] = m_SRV;
	}
	if (m_UAV.ptr)
	{
		handles[numHandles++] = m_UAV;
	}

	if (numHandles)
	{
		m_pDevice->OnD3D12HandleDestroyed(numHandles, handles);
	}
}