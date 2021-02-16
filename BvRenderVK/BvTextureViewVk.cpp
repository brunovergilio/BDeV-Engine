#include "BvTextureViewVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"


BvTextureViewVk::BvTextureViewVk(const BvRenderDeviceVk & device, const TextureViewDesc & textureViewDesc)
	: BvTextureView(textureViewDesc), m_Device(device)
{
}


BvTextureViewVk::~BvTextureViewVk()
{
	Destroy();
}


bool BvTextureViewVk::Create()
{
	BvAssert(m_TextureViewDesc.m_pTexture != nullptr);

	decltype(auto) textureDesc = m_TextureViewDesc.m_pTexture->GetDesc();

	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = m_TextureViewDesc.m_pTexture->GetClassType() == BvTexture::ClassType::kTexture ?
		reinterpret_cast<BvTextureVk*>(m_TextureViewDesc.m_pTexture)->GetHandle() :
		reinterpret_cast<BvSwapChainTextureVk*>(m_TextureViewDesc.m_pTexture)->GetHandle();
	imageViewCreateInfo.viewType = GetVkImageViewType(m_TextureViewDesc.m_ViewType);
	imageViewCreateInfo.format = GetVkFormat(textureDesc.m_Format);
	imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imageViewCreateInfo.subresourceRange =
	{
		GetVkAspectMaskFlags(imageViewCreateInfo.format),
		m_TextureViewDesc.m_SubresourceDesc.firstMip,
		m_TextureViewDesc.m_SubresourceDesc.mipCount,
		m_TextureViewDesc.m_SubresourceDesc.firstLayer,
		m_TextureViewDesc.m_SubresourceDesc.layerCount
	};

	auto result = m_Device.GetDeviceFunctions().vkCreateImageView(m_Device.GetHandle(), &imageViewCreateInfo, nullptr, &m_View);
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
		m_Device.GetDeviceFunctions().vkDestroyImageView(m_Device.GetHandle(), m_View, nullptr);
		m_View = VK_NULL_HANDLE;
	}
}