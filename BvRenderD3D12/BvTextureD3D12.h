#pragma once


#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;
class BvSwapChainD3D12;
class BvCommandContextD3D12;


BV_OBJECT_DEFINE_ID(BvTextureD3D12, "1a570a6f-508b-4dad-aeb8-538df7a06f75");
class BvTextureD3D12 final : public IBvTexture, public IBvResourceD3D12
{
	BV_NOCOPYMOVE(BvTextureD3D12);
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvTextureD3D12(BvRenderDeviceD3D12* pDevice, const TextureDesc& textureDesc, const TextureInitData* pInitData);
	BvTextureD3D12(BvRenderDeviceD3D12* pDevice, BvSwapChainD3D12* pSwapChain, const TextureDesc& textureDesc, ID3D12Resource* pResource);
	~BvTextureD3D12();

	BV_INLINE const TextureDesc& GetDesc() const override { return m_TextureDesc; }
	BV_INLINE ID3D12Resource* GetHandle() const { return m_Resource.Get(); }
	BV_INLINE BvSwapChainD3D12* GetSwapChain() const { return m_pSwapChain; }
	BV_INLINE bool IsValid() const { return m_Resource; }

	//BV_OBJECT_IMPL_INTERFACE(IBvTextureD3D12, IBvTexture, IBvRenderDeviceObject);

private:
	void Create(const TextureInitData* pInitData);
	void Destroy();

	void CopyInitDataToGPUAndTransitionState(const TextureInitData* pInitData, u32 mipCount);
	void GenerateMips(BvCommandContextD3D12* pContext);

protected:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12Resource> m_Resource;
	ComPtr<D3D12MA::Allocation> m_Allocation;
	BvSwapChainD3D12* m_pSwapChain = nullptr;
	TextureDesc m_TextureDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvTextureD3D12);


BV_CREATE_CAST_TO_D3D12(BvTexture)