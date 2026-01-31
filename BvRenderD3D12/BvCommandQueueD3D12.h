#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


class BvRenderDeviceD3D12;
class BvCommandListD3D12;
class BvGPUFenceD3D12;


class BvCommandQueueD3D12 final
{
	BV_NOCOPY(BvCommandQueueD3D12);

public:
	BvCommandQueueD3D12(BvRenderDeviceD3D12* pDevice, ID3D12CommandQueue* pQueue);
	~BvCommandQueueD3D12();

	void AddWaitFence(ID3D12Fence* pWaitFence, u64 value);
	void AddSignalFence(ID3D12Fence* pSignalFence, u64 value);
	void Submit(const BvVector<ID3D12GraphicsCommandList*>& commandLists, ID3D12Fence* signalFence, u64 value);
	void WaitIdle();

	BV_INLINE ID3D12CommandQueue* GetHandle() const { return m_Queue.Get(); }

private:
	ComPtr<ID3D12CommandQueue> m_Queue;
	BvVector<ID3D12CommandList*> m_Commandlists;
	BvVector<std::pair<ID3D12Fence*, u64>> m_WaitFences;
	BvVector<std::pair<ID3D12Fence*, u64>> m_SignalFences;
	BvRCRef<BvGPUFenceD3D12> m_IdleFence;
	u64 m_IdleFenceValue = 0;
};