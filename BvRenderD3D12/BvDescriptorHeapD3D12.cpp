#include "BvDescriptorHeapD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvShaderResourceD3D12.h"


BvGPUDescriptorHeapD3D12::BvGPUDescriptorHeapD3D12(BvRenderDeviceD3D12* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptors)
	: m_NumDescriptors(numDescriptors), m_DescriptorSize(pDevice->GetHandle()->GetDescriptorHandleIncrementSize(heapType))
{
	D3D12_DESCRIPTOR_HEAP_DESC dhd{ heapType, numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 0 };
	auto hr = pDevice->GetHandle()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_Heap));
	if (FAILED(hr))
	{
		// TODO: Handle error
	}

	m_Handle = { m_Heap->GetCPUDescriptorHandleForHeapStart(), m_Heap->GetGPUDescriptorHandleForHeapStart() };
}


BvGPUDescriptorHeapD3D12::~BvGPUDescriptorHeapD3D12()
{
	m_Heap.Reset();
}


BvDescriptorHandle BvGPUDescriptorHeapD3D12::Allocate(u32 count /*= 1*/)
{
	auto currIndex = m_CurrUsed.fetch_add(count);
	if (currIndex + count > m_NumDescriptors)
	{
		BV_ASSERT(false, "Not enough descriptors");
		return {};
	}

	return m_Handle.GetByIndex(currIndex, m_DescriptorSize);
}


BvCPUDescriptorHeapD3D12::BvCPUDescriptorHeapD3D12(BvRenderDeviceD3D12* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptorsPerHeap)
	: m_pDevice(pDevice), m_HeapType(heapType), m_NumDescriptors(numDescriptorsPerHeap), m_CurrUsed(numDescriptorsPerHeap),
	m_DescriptorSize(pDevice->GetHandle()->GetDescriptorHandleIncrementSize(heapType))
{
}


BvCPUDescriptorHeapD3D12::~BvCPUDescriptorHeapD3D12()
{
	for (auto& heap : m_CPUHeaps)
	{
		heap->Release();
	}
}


D3D12_CPU_DESCRIPTOR_HANDLE BvCPUDescriptorHeapD3D12::Allocate(u32 count /*= 1*/)
{
	if (count + m_CurrUsed > m_NumDescriptors)
	{
		auto& heap = m_CPUHeaps.EmplaceBack();
		D3D12_DESCRIPTOR_HEAP_DESC dhd{ m_HeapType, m_NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
		auto hr = m_pDevice->GetHandle()->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&heap));
		if (FAILED(hr))
		{
			// TODO: Handle error
			return { kInvalidPos };
		}

		m_Handle = heap->GetCPUDescriptorHandleForHeapStart();
		m_CurrUsed = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle = { m_Handle.ptr + m_DescriptorSize * m_CurrUsed };
	m_CurrUsed += count;

	return handle;
}


BvDescriptorPoolD3D12::BvDescriptorPoolD3D12()
{
}


BvDescriptorPoolD3D12::BvDescriptorPoolD3D12(BvRenderDeviceD3D12* pDevice, const BvShaderResourceLayoutD3D12* pLayout, u32 rootIndex, u32 maxAllocationsPerPool)
	: m_RootIndex(rootIndex), m_MaxAllocationsPerPool(maxAllocationsPerPool)
{
	auto& rootParam = pLayout->GetRootParams()[rootIndex];
	BV_ASSERT(rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, "Root parameter is not a descriptor table");
	
	for (auto i = 0; i < rootParam.DescriptorTable.NumDescriptorRanges; i++)
	{
		const auto& range = rootParam.DescriptorTable.pDescriptorRanges[i];

		if (!m_pDescriptorHeap)
		{
			if (range.RangeType != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			{
				m_pDescriptorHeap = pDevice->GetGPUShaderHeap();
				m_HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			}
			else
			{
				m_pDescriptorHeap = pDevice->GetGPUSamplerHeap();
				m_HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			}
		}

		m_HandleCount += range.NumDescriptors;
	}
}


BvDescriptorPoolD3D12::~BvDescriptorPoolD3D12()
{
}


BvDescriptorHandle BvDescriptorPoolD3D12::Allocate()
{
	if (!m_FreeDescriptors.Empty())
	{
		auto handle = m_FreeDescriptors.Back();
		m_FreeDescriptors.PopBack();

		return handle;
	}

	if (!m_Pools.Size() || (m_Pools[m_CurrPoolIndex].m_CurrAllocations == m_MaxAllocationsPerPool && ++m_CurrPoolIndex == m_Pools.Size()))
	{
		m_Pools.EmplaceBack().m_Handle = m_pDescriptorHeap->Allocate(m_HandleCount * m_MaxAllocationsPerPool);
	}

	auto& pool = m_Pools[m_CurrPoolIndex];
	return pool.m_Handle.GetByIndex(m_HandleCount * pool.m_CurrAllocations++, m_pDescriptorHeap->GetDescriptorSize());
}


void BvDescriptorPoolD3D12::RecycleDescriptor(const BvDescriptorHandle& handle)
{
	m_FreeDescriptors.PushBack(handle);
}


void BvDescriptorPoolD3D12::Reset()
{
	m_Pools.Clear();
	m_CurrPoolIndex = 0;
}


void BvResourceBindingStateD3D12::SetResource(const BvDescriptorHandle& handle, u32 registerSpace, u32 binding, u32 arrayIndex)
{
	auto [pDescriptor, newElem] = AddOrRetrieveResourceData(registerSpace, binding, arrayIndex);
	if (newElem || pDescriptor->GetCPUHandle() != handle.GetCPUHandle())
	{
		m_DirtySets[registerSpace] = true;
		*pDescriptor = handle;
	}
}


void BvResourceBindingStateD3D12::Reset()
{
	m_Bindings.Clear();
	m_Descriptors.Clear();
}


const BvDescriptorHandle* BvResourceBindingStateD3D12::GetResource(const ResourceIdD3D12& resId) const
{
	auto it = m_Bindings.FindKey(resId);
	return it != m_Bindings.cend() ? &m_Descriptors[it->second] : nullptr;
}


std::pair<BvDescriptorHandle*, bool> BvResourceBindingStateD3D12::AddOrRetrieveResourceData(u32 registerSpace, u32 binding, u32 arrayIndex)
{
	ResourceIdD3D12 resId{ registerSpace, binding, arrayIndex };
	auto bindingIt = m_Bindings.Emplace(resId, 0);
	u32 index = 0;
	bool newElem = false;
	if (!bindingIt.second)
	{
		index = bindingIt.first->second;
	}
	else
	{
		m_Descriptors.EmplaceBack();
		index = static_cast<u32>(m_Descriptors.Size()) - 1;
		bindingIt.first->second = index;
		newElem = true;
	}

	return std::make_pair(&m_Descriptors[index], newElem);
}