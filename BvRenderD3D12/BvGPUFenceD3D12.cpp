#include "BvGPUFenceD3D12.h"
#include "BvRenderDeviceD3D12.h"


BvGPUFenceD3D12::BvGPUFenceD3D12(BvRenderDeviceD3D12* pDevice, u64 initialValue)
	: m_pDevice(pDevice)
{
	Create(initialValue);
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
	u64 timeoutInMs = (timeout / 1000000);
	DWORD waitValue = timeoutInMs > u64(INFINITE) ? INFINITE : DWORD(timeoutInMs);

	m_Fence->SetEventOnCompletion(value, m_Event);
	auto result = WaitForSingleObject(m_Event, waitValue);

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


void BvGPUFenceD3D12::Create(u64 initialValue)
{
	auto hr = m_pDevice->GetHandle()->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
	if (FAILED(hr))
	{
		// TODO: Handle error
	}

	m_Event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}


void BvGPUFenceD3D12::Destroy()
{
	if (m_Fence)
	{
		m_Fence = nullptr;
		CloseHandle(m_Event);
	}
}