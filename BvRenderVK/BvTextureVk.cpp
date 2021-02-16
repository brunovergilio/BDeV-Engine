#include "BvTextureVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"


BvTextureVk::BvTextureVk(const BvRenderDeviceVk & device, const TextureDesc & textureDesc)
	: BvTexture(textureDesc), m_Device(device)
{
}


BvTextureVk::~BvTextureVk()
{
	Destroy();
}


bool BvTextureVk::Create()
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

	auto result = m_Device.GetDeviceFunctions().vkCreateImage(device, &imageCreateInfo, nullptr, &m_Image);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return false;
	}

	VkMemoryRequirements reqs{};
	m_Device.GetDeviceFunctions().vkGetImageMemoryRequirements(device, m_Image, &reqs);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = reqs.size;
	allocateInfo.memoryTypeIndex = m_Device.GetMemoryTypeIndex(reqs.memoryTypeBits, GetVkMemoryPropertyFlags(m_TextureDesc.m_MemoryFlags));

	result = m_Device.GetDeviceFunctions().vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		m_Device.GetDeviceFunctions().vkDestroyImage(device, m_Image, nullptr);
		return false;
	}

	result = m_Device.GetDeviceFunctions().vkBindImageMemory(device, m_Image, m_Memory, 0);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		m_Device.GetDeviceFunctions().vkDestroyImage(device, m_Image, nullptr);
		m_Device.GetDeviceFunctions().vkFreeMemory(device, m_Memory, nullptr);
		return false;
	}

	return true;
}


void BvTextureVk::Destroy()
{
	auto device = m_Device.GetHandle();

	// Because we also account for textures from the swap chain, we do this simple check for the memory first
	// Swapchain images will have their textures automatically created by vulkan when one is created, so we
	// check for the device memory first. If there's no memory handle, then we know it's a swapchain
	// image, so don't bother destroying it
	if (m_Memory)
	{
		if (m_Image)
		{
			m_Device.GetDeviceFunctions().vkDestroyImage(device, m_Image, nullptr);
		}
		m_Device.GetDeviceFunctions().vkFreeMemory(device, m_Memory, nullptr);
	}
	m_Image = VK_NULL_HANDLE;
}


BvSwapChainTextureVk::BvSwapChainTextureVk(BvSwapChainVk * pSwapChain, const TextureDesc & textureDesc, VkImage image)
	: BvTexture(textureDesc), m_pSwapChain(pSwapChain), m_Image(image)
{
}


BvSwapChainTextureVk::~BvSwapChainTextureVk()
{
}