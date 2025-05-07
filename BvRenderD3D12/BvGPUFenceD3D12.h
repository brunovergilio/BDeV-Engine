#pragma once

#include "BDeV/Core/RenderAPI/BvGPUFence.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvGPUFenceD3D12 final : public IBvGPUFence, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvGPUFenceD3D12);

public:
	BvGPUFenceD3D12(BvRenderDeviceD3D12* pDevice, u64 initialValue = 0);
	~BvGPUFenceD3D12();

	void Signal(u64 value) override;
	bool Wait(u64 value, u64 timeout = kU64Max) override;
	bool IsDone(u64 value) override;
	u64 GetCompletedValue() override;

	BV_INLINE ID3D12Fence* GetHandle() const { return m_Fence.Get(); }
	BV_INLINE bool IsValid() const { return m_Fence; }

private:
	void Create(u64 initialValue);
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12Fence> m_Fence;
	HANDLE m_Event = nullptr;
};


BV_CREATE_CAST_TO_D3D12(BvGPUFence)