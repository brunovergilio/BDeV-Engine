#include "BvTextureViewVk.h"
#include "BvTextureVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvTextureViewVk::BvTextureViewVk(BvRenderDeviceVk* pDevice, const TextureViewDesc & textureViewDesc, VkImageView view)
	: m_TextureViewDesc(textureViewDesc), m_pDevice(pDevice), m_View(view)
{
}


BvTextureViewVk::~BvTextureViewVk()
{
	Destroy();
}


void BvTextureViewVk::Destroy()
{
	if (m_View)
	{
		vkDestroyImageView(m_pDevice->GetHandle(), m_View, nullptr);
		m_pDevice->OnVkHandleDestroyed(u64(m_View), true);
		m_View = VK_NULL_HANDLE;
	}
}