#pragma once


#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BvCommonVk.h"


class IBvRenderDeviceVk;
class IBvSwapChainVk;
class IBvCommandContextVk;


BV_OBJECT_DEFINE_ID(IBvTextureVk, "be4459ed-ed7b-4674-a638-6eff292841d4");
class IBvTextureVk : public IBvTexture
{
	BV_NOCOPYMOVE(IBvTextureVk);

public:
	virtual VkImage GetHandle() const = 0;
	virtual IBvSwapChainVk* GetSwapChain() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvTextureVk() {}
	~IBvTextureVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvTextureVk);


class BvTextureVk final : public IBvTextureVk
{
	BV_NOCOPYMOVE(BvTextureVk);

public:
	BvTextureVk(IBvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, const TextureInitData* pInitData);
	BvTextureVk(IBvRenderDeviceVk* pDevice, IBvSwapChainVk* pSwapChain, const TextureDesc& textureDesc, VkImage image);
	~BvTextureVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const TextureDesc& GetDesc() const override { return m_TextureDesc; }
	BV_INLINE VkImage GetHandle() const override { return m_Image; }
	BV_INLINE IBvSwapChainVk* GetSwapChain() const override { return m_pSwapChain; }
	BV_INLINE bool IsValid() const override { return m_Image != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvTextureVk, IBvTexture, IBvRenderDeviceObject);

private:
	void Create(const TextureInitData* pInitData);
	void Destroy();

	void CopyInitDataAndTransitionState(const TextureInitData* pInitData, u32 mipCount);
	void GenerateMips(IBvCommandContextVk* pContext);

protected:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	VkImage m_Image = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
	IBvSwapChainVk* m_pSwapChain = nullptr;
	TextureDesc m_TextureDesc;
};


BV_CREATE_CAST_TO_VK(IBvTexture)