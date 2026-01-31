#include "BvBufferD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"


BvBufferD3D12::BvBufferD3D12(BvRenderDeviceD3D12* pDevice, const BufferDesc& bufferDesc, ComPtr<ID3D12Resource>& resource,
	ComPtr<D3D12MA::Allocation>& allocation, void* pMappedMemory)
	: m_pDevice(pDevice), m_BufferDesc(bufferDesc), m_Buffer(std::move(resource)), m_Allocation(std::move(allocation)), m_pMapped(pMappedMemory)
{
}


BvBufferD3D12::~BvBufferD3D12()
{
	Destroy();
}


void* const BvBufferD3D12::Map(u64 size, u64 offset)
{
	BV_ASSERT(m_BufferDesc.m_MemoryType != MemoryType::kDevice, "Can't map device memory");

	if (m_pMapped)
	{
		return m_pMapped;
	}

	m_Range = D3D12_RANGE{ offset, offset + size };
	auto hr = m_Buffer->Map(0, &m_Range, &m_pMapped);
	if (FAILED(hr))
	{
		// TODO: Handle error
	}

	return m_pMapped;
}


void BvBufferD3D12::Unmap()
{
	if (!m_pMapped)
	{
		return;
	}

	m_Buffer->Unmap(0, &m_Range);
	m_pMapped = nullptr;
}


void BvBufferD3D12::Destroy()
{
	if (m_Buffer)
	{
		m_Buffer = nullptr;
		m_Allocation = nullptr;
	}
}