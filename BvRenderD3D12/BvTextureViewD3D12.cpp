#include "BvTextureViewD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"


BV_D3D12_DEVICE_RES_DEF(BvTextureViewD3D12)


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

	auto srv = GetD3D12SRVDesc(m_TextureViewDesc);
	auto uav = GetD3D12UAVDesc(m_TextureViewDesc);

	// TODO: Allocate CPU descriptors

	auto pDevice = m_pDevice->GetHandle();
	pDevice->CreateShaderResourceView(pRes, &srv, m_SRV);
	pDevice->CreateUnorderedAccessView(pRes, nullptr, &uav, m_UAV);
}


void BvTextureViewD3D12::Destroy()
{
}