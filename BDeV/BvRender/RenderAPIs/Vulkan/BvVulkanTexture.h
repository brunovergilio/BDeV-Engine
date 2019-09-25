#pragma once


#include "BvRender/BvRenderDefines.h"


struct TextureDesc
{
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
	uint32_t m_LayerCount = 1;
	uint32_t m_MipLevels = 1;
	VkImageUsageFlags m_UsageFlags = 0;
	VkSampleCountFlags m_SampleCount = VK_SAMPLE_COUNT_1_BIT;
	VkFormat m_Format = VK_FORMAT_UNDEFINED;
	bool m_UseAsCubeMap = false;
	bool m_UseForStenciling = false;
	bool m_Offscreen = true;

	TextureDesc() = default;

	TextureDesc(const uint32_t width, const uint32_t height, const VkFormat format, const VkImageUsageFlags usageFlags,
		const uint32_t layerCount = 1, const VkSampleCountFlags sampleCount = VK_SAMPLE_COUNT_1_BIT, const bool useAsCubeMap = false)
		: m_Width(width), m_Height(height), m_Format(format), m_UsageFlags(usageFlags),
		m_LayerCount(layerCount), m_SampleCount(sampleCount), m_UseAsCubeMap(useAsCubeMap) {}

	BV_INLINE TextureDesc & SetDimensions(const uint32_t width, const uint32_t height) { m_Width = width; m_Height = height; return *this; }
	BV_INLINE TextureDesc & SetFormat(const VkFormat format) { m_Format = format; return *this; }
	BV_INLINE TextureDesc & SetLayerCount(const uint32_t layerCount = 1) { m_LayerCount = layerCount; return *this; }
	BV_INLINE TextureDesc & SetSampleCount(const VkSampleCountFlags sampleCount = VK_SAMPLE_COUNT_1_BIT) { m_SampleCount = sampleCount; return *this; }
	BV_INLINE TextureDesc & SetUsage(const VkImageUsageFlags usageFlags) { m_UsageFlags = usageFlags; return *this; }
	BV_INLINE TextureDesc & UseAsCubeMap(const bool use = false) { m_UseAsCubeMap = use; return *this; }

	BV_INLINE TextureDesc & UseForColor() { m_UsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; return *this; }
	BV_INLINE TextureDesc & UseForDepthStencil() { m_UsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; return *this; }
	BV_INLINE TextureDesc & UseForShaderResource() { m_UsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT; return *this; }
	BV_INLINE TextureDesc & UseForSubpassInput() { m_UsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; return *this; }
	BV_INLINE TextureDesc & UseForTransient() { m_UsageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT; return *this; }
	BV_INLINE TextureDesc & UseForSrcTransfer() { m_UsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; return *this; }
	BV_INLINE TextureDesc & UseForDstTransfer() { m_UsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; return *this; }

	BV_INLINE const VkImageAspectFlags GetAspectMask() const
	{
		VkImageAspectFlags aspectMask = 0;
		if (m_UsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		else
		{
			const VkFormat formats[] =
			{
				VK_FORMAT_D16_UNORM,
				VK_FORMAT_X8_D24_UNORM_PACK32,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_S8_UINT
			};

			for (auto i = 0U; i < 7; ++i)
			{
				if (m_Format == formats[i])
				{
					if (i < 6) { aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT; }
					if (i > 2) { aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT; }
					break;
				}
			}
		}

		return aspectMask;
	}

	BV_INLINE const bool IsDepthTexture() const { return GetAspectMask() & VK_IMAGE_ASPECT_DEPTH_BIT; }
	BV_INLINE const bool IsStencilTexture() const { return GetAspectMask() & VK_IMAGE_ASPECT_STENCIL_BIT; }
};


class BvVulkanDevice;


class BvVulkanTexture
{
public:
	// This will create an image (with its own memory) and an image view
	// If the third parameter is specified, only an image view is created, based on the provided srcImage and the texture description
	BvVulkanTexture(const BvVulkanDevice * const pDevice, const TextureDesc & textureDesc, const VkImage srcImage = VK_NULL_HANDLE);
	~BvVulkanTexture();

	// Used for when a buffer resize is needed
	// Previous resources are destroyed
	void Recreate(const TextureDesc & textureDesc, const VkImage srcImage = VK_NULL_HANDLE);

	BV_INLINE VkImage GetImage() const { return m_Image; }
	BV_INLINE VkImageView GetView() const { return m_View; }
	BV_INLINE const TextureDesc & GetTextureDesc() const { return m_TextureDesc; }

private:
	void Create();
	void Destroy();

private:
	const BvVulkanDevice * const m_pDevice = nullptr;
	VkImage m_Image = VK_NULL_HANDLE;
	VkImageView m_View = VK_NULL_HANDLE;
	VkDeviceMemory m_Memory = VK_NULL_HANDLE;
	TextureDesc m_TextureDesc;
};