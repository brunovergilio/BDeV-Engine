#include "BvBufferViewVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"


BV_VK_DEVICE_RES_DEF(BvBufferViewVk)


BvBufferViewVk::BvBufferViewVk(BvRenderDeviceVk* pDevice, const BufferViewDesc & bufferViewDesc)
	: m_BufferViewDesc(bufferViewDesc), m_pDevice(pDevice)
{
	Create();
}


BvBufferViewVk::~BvBufferViewVk()
{
	Destroy();
}


void BvBufferViewVk::Create()
{
	BV_ASSERT(m_BufferViewDesc.m_pBuffer != nullptr, "Invalid buffer handle");

	if (m_BufferViewDesc.m_Format == Format::kUnknown)
	{
		return;
	}

	VkBufferViewCreateInfo bufferViewCreateInfo{};
	bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	//bufferViewCreateInfo.pNext = nullptr;
	//bufferViewCreateInfo.flags = 0;
	bufferViewCreateInfo.buffer = reinterpret_cast<BvBufferVk *>(m_BufferViewDesc.m_pBuffer)->GetHandle();
	bufferViewCreateInfo.format = GetVkFormat(m_BufferViewDesc.m_Format);
	bufferViewCreateInfo.offset = m_BufferViewDesc.m_Offset;
	bufferViewCreateInfo.range = m_BufferViewDesc.m_ElementCount * m_BufferViewDesc.m_Stride;

	auto result = vkCreateBufferView(m_pDevice->GetHandle(), &bufferViewCreateInfo, nullptr, &m_View);
	BV_ASSERT(result == VK_SUCCESS, "Failed to create buffer view");
}


void BvBufferViewVk::Destroy()
{
	if (m_View)
	{
		vkDestroyBufferView(m_pDevice->GetHandle(), m_View, nullptr);
		m_View = VK_NULL_HANDLE;
	}
}