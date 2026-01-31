#pragma once


#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;
class BvSwapChainD3D12;
class BvCommandContextD3D12;


class BvTextureD3D12 final : public IBvTexture, public IBvResourceD3D12
{
public:
	BvTextureD3D12(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, ComPtr<ID3D12Resource>& resource, ComPtr<D3D12MA::Allocation>& allocation);
	BvTextureD3D12(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, ID3D12Resource* pResource);
	~BvTextureD3D12();

	BV_INLINE const TextureDesc& GetDesc() const override { return m_TextureDesc; }
	BV_INLINE ID3D12Resource* GetHandle() const { return m_Resource.Get(); }
	BV_INLINE bool IsValid() const { return m_Resource; }

private:
	void Destroy();

protected:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12Resource> m_Resource;
	ComPtr<D3D12MA::Allocation> m_Allocation;
	TextureDesc m_TextureDesc;
};
BV_OBJECT_DEFINE_ID(BvTextureD3D12, "1a570a6f-508b-4dad-aeb8-538df7a06f75");
BV_CREATE_CAST_TO_D3D12(BvTexture)