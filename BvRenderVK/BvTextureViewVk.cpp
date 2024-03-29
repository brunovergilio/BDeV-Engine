#include "BvTextureViewVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"


BvTextureViewVk::BvTextureViewVk(const BvRenderDeviceVk & device, const TextureViewDesc & textureViewDesc)
	: BvTextureView(textureViewDesc), m_Device(device)
{
	Create();
}


BvTextureViewVk::~BvTextureViewVk()
{
	Destroy();
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
	imageViewCreateInfo.image = m_TextureViewDesc.m_pTexture->GetClassType() == BvTexture::ClassType::kTexture ?
		reinterpret_cast<BvTextureVk*>(m_TextureViewDesc.m_pTexture)->GetHandle() :
		reinterpret_cast<BvSwapChainTextureVk*>(m_TextureViewDesc.m_pTexture)->GetHandle();
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

	auto result = vkCreateImageView(m_Device.GetHandle(), &imageViewCreateInfo, nullptr, &m_View);
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
		vkDestroyImageView(m_Device.GetHandle(), m_View, nullptr);
		m_View = VK_NULL_HANDLE;
	}
}