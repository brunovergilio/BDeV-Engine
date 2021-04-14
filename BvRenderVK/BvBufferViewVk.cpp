#include "BvBufferViewVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"


BvBufferViewVk::BvBufferViewVk(const BvRenderDeviceVk & device, const BufferViewDesc & bufferViewDesc)
	: BvBufferView(bufferViewDesc), m_Device(device)
{
}


BvBufferViewVk::~BvBufferViewVk()
{
	Destroy();
}


void BvBufferViewVk::Create()
{
	BvAssert(m_BufferViewDesc.m_pBuffer != nullptr, "Invalid buffer handle");

	if (m_BufferViewDesc.m_Format == Format::kUndefined)
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

	auto result = m_Device.GetDeviceFunctions().vkCreateBufferView(m_Device.GetHandle(), &bufferViewCreateInfo, nullptr, &m_View);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
	}
}


void BvBufferViewVk::Destroy()
{
	if (m_View)
	{
		m_Device.GetDeviceFunctions().vkDestroyBufferView(m_Device.GetHandle(), m_View, nullptr);
		m_View = VK_NULL_HANDLE;
	}
}