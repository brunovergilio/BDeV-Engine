#include "BvTextureVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"
#include <vma/vk_mem_alloc.h>


BvTextureVk::BvTextureVk(const BvRenderDeviceVk & device, const TextureDesc & textureDesc)
	: BvTexture(textureDesc), m_Device(device)
{
	Create();
}


BvTextureVk::~BvTextureVk()
{
	Destroy();
}


void BvTextureVk::Create()
{
	VkImageCreateFlags imageCreateFlags = 0;
	if (m_TextureDesc.m_UseAsCubeMap)
	{
		imageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	auto device = m_Device.GetHandle();

	u32 layerCount = 0;
	u32 depth = 0;
	switch (m_TextureDesc.m_ImageType)
	{
	case TextureType::kTexture1D:
		layerCount = m_TextureDesc.m_Size.height;
		depth = 1;
		break;
	case TextureType::kTexture2D:
		layerCount = m_TextureDesc.m_Size.depthOrLayerCount;
		depth = 1;
		break;
	case TextureType::kTexture3D:
		layerCount = 1;
		depth = m_TextureDesc.m_Size.depthOrLayerCount;
		break;
	}

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = imageCreateFlags;
	imageCreateInfo.imageType = GetVkImageType(m_TextureDesc.m_ImageType);
	imageCreateInfo.format = GetVkFormat(m_TextureDesc.m_Format);
	imageCreateInfo.extent = { m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height, depth };
	imageCreateInfo.mipLevels = m_TextureDesc.m_MipLevels;
	imageCreateInfo.arrayLayers = layerCount;
	imageCreateInfo.samples = (VkSampleCountFlagBits)m_TextureDesc.m_SampleCount;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = GetVkImageUsageFlags(m_TextureDesc.m_UsageFlags);
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	auto result = vkCreateImage(device, &imageCreateInfo, nullptr, &m_Image);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}

	auto vma = m_Device.GetAllocator();
	VmaAllocationCreateInfo vmaACI = {};
	vmaACI.requiredFlags = GetVkMemoryPropertyFlags(m_TextureDesc.m_MemoryFlags);

	VmaAllocationInfo vmaAI;
	VmaAllocation vmaA;
	result = vmaAllocateMemoryForImage(vma, m_Image, &vmaACI, &vmaA, &vmaAI);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		vkDestroyImage(device, m_Image, nullptr);
		return;
	}

	result = vkBindImageMemory(device, m_Image, vmaAI.deviceMemory, vmaAI.offset);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		vkDestroyImage(device, m_Image, nullptr);
		vmaFreeMemory(vma, m_VMAAllocation);
	}
	m_VMAAllocation = vmaA;
}


void BvTextureVk::Destroy()
{
	auto device = m_Device.GetHandle();

	if (m_Image)
	{
		vkDestroyImage(device, m_Image, nullptr);
	}
	vmaFreeMemory(m_Device.GetAllocator(), m_VMAAllocation);
}


BvSwapChainTextureVk::BvSwapChainTextureVk(BvSwapChainVk * pSwapChain, const TextureDesc & textureDesc, VkImage image)
	: BvTexture(textureDesc), m_pSwapChain(pSwapChain), m_Image(image)
{
}


BvSwapChainTextureVk::~BvSwapChainTextureVk()
{
}