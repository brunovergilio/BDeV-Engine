#include "BvRenderPassVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"


BvRenderPassVk::BvRenderPassVk(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc, VkRenderPass renderPass)
	: m_RenderPassDesc(renderPassDesc), m_pDevice(pDevice), m_RenderPass(renderPass)
{
}


BvRenderPassVk::~BvRenderPassVk()
{
	Destroy();
}


void BvRenderPassVk::Destroy()
{
	if (m_RenderPass)
	{
		VkHelpers::DestroyDeviceObject(m_pDevice->GetHandle(), m_RenderPass);
		m_RenderPass = VK_NULL_HANDLE;
	}
}