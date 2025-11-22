#pragma once


#include "BDeV/Core/RenderAPI/BvBufferView.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvBufferViewD3D12 final : public IBvBufferView, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvBufferViewD3D12);
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvBufferViewD3D12(BvRenderDeviceD3D12* pDevice, const BufferViewDesc& bufferViewDesc);
	~BvBufferViewD3D12();

	BV_INLINE const BufferViewDesc& GetDesc() const { return m_BufferViewDesc; }
	BV_INLINE bool IsValid() const { return m_BufferViewDesc.m_pBuffer != nullptr; }

	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetCBV() const { return m_CBV; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_SRV; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetUAV() const { return m_UAV; }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BufferViewDesc m_BufferViewDesc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CBV;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_SRV;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_UAV;
};


BV_CREATE_CAST_TO_D3D12(BvBufferView)