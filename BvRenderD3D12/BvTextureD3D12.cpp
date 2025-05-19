#include "BvTextureD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BV_D3D12_DEVICE_RES_DEF(BvTextureD3D12);


BvTextureD3D12::BvTextureD3D12(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, const TextureInitData* pInitData)
	: m_pDevice(pDevice), m_TextureDesc(textureDesc)
{
	Create(pInitData);
}


BvTextureD3D12::BvTextureD3D12(BvRenderDeviceD3D12* pDevice, BvSwapChainD3D12* pSwapChain, const TextureDesc& textureDesc, ID3D12Resource* pResource)
	: m_pDevice(pDevice), m_pSwapChain(pSwapChain), m_TextureDesc(textureDesc), m_Resource(pResource)
{
}


BvTextureD3D12::~BvTextureD3D12()
{
	Destroy();
}


void BvTextureD3D12::Create(const TextureInitData* pInitData)
{
	if (pInitData && m_TextureDesc.m_MemoryType != MemoryType::kDevice)
	{
		// TODO: Use a Buffer instead
		BV_ASSERT(false, "Not supported");
	}

	u32 mipCount = m_TextureDesc.m_MipLevels;

	CD3DX12_RESOURCE_DESC resourceDesc = GetD3D12ResourceDesc(m_TextureDesc);
	if (m_TextureDesc.m_MipLevels == 1 && EHasFlag(m_TextureDesc.m_CreateFlags, TextureCreateFlags::kReserveMips))
	{
		resourceDesc.MipLevels = GetMipCount(m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height, m_TextureDesc.m_Size.depth);
	}

	auto pAllocator = m_pDevice->GetAllocator();
	auto pDevice = m_pDevice->GetHandle();
	D3D12MA::ALLOCATION_DESC allocationDesc{};
	allocationDesc.HeapType = GetD3D12HeapType(m_TextureDesc.m_MemoryType);
	D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = pDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
	auto hr = pAllocator->AllocateMemory(&allocationDesc, &allocationInfo, &m_Allocation);
	if (FAILED(hr))
	{
		// TODO: Handle error
	}

	hr = pDevice->CreatePlacedResource(m_Allocation->GetHeap(), m_Allocation->GetOffset(), &resourceDesc,
		GetD3D12ResourceState(m_TextureDesc.m_ResourceState), nullptr, IID_PPV_ARGS(&m_Resource));
	if (FAILED(hr))
	{
		m_Allocation = nullptr;
		// TODO: Handle error
	}

	if (m_TextureDesc.m_ResourceState != ResourceState::kCommon && pInitData)
	{
		BV_ASSERT(pInitData->m_pContext != nullptr, "Invalid command context");
		CopyInitDataToGPUAndTransitionState(pInitData, mipCount);
	}
}


void BvTextureD3D12::Destroy()
{
	if (m_Resource)
	{
		m_Resource = nullptr;
		m_Allocation = nullptr;
	}
}


void BvTextureD3D12::CopyInitDataToGPUAndTransitionState(const TextureInitData* pInitData, u32 mipCount)
{
	BV_ASSERT(false, "Not implemented");
}


void BvTextureD3D12::GenerateMips(BvCommandContextD3D12* pContext)
{
	BV_ASSERT(false, "Not implemented");
}