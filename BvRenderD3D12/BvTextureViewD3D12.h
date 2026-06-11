#pragma once


#include "BDeV/Core/RenderAPI/BvTextureView.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvTextureViewD3D12 final : public IBvTextureView, public IBvResourceD3D12
{
public:
	BvTextureViewD3D12(BvRenderDeviceD3D12* pDevice, const TextureViewDesc& textureViewDesc);
	~BvTextureViewD3D12();

	BV_INLINE const TextureViewDesc& GetDesc() const override { return m_TextureViewDesc; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_SRV; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetUAV() const { return m_UAV; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetRTV() const { return m_RTV; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSV() const { return m_DSV; }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	TextureViewDesc m_TextureViewDesc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_SRV{};
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_UAV{};
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_RTV{};
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_DSV{};
};
BV_OBJECT_DEFINE_ID(BvTextureViewD3D12, "7ce274c0-e876-4b9a-8016-4b8ca8c84640");
BV_CREATE_CAST_TO_D3D12(BvTextureView)