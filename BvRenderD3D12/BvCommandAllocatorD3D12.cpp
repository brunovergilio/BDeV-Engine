#include "BvCommandAllocatorD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvCommandListD3D12.h"


BvCommandAllocatorD3D12::BvCommandAllocatorD3D12(BvRenderDeviceD3D12* pDevice, CommandType commandType)
	: m_pDevice(pDevice), m_CommandType(commandType)
{
	Create();
}


BvCommandAllocatorD3D12::~BvCommandAllocatorD3D12()
{
	Destroy();
}


void BvCommandAllocatorD3D12::Create()
{
	auto result = m_pDevice->GetHandle()->CreateCommandAllocator(GetD3D12CommandListType(m_CommandType), IID_PPV_ARGS(&m_CommandAllocator));
	// TODO: Check error
}


void BvCommandAllocatorD3D12::Destroy()
{
	if (m_CommandAllocator)
	{
		for (auto pCL : m_CommandLists)
		{
			BV_DELETE(pCL);
		}

		m_CommandAllocator.Reset();
	}
}


BvCommandListD3D12* BvCommandAllocatorD3D12::GetCommandList(BvFrameDataD3D12* pFrameData)
{
	BvCommandListD3D12* pCommandList = nullptr;
	if (m_ActiveCommandBufferCount < m_CommandLists.Size())
	{
		pCommandList = m_CommandLists[m_ActiveCommandBufferCount++];
		pCommandList->Reset();
	}
	else
	{
		ComPtr<ID3D12GraphicsCommandList> commandList;
		auto hr = m_pDevice->GetHandle()->CreateCommandList(0, GetD3D12CommandListType(m_CommandType), m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
		if (FAILED(hr))
		{
			// TODO: Handle error
			return nullptr;
		}

		pCommandList = m_CommandLists.EmplaceBack(BV_NEW(BvCommandListD3D12)(m_pDevice, m_CommandAllocator.Get(), commandList.Get(), pFrameData));
	}

	++m_ActiveCommandBufferCount;
	pCommandList->Begin();

	return pCommandList;
}


void BvCommandAllocatorD3D12::Reset()
{
	m_CommandAllocator->Reset();
	m_ActiveCommandBufferCount = 0;
}