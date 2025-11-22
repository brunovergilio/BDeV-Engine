#include "BvDescriptorHeapD3D12.h"
#include "BvRenderDeviceD3D12.h"


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
	: m_pDevice(pDevice), m_HeapType(heapType), m_NumDescriptors(numDescriptorsPerHeap),
	m_DescriptorSize(pDevice->GetHandle()->GetDescriptorHandleIncrementSize(heapType)), m_CurrUsed(numDescriptorsPerHeap)
{
}


BvCPUDescriptorHeapD3D12::~BvCPUDescriptorHeapD3D12()
{
	for (auto& heap : m_CPUHeaps)
	{
		heap.Reset();
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