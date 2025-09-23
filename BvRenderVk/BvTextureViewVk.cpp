#include "BvTextureViewVk.h"
#include "BvTextureVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"


BV_VK_DEVICE_RES_DEF(BvTextureViewVk)


BvTextureViewVk::BvTextureViewVk(BvRenderDeviceVk* pDevice, const TextureViewDesc & textureViewDesc)
	: m_TextureViewDesc(textureViewDesc), m_pDevice(pDevice)
{
	Create();
}


BvTextureViewVk::~BvTextureViewVk()
{
	Destroy();
}


bool BvTextureViewVk::Create()
{
	BV_ASSERT(m_TextureViewDesc.m_pTexture != nullptr, "Invalid texture handle");

	decltype(auto) textureViewDesc = m_TextureViewDesc.m_pTexture->GetDesc();
	decltype(auto) vkFormatMap = GetVkFormatMap(m_TextureViewDesc.m_Format);
	BV_ASSERT(vkFormatMap.format != VK_FORMAT_UNDEFINED, "Format not supported in Vulkan");

	VkImageAspectFlags aspectFlags = vkFormatMap.aspectFlags;
	if ((vkFormatMap.aspectFlags & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		&& EHasAnyFlags(m_TextureViewDesc.m_pTexture->GetDesc().m_UsageFlags, TextureUsage::kShaderResource | TextureUsage::kInputAttachment))
	{
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (vkFormatMap.aspectFlags & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))
	{
		switch (m_TextureViewDesc.m_SubresourceDesc.planeSlice)
		{
		case 0: aspectFlags = VK_IMAGE_ASPECT_PLANE_0_BIT; break;
		case 1: aspectFlags = VK_IMAGE_ASPECT_PLANE_1_BIT; break;
		case 2: aspectFlags = VK_IMAGE_ASPECT_PLANE_2_BIT; break;
		default:
			BV_ASSERT(m_TextureViewDesc.m_SubresourceDesc.planeSlice > 2, "Invalid plane slice");
		}
	}

	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = TO_VK(m_TextureViewDesc.m_pTexture)->GetHandle();
	imageViewCreateInfo.viewType = GetVkImageViewType(m_TextureViewDesc.m_ViewType);
	imageViewCreateInfo.format = vkFormatMap.format;
	imageViewCreateInfo.components = vkFormatMap.componentMapping;
	imageViewCreateInfo.subresourceRange =
	{
		aspectFlags,
		m_TextureViewDesc.m_SubresourceDesc.firstMip,
		m_TextureViewDesc.m_SubresourceDesc.mipCount,
		m_TextureViewDesc.m_SubresourceDesc.firstLayer,
		m_TextureViewDesc.m_SubresourceDesc.layerCount
	};

	auto result = vkCreateImageView(m_pDevice->GetHandle(), &imageViewCreateInfo, nullptr, &m_View);
	BV_ASSERT(result == VK_SUCCESS, "Failed to create image view");

	return true;
}


void BvTextureViewVk::Destroy()
{
	if (m_View)
	{
		vkDestroyImageView(m_pDevice->GetHandle(), m_View, nullptr);
		m_View = VK_NULL_HANDLE;
	}
}