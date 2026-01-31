#include "BvTextureD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvTextureD3D12::BvTextureD3D12(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, ComPtr<ID3D12Resource>& resource,
	ComPtr<D3D12MA::Allocation>& allocation)
	: m_pDevice(pDevice), m_TextureDesc(textureDesc), m_Resource(std::move(resource)), m_Allocation(std::move(allocation))
{
}


BvTextureD3D12::BvTextureD3D12(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, ID3D12Resource* pResource)
	: m_pDevice(pDevice), m_TextureDesc(textureDesc)
{
	m_Resource.Attach(pResource);
}


BvTextureD3D12::~BvTextureD3D12()
{
	Destroy();
}


void BvTextureD3D12::Destroy()
{
	if (m_Resource)
	{
		m_Resource = nullptr;
		m_Allocation = nullptr;
	}
}