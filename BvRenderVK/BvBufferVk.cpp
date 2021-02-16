#include "BvBufferVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"


BvBufferVk::BvBufferVk(const BvRenderDeviceVk & device, const BufferDesc & bufferDesc)
	: BvBuffer(bufferDesc), m_Device(device)
{
}


BvBufferVk::~BvBufferVk()
{
	Destroy();
}


void BvBufferVk::Create()
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//bufferCreateInfo.pNext = nullptr;
	//bufferCreateInfo.flags = 0; // No Sparse Binding for now
	bufferCreateInfo.size = m_BufferDesc.m_Size;
	bufferCreateInfo.usage = GetVkBufferUsageFlags(m_BufferDesc.m_UsageFlags);
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//bufferCreateInfo.queueFamilyIndexCount = 0;
	//bufferCreateInfo.pQueueFamilyIndices = nullptr;


	auto device = m_Device.GetHandle();
	auto result = m_Device.GetDeviceFunctions().vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_Buffer);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
	}

	VkMemoryRequirements reqs{};
	m_Device.GetDeviceFunctions().vkGetBufferMemoryRequirements(device, m_Buffer, &reqs);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = reqs.size;
	allocateInfo.memoryTypeIndex = m_Device.GetMemoryTypeIndex(reqs.memoryTypeBits, GetVkMemoryPropertyFlags(m_BufferDesc.m_MemoryFlags));

	result = m_Device.GetDeviceFunctions().vkAllocateMemory(device, &allocateInfo, nullptr, &m_Memory);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		m_Device.GetDeviceFunctions().vkDestroyBuffer(device, m_Buffer, nullptr);
	}

	result = m_Device.GetDeviceFunctions().vkBindBufferMemory(device, m_Buffer, m_Memory, 0);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		m_Device.GetDeviceFunctions().vkDestroyBuffer(device, m_Buffer, nullptr);
		m_Device.GetDeviceFunctions().vkFreeMemory(device, m_Memory, nullptr);
	}
}


void BvBufferVk::Destroy()
{
	auto device = m_Device.GetHandle();
	if (m_Buffer)
	{
		m_Device.GetDeviceFunctions().vkDestroyBuffer(device, m_Buffer, nullptr);
	}
	if (m_Memory)
	{
		m_Device.GetDeviceFunctions().vkFreeMemory(device, m_Memory, nullptr);
	}
}


void * const BvBufferVk::Map(const u64 size, const u64 offset)
{
	void *pMappedMemory = nullptr;
	auto result = m_Device.GetDeviceFunctions().vkMapMemory(m_Device.GetHandle(), m_Memory, offset, size, 0, &pMappedMemory);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return nullptr;
	}

	return pMappedMemory;
}


void BvBufferVk::Unmap()
{
	m_Device.GetDeviceFunctions().vkUnmapMemory(m_Device.GetHandle(), m_Memory);
}


void BvBufferVk::Flush(const u64 size, const u64 offset) const
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = m_Memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	auto result = m_Device.GetDeviceFunctions().vkFlushMappedMemoryRanges(m_Device.GetHandle(), 1, &mappedRange);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}
}


void BvBufferVk::Invalidate(const u64 size, const u64 offset) const
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = m_Memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	auto result = m_Device.GetDeviceFunctions().vkInvalidateMappedMemoryRanges(m_Device.GetHandle(), 1, &mappedRange);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}
}