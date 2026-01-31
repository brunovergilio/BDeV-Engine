#include "BvGPUFenceD3D12.h"
#include "BvRenderDeviceD3D12.h"


BvGPUFenceD3D12::BvGPUFenceD3D12(BvRenderDeviceD3D12* pDevice, ComPtr<ID3D12Fence>& fence, HANDLE event)
	: m_pDevice(pDevice), m_Fence(std::move(fence)), m_Event(event)
{
}


BvGPUFenceD3D12::~BvGPUFenceD3D12()
{
	Destroy();
}


void BvGPUFenceD3D12::Signal(u64 value)
{
	m_Fence->Signal(value);
}


bool BvGPUFenceD3D12::Wait(u64 value, u64 timeout)
{
	if (m_Fence->GetCompletedValue() >= value)
	{
		return true;
	}

	u64 timeoutInMs = (timeout / 1000000);
	DWORD waitTime = DWORD(std::min(timeoutInMs, u64(INFINITE)));

	m_Fence->SetEventOnCompletion(value, m_Event);
	auto result = WaitForSingleObject(m_Event, waitTime);

	return result == WAIT_OBJECT_0;
}


bool BvGPUFenceD3D12::IsDone(u64 value)
{
	return GetCompletedValue() >= value;
}


u64 BvGPUFenceD3D12::GetCompletedValue()
{
	return m_Fence->GetCompletedValue();
}


void BvGPUFenceD3D12::Destroy()
{
	if (m_Fence)
	{
		m_Fence.Reset();
		CloseHandle(m_Event);
	}
}