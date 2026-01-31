#include "BvTextureVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandContextVk.h"
#include "BvBufferVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"
#include "BvCommandBufferVk.h"


BvTextureVk::BvTextureVk(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, VkImage image, VmaAllocation allocation)
	: m_TextureDesc(textureDesc), m_pDevice(pDevice), m_Image(image), m_VMAAllocation(allocation)
{
}


BvTextureVk::BvTextureVk(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, BvSwapChainVk* pSwapChain, VkImage image)
	: m_TextureDesc(textureDesc), m_pDevice(pDevice), m_pSwapChain(pSwapChain), m_Image(image)
{
}


BvTextureVk::~BvTextureVk()
{
	Destroy();
}


void BvTextureVk::Destroy()
{
	if (!m_pSwapChain)
	{
		if (m_Image)
		{
			VkHelpers::DestroyDeviceObject(*m_pDevice, m_Image, m_pDevice->GetAllocator(), m_VMAAllocation);
			m_Image = VK_NULL_HANDLE;
		}
	}
}