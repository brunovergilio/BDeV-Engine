#include "BvBufferViewVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"
#include "BvRenderDeviceVk.h"


BvBufferViewVk::BvBufferViewVk(BvRenderDeviceVk* pDevice, const BufferViewDesc& bufferViewDesc, VkBufferView view)
	: m_BufferViewDesc(bufferViewDesc), m_pDevice(pDevice), m_View(view)
{
}


BvBufferViewVk::~BvBufferViewVk()
{
	Destroy();
}


void BvBufferViewVk::Destroy()
{
	if (m_View)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_View);
		if (m_View != VK_NULL_HANDLE)
		{
			m_pDevice->OnVkHandleDestroyed(u64(m_View), false);
		}
		m_View = VK_NULL_HANDLE;
	}
}