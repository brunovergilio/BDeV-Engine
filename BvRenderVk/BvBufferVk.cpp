#include "BvBufferVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvCommandContextVk.h"
#include "BvTypeConversionsVk.h"


BvBufferVk::BvBufferVk(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc, const BufferInitData* pInitData)
	: m_BufferDesc(bufferDesc), m_pDevice(pDevice)
{
	Create(pInitData);
}


BvBufferVk::~BvBufferVk()
{
	Destroy();
}


void * const BvBufferVk::Map(const u64 size, const u64 offset)
{
	if (!m_pMapped)
	{
		auto vma = m_pDevice->GetAllocator();
		auto result = vmaMapMemory(vma, m_VMAAllocation, &m_pMapped);
		if (result != VK_SUCCESS)
		{
			BvDebugVkResult(result);
			return nullptr;
		}
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
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}
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
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}
}


IBvRenderDevice* BvBufferVk::GetDevice()
{
	return m_pDevice;
}


void BvBufferVk::Create(const BufferInitData* pInitData)
{
	VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	//bufferCreateInfo.pNext = nullptr;
	//bufferCreateInfo.flags = 0; // No Sparse Binding for now
	bufferCreateInfo.size = m_BufferDesc.m_Size;
	bufferCreateInfo.usage = GetVkBufferUsageFlags(m_BufferDesc.m_UsageFlags, m_BufferDesc.m_Formatted);
	if (m_pDevice->GetDeviceInfo()->m_DeviceFeatures1_2.bufferDeviceAddress)
	{
		bufferCreateInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	}
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//bufferCreateInfo.queueFamilyIndexCount = 0;
	//bufferCreateInfo.pQueueFamilyIndices = nullptr;

	auto device = m_pDevice->GetHandle();
	auto result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_Buffer);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}

	auto vma = m_pDevice->GetAllocator();
	VmaAllocationCreateInfo vmaACI = {};
	vmaACI.requiredFlags = GetVkMemoryPropertyFlags(m_BufferDesc.m_MemoryType);
	vmaACI.preferredFlags = vmaACI.requiredFlags | GetPreferredVkMemoryPropertyFlags(m_BufferDesc.m_MemoryType);

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
		vmaFreeMemory(vma, vmaA);
		return;
	}
	m_VMAAllocation = vmaA;

	if (bufferCreateInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		VkBufferDeviceAddressInfo addressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, m_Buffer };
		m_DeviceAddress = vkGetBufferDeviceAddress(device, &addressInfo);
	}

	const auto& memoryType = m_pDevice->GetDeviceInfo()->m_DeviceMemoryProperties.memoryProperties.memoryTypes[vmaAI.memoryType];
	m_NeedsFlush = ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0);

	if (m_BufferDesc.m_MemoryType != MemoryType::kDevice
		&& (EHasFlag(m_BufferDesc.m_CreateFlags, BufferCreateFlags::kCreateMapped)))
	{
		Map(kU64Max, 0);
	}

	if (m_BufferDesc.m_ResourceState != ResourceState::kCommon || pInitData)
	{
		if (m_BufferDesc.m_MemoryType == MemoryType::kDevice)
		{
			CopyInitDataAndTransitionState(pInitData);
		}
		else
		{
			BV_ASSERT(m_BufferDesc.m_MemoryType != MemoryType::kReadBack, "Readback shouldn't have initial data");

			auto pMappedData = GetMappedData();
			if (!pMappedData)
			{
				pMappedData = Map(kU64Max, 0);
			}

			memcpy(pMappedData, pInitData->m_pData, pInitData->m_Size);

			if (m_NeedsFlush)
			{
				Flush(kU64Max, 0);
			}
		}
	}
}


void BvBufferVk::Destroy()
{
	auto device = m_pDevice->GetHandle();
	Unmap();
	if (m_Buffer)
	{
		vkDestroyBuffer(device, m_Buffer, nullptr);
		m_Buffer = VK_NULL_HANDLE;
		
		vmaFreeMemory(m_pDevice->GetAllocator(), m_VMAAllocation);
	}
}


void BvBufferVk::CopyInitDataAndTransitionState(const BufferInitData* pInitData)
{
	BV_ASSERT(pInitData->m_pContext != nullptr && pInitData->m_Size > 0 && pInitData->m_pData != nullptr, "Invalid init data parameters");

	BufferDesc bufferDesc;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_Size = pInitData->m_Size;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	
	BvBufferVk srcBuffer(m_pDevice, bufferDesc, pInitData);
	VkBufferCopy copyRegion{};
	copyRegion.size = std::min(m_BufferDesc.m_Size, pInitData->m_Size);

	auto pContext = static_cast<BvCommandContextVk*>(pInitData->m_pContext);
	pContext->NewCommandList();
	pContext->CopyBufferVk(&srcBuffer, this, copyRegion);

	VkBufferMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
	//barrier.pNext = nullptr;
	barrier.buffer = m_Buffer;
	barrier.size = VK_WHOLE_SIZE;
	//barrier.offset = 0;

	barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
	barrier.dstAccessMask = GetVkAccessFlags(m_BufferDesc.m_UsageFlags);

	barrier.srcStageMask |= VK_PIPELINE_STAGE_2_HOST_BIT;
	barrier.dstStageMask |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	
	pContext->ResourceBarrierVk(1, &barrier, 0, nullptr, 0, nullptr);
	pContext->Execute();
	pContext->WaitForGPU();
}