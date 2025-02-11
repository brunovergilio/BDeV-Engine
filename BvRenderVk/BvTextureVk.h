#pragma once


#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;
class BvSwapChainVk;
class BvCommandContextVk;


BV_OBJECT_DEFINE_ID(BvTextureVk, "be4459ed-ed7b-4674-a638-6eff292841d4");
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
	BV_INLINE bool IsValid() const { return m_Image != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvTextureVk, BvTexture, IBvRenderDeviceObject);

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
BV_OBJECT_ENABLE_ID_OPERATOR(BvTextureVk);


BV_CREATE_CAST_TO_VK(BvTexture)