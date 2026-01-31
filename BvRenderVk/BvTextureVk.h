#pragma once


#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;
class BvSwapChainVk;
class BvCommandContextVk;


class BvTextureVk final : public IBvTexture, public IBvResourceVk
{
public:
	BvTextureVk(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, VkImage image, VmaAllocation allocation);
	BvTextureVk(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, BvSwapChainVk* pSwapChain, VkImage image);
	~BvTextureVk();

	BV_INLINE const TextureDesc& GetDesc() const override { return m_TextureDesc; }
	BV_INLINE VkImage GetHandle() const { return m_Image; }
	BV_INLINE BvSwapChainVk* GetSwapChain() const { return m_pSwapChain; }
	BV_INLINE bool IsValid() const { return m_Image != VK_NULL_HANDLE; }

private:
	void Destroy();

protected:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkImage m_Image = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
	BvSwapChainVk* m_pSwapChain = nullptr;
	TextureDesc m_TextureDesc;
};
BV_OBJECT_DEFINE_ID(BvTextureVk, "be4459ed-ed7b-4674-a638-6eff292841d4");
BV_CREATE_CAST_TO_VK(BvTexture)