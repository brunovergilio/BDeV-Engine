#include "BvCommandQueueD3D12.h"
#include "BvCommandListD3D12.h"
#include "BvUtilsD3D12.h"
#include "BvGPUFenceD3D12.h"


BvCommandQueueD3D12::BvCommandQueueD3D12(BvRenderDeviceD3D12* pDevice, ID3D12CommandQueue* pQueue)
	: m_Queue(pQueue)
{
	auto fenceObj = D3D12Utils::CreateGPUFence(pDevice, { 0 });
	BV_ASSERT(SUCCEEDED(fenceObj.first), "Error creating gpu fence");
	auto pFence = BV_RC_CREATE(BvGPUFenceD3D12, pDevice, fenceObj.second.m_Fence, fenceObj.second.m_Event);
	m_IdleFence.Attach(pFence);
}


BvCommandQueueD3D12::~BvCommandQueueD3D12()
{
}


void BvCommandQueueD3D12::AddWaitFence(ID3D12Fence* pWaitFence, u64 value)
{
	m_WaitFences.PushBack({ pWaitFence, value });
}


void BvCommandQueueD3D12::AddSignalFence(ID3D12Fence* pSignalFence, u64 value)
{
	m_SignalFences.PushBack({ pSignalFence, value });
}


void BvCommandQueueD3D12::Submit(const BvVector<ID3D12GraphicsCommandList*>& commandLists, ID3D12Fence* signalFence, u64 value)
{
	for (auto cmdList : commandLists)
	{
		m_Commandlists.EmplaceBack(cmdList);
	}

	for (auto& wait : m_WaitFences)
	{
		m_Queue->Wait(wait.first, wait.second);
	}

	m_Queue->ExecuteCommandLists(m_Commandlists.Size(), m_Commandlists.Data());

	for (auto& signal : m_SignalFences)
	{
		m_Queue->Signal(signal.first, signal.second);
	}
	m_Queue->Signal(signalFence, value);

	m_WaitFences.Clear();
	m_Commandlists.Clear();
	m_SignalFences.Clear();
}


void BvCommandQueueD3D12::WaitIdle()
{
	m_Queue->Signal(m_IdleFence->GetHandle(), ++m_IdleFenceValue);
	m_IdleFence->Wait(m_IdleFenceValue);
}