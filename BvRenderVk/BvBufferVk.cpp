#include "BvBufferVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvCommandContextVk.h"
#include "BvTypeConversionsVk.h"



BvBufferVk::BvBufferVk(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc, VkBuffer buffer, VkDeviceAddress deviceAddress,
	VmaAllocation memory, void* pMappedMemory, bool needsFlush)
	: m_pDevice(pDevice), m_BufferDesc(bufferDesc), m_Buffer(buffer), m_DeviceAddress(deviceAddress), m_VMAAllocation(memory),
	m_pMapped(pMappedMemory), m_NeedsFlush(needsFlush)
{
}


BvBufferVk::~BvBufferVk()
{
	Destroy();
}


void* const BvBufferVk::Map(const u64 size, const u64 offset)
{
	if (!m_pMapped)
	{
		auto vma = m_pDevice->GetAllocator();
		auto result = vmaMapMemory(vma, m_VMAAllocation, &m_pMapped);
		BV_ASSERT(result == VK_SUCCESS, "Failed to map buffer memory");
	}

	return m_pMapped;
}


void BvBufferVk::Unmap()
{
	if (m_pMapped)
	{
		auto vma = m_pDevice->GetAllocator();
		vmaUnmapMemory(vma, m_VMAAllocation);

		m_pMapped = nullptr;
	}
}


void BvBufferVk::Flush(const u64 size, const u64 offset) const
{
	if (!m_NeedsFlush)
	{
		return;
	}

	auto vma = m_pDevice->GetAllocator();
	VmaAllocationInfo vmaAI;
	vmaGetAllocationInfo(vma, m_VMAAllocation, &vmaAI);

	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = vmaAI.deviceMemory;
	mappedRange.offset = vmaAI.offset + offset;
	mappedRange.size = size;
	auto result = vkFlushMappedMemoryRanges(m_pDevice->GetHandle(), 1, &mappedRange);
	BV_ASSERT(result == VK_SUCCESS, "Failed to flush buffer memory");
}


void BvBufferVk::Invalidate(const u64 size, const u64 offset) const
{
	if (!m_NeedsFlush)
	{
		return;
	}

	auto vma = m_pDevice->GetAllocator();
	VmaAllocationInfo vmaAI;
	vmaGetAllocationInfo(vma, m_VMAAllocation, &vmaAI);

	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = vmaAI.deviceMemory;
	mappedRange.offset = vmaAI.offset + offset;
	mappedRange.size = size;
	auto result = vkInvalidateMappedMemoryRanges(m_pDevice->GetHandle(), 1, &mappedRange);
	BV_ASSERT(result == VK_SUCCESS, "Failed to invalidate buffer memory");
}


void BvBufferVk::Destroy()
{
	if (m_Buffer)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Buffer, m_pDevice->GetAllocator(), m_VMAAllocation);
		m_Buffer = VK_NULL_HANDLE;
	}
}