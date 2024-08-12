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
	BvTextureVk(const BvRenderDeviceVk& device, const TextureDesc& textureDesc, const TextureInitData* pInitData);
	~BvTextureVk();

	void Create(const TextureInitData* pInitData);
	void Destroy();

	BV_INLINE VkImage GetHandle() const { return m_Image; }
	BV_INLINE ClassType GetClassType() const override { return ClassType::kTexture; }

private:
	void CopyInitDataAndTransitionState(const TextureInitData* pInitData, u32 mipCount);

protected:
	const BvRenderDeviceVk& m_Device;
	VkImage m_Image = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
};


class BvSwapChainTextureVk final : public BvTexture
{
	BV_NOCOPYMOVE(BvSwapChainTextureVk);

public:
	BvSwapChainTextureVk(BvSwapChainVk* pSwapChain, const TextureDesc& textureDesc, VkImage image);
	~BvSwapChainTextureVk();

	BV_INLINE BvSwapChainVk* GetSwapChain() const { return m_pSwapChain; }
	BV_INLINE VkImage GetHandle() const { return m_Image; }
	BV_INLINE ClassType GetClassType() const override { return ClassType::kSwapChainTexture; }

private:
	BvSwapChainVk* m_pSwapChain = nullptr;
	VkImage m_Image = VK_NULL_HANDLE;
};


BV_CREATE_CAST_TO_VK(BvTexture)