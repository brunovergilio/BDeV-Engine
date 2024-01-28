#include "BvBufferVk.h"
#include "BvRenderVk/BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"
#include <VulkanMemoryAllocator/include/vk_mem_alloc.h>


BvBufferVk::BvBufferVk(const BvRenderDeviceVk & device, const BufferDesc & bufferDesc)
	: BvBuffer(bufferDesc), m_Device(device)
{
	Create();
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
	auto result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_Buffer);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
	}

	auto vma = m_Device.GetAllocator();
	VmaAllocationCreateInfo vmaACI = {};
	vmaACI.requiredFlags = GetVkMemoryPropertyFlags(m_BufferDesc.m_MemoryFlags);

	VmaAllocationInfo vmaAI;
	VmaAllocation vmaA;
	result = vmaAllocateMemoryForBuffer(vma, m_Buffer, &vmaACI, &vmaA, &vmaAI);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		vkDestroyBuffer(device, m_Buffer, nullptr);
		return;
	}

	result = vkBindBufferMemory(device, m_Buffer, vmaAI.deviceMemory, vmaAI.offset);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		vmaFreeMemory(vma, m_VMAAllocation);
	}
	m_VMAAllocation = vmaA;
}


void BvBufferVk::Destroy()
{
	auto device = m_Device.GetHandle();
	auto vma = m_Device.GetAllocator();
	if (m_pMapped)
	{
		vmaUnmapMemory(vma, m_VMAAllocation);
	}
	if (m_Buffer)
	{
		vkDestroyBuffer(device, m_Buffer, nullptr);
	}
	vmaFreeMemory(m_Device.GetAllocator(), m_VMAAllocation);
}


void * const BvBufferVk::Map(const u64 size, const u64 offset)
{
	auto vma = m_Device.GetAllocator();
	auto result = vmaMapMemory(vma, m_VMAAllocation, &m_pMapped);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return nullptr;
	}

	return m_pMapped;
}


void BvBufferVk::Unmap()
{
	auto vma = m_Device.GetAllocator();
	vmaUnmapMemory(vma, m_VMAAllocation);

	m_pMapped = nullptr;
}


void BvBufferVk::Flush(const u64 size, const u64 offset) const
{
	auto vma = m_Device.GetAllocator();
	VmaAllocationInfo vmaAI;
	vmaGetAllocationInfo(vma, m_VMAAllocation, &vmaAI);

	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = vmaAI.deviceMemory;
	mappedRange.offset = vmaAI.offset + offset;
	mappedRange.size = size;
	auto result = vkFlushMappedMemoryRanges(m_Device.GetHandle(), 1, &mappedRange);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}
}


void BvBufferVk::Invalidate(const u64 size, const u64 offset) const
{
	auto vma = m_Device.GetAllocator();
	VmaAllocationInfo vmaAI;
	vmaGetAllocationInfo(vma, m_VMAAllocation, &vmaAI);

	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = vmaAI.deviceMemory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	auto result = vkInvalidateMappedMemoryRanges(m_Device.GetHandle(), 1, &mappedRange);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}
}