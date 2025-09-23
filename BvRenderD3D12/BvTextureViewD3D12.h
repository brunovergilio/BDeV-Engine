#pragma once


#include "BDeV/Core/RenderAPI/BvTextureView.h"
#include "BvTextureD3D12.h"


class BvRenderDeviceD3D12;


//BV_OBJECT_DEFINE_ID(IBvTextureViewD3D12, "7ce274c0-e876-4b9a-8016-4b8ca8c84640");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvTextureViewD3D12);


class BvTextureViewD3D12 final : public IBvTextureView, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvTextureViewD3D12);
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvTextureViewD3D12(BvRenderDeviceD3D12* pDevice, const TextureViewDesc& textureViewDesc);
	~BvTextureViewD3D12();

	BV_INLINE const TextureViewDesc& GetDesc() const override { return m_TextureViewDesc; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_SRV; }
	BV_INLINE const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetUAV() const { return m_UAV; }

	//BV_OBJECT_IMPL_INTERFACE(IBvTextureViewD3D12, IBvTextureView, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	TextureViewDesc m_TextureViewDesc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_SRV;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_UAV;
};


BV_CREATE_CAST_TO_D3D12(BvTextureView)