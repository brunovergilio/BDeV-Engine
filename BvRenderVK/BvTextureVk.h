#pragma once


#include "BDeV/Render/BvTexture.h"
#include "BvCommonVk.h"
#include <vma/vk_mem_alloc.h>


class BvRenderDeviceVk;
class BvSwapChainVk;


class BvTextureVk final : public BvTexture
{
	BV_NOCOPYMOVE(BvTextureVk);

public:
	BvTextureVk(const BvRenderDeviceVk & device, const TextureDesc & textureDesc);
	~BvTextureVk();

	void Create();
	void Destroy();

	BV_INLINE VkImage GetHandle() const { return m_Image; }
	BV_INLINE ClassType GetClassType() const override { return ClassType::kTexture; }

protected:
	const BvRenderDeviceVk & m_Device;
	VkImage m_Image = VK_NULL_HANDLE;
	VmaAllocation m_VMAAllocation = nullptr;
};


class BvSwapChainTextureVk final : public BvTexture
{
	BV_NOCOPYMOVE(BvSwapChainTextureVk);

public:
	BvSwapChainTextureVk(BvSwapChainVk * pSwapChain, const TextureDesc & textureDesc, VkImage image);
	~BvSwapChainTextureVk();

	BV_INLINE BvSwapChainVk * GetSwapChain() const { return m_pSwapChain; }
	BV_INLINE VkImage GetHandle() const { return m_Image; }
	BV_INLINE ClassType GetClassType() const override { return ClassType::kSwapChainTexture; }

private:
	BvSwapChainVk * m_pSwapChain = nullptr;
	VkImage m_Image = VK_NULL_HANDLE;
};