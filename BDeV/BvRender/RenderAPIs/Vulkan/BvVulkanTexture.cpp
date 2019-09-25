#include "BvVulkanTexture.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"


BvVulkanTexture::BvVulkanTexture(const BvVulkanDevice * const pDevice, const TextureDesc & textureDesc, const VkImage srcImage)
	: m_pDevice(pDevice), m_TextureDesc(textureDesc), m_Image(srcImage)
{
	Create();
}


BvVulkanTexture::~BvVulkanTexture()
{
	Destroy();
}


void BvVulkanTexture::Recreate(const TextureDesc & textureDesc, const VkImage srcImage)
{
	Destroy();

	m_TextureDesc = textureDesc;
	m_Image = srcImage;

	Create();
}


void BvVulkanTexture::Create()
{
	VkImageCreateFlags imageCreateFlags = 0;
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
	u32 layers = m_TextureDesc.m_LayerCount;
	if (m_TextureDesc.m_LayerCount > 1 && !m_TextureDesc.m_UseAsCubeMap)
	{
		viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	}
	if (m_TextureDesc.m_UseAsCubeMap)
	{
		viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		if (layers > 1)
		{
			viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}
		layers *= 6;
	}

	auto device = m_pDevice->GetLogical();

	// If an image wasn't provided then we create one
	if (m_Image == VK_NULL_HANDLE)
	{
		VkImageCreateInfo imageCreateInfo
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			nullptr,
			imageCreateFlags,
			VK_IMAGE_TYPE_2D,
			m_TextureDesc.m_Format,
			{ m_TextureDesc.m_Width, m_TextureDesc.m_Height, 1 },
			m_TextureDesc.m_MipLevels,
			layers,
			(VkSampleCountFlagBits)m_TextureDesc.m_SampleCount,
			VK_IMAGE_TILING_OPTIMAL,
			m_TextureDesc.m_UsageFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			VK_IMAGE_LAYOUT_UNDEFINED
		};

		vkCreateImage(device, &imageCreateInfo, nullptr, &m_Image);

		VkMemoryRequirements reqs{};
		vkGetImageMemoryRequirements(device, m_Image, &reqs);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = reqs.size;
		allocateInfo.memoryTypeIndex = m_pDevice->GetMemoryTypeIndex(reqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | ((imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0 ?
			VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : 0));

		vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory);
		vkBindImageMemory(device, m_Image, m_Memory, 0);
	}

	VkImageViewCreateInfo imageViewCreateInfo
	{
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		nullptr,
		0,
		m_Image,
		viewType,
		m_TextureDesc.m_Format,
		{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
		{ m_TextureDesc.GetAspectMask(), 0, m_TextureDesc.m_MipLevels, 0, layers }
	};

	vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_View);
}


void BvVulkanTexture::Destroy()
{
	auto device = m_pDevice->GetLogical();

	if (m_View)
	{
		vkDestroyImageView(device, m_View, nullptr);
		m_View = VK_NULL_HANDLE;
	}
	// If this image doesn't have a memory associated with it, then that means this was a view-only texture
	// So we don't destroy it here
	if (m_Memory)
	{
		if (m_Image)
		{
			vkDestroyImage(device, m_Image, nullptr);
		}

		vkFreeMemory(device, m_Memory, nullptr);
		m_Memory = VK_NULL_HANDLE;
	}
	m_Image = VK_NULL_HANDLE;
}