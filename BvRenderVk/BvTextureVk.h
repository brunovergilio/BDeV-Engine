#pragma once


#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;
class BvSwapChainVk;
class BvCommandContextVk;


class BvTextureVk final : public BvTexture
{
	BV_NOCOPYMOVE(BvTextureVk);

public:
	BvTextureVk(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, const TextureInitData* pInitData);
	BvTextureVk(BvRenderDeviceVk* pDevice, BvSwapChainVk* pSwapChain, const TextureDesc& textureDesc, VkImage image);
	~BvTextureVk();

	BvRenderDevice* GetDevice() override;

	BV_INLINE VkImage GetHandle() const { return m_Image; }
	BV_INLINE BvSwapChainVk* GetSwapChain() const { return m_pSwapChain; }

private:
	void Create(const TextureInitData* pInitData);
	void Destroy();

	void CopyInitDataAndTransitionState(const TextureInitData* pInitData, u32 mipCount);
	void GenerateMips(BvCommandContextVk* pContext);

protected:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkImage m_Image = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
	BvSwapChainVk* m_pSwapChain = nullptr;
};


BV_CREATE_CAST_TO_VK(BvTexture)