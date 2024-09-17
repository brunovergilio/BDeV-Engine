#include "BvTextureViewVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"


BvTextureViewVk::BvTextureViewVk(BvRenderDeviceVk* pDevice, const TextureViewDesc & textureViewDesc)
	: BvTextureView(textureViewDesc), m_pDevice(pDevice)
{
	Create();
}


BvTextureViewVk::~BvTextureViewVk()
{
	Destroy();
}


BvRenderDevice* BvTextureViewVk::GetDevice()
{
	return m_pDevice;
}


bool BvTextureViewVk::Create()
{
	BvAssert(m_TextureViewDesc.m_pTexture != nullptr, "Invalid texture handle");

	decltype(auto) textureViewDesc = m_TextureViewDesc.m_pTexture->GetDesc();
	decltype(auto) vkFormatMap = GetVkFormatMap(m_TextureViewDesc.m_Format);
	BvAssert(vkFormatMap.format != VK_FORMAT_UNDEFINED, "Format not supported in Vulkan");

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
		vkFormatMap.aspectFlags,
		m_TextureViewDesc.m_SubresourceDesc.firstMip,
		m_TextureViewDesc.m_SubresourceDesc.mipCount,
		m_TextureViewDesc.m_SubresourceDesc.firstLayer,
		m_TextureViewDesc.m_SubresourceDesc.layerCount
	};

	auto result = vkCreateImageView(m_pDevice->GetHandle(), &imageViewCreateInfo, nullptr, &m_View);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return false;
	}

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