#include "BvBufferD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"


BV_D3D12_DEVICE_RES_DEF(BvBufferD3D12);


BvBufferD3D12::BvBufferD3D12(BvRenderDeviceD3D12* pDevice, const BufferDesc& bufferDesc, const BufferInitData* pInitData)
	: m_pDevice(pDevice), m_BufferDesc(bufferDesc)
{
	Create(pInitData);
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


void BvBufferD3D12::Create(const BufferInitData* pInitData)
{
	HRESULT hr = S_OK;

	u64 alignedSize = m_BufferDesc.m_Size;
	if (EHasFlag(m_BufferDesc.m_UsageFlags, BufferUsage::kConstantBuffer))
	{
		alignedSize = RoundToNearestPowerOf2(alignedSize, u64(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
	}

	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedSize, GetD3D12ResourceFlags(m_BufferDesc.m_UsageFlags));

	D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

	D3D12MA::ALLOCATION_DESC allocationDesc{};
	allocationDesc.HeapType = GetD3D12HeapType(m_BufferDesc.m_MemoryType);
	if (allocationDesc.HeapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else if (allocationDesc.HeapType == D3D12_HEAP_TYPE_READBACK)
	{
		resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}

	auto pAllocator = m_pDevice->GetAllocator();
	auto pDevice = m_pDevice->GetHandle();
	D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = pDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
	hr = pAllocator->AllocateMemory(&allocationDesc, &allocationInfo, &m_Allocation);
	if (FAILED(hr))
	{
		// TODO: Handle error
	}

	hr = pDevice->CreatePlacedResource(m_Allocation->GetHeap(), m_Allocation->GetOffset(), &resourceDesc, resourceState, nullptr, IID_PPV_ARGS(&m_Buffer));
	if (FAILED(hr))
	{
		m_Allocation = nullptr;
		// TODO: Handle error
	}

	if (m_BufferDesc.m_MemoryType != MemoryType::kDevice)
	{
		if (EHasFlag(m_BufferDesc.m_CreateFlags, BufferCreateFlags::kCreateMapped))
		{
			Map(alignedSize, 0);
		}

		if (pInitData)
		{
			if (!m_pMapped)
			{
				Map(alignedSize, 0);
			}
			memcpy(m_pMapped, pInitData->m_pData, std::min(alignedSize, pInitData->m_Size));

			if (!EHasFlag(m_BufferDesc.m_CreateFlags, BufferCreateFlags::kCreateMapped))
			{
				Unmap();
			}
		}
	}
	else
	{
		CopyInitDataToGPU(pInitData);
	}
}


void BvBufferD3D12::Destroy()
{
	if (m_Buffer)
	{
		m_Buffer = nullptr;
		m_Allocation = nullptr;
	}
}


void BvBufferD3D12::CopyInitDataToGPU(const BufferInitData* pInitData)
{
	BV_ASSERT(false, "Not implemented");
}