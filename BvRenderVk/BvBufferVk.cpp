#include "BvBufferVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvCommandContextVk.h"
#include "BvTypeConversionsVk.h"


BV_VK_DEVICE_RES_DEF(BvBufferVk)


BvBufferVk::BvBufferVk(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc, const BufferInitData* pInitData)
	: m_BufferDesc(bufferDesc), m_pDevice(pDevice)
{
	Create(pInitData);
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
	if (m_NeedsFlush)
	{
		if (m_BufferDesc.m_MemoryType == MemoryType::kReadBack)
		{
			m_BufferDesc.m_MemoryType = MemoryType::kReadBackNC;
		}
		else if (m_BufferDesc.m_MemoryType == MemoryType::kUpload)
		{
			m_BufferDesc.m_MemoryType = MemoryType::kUploadNC;
		}
	}

	bool createMapped = EHasFlag(m_BufferDesc.m_CreateFlags, BufferCreateFlags::kCreateMapped);
	if (m_BufferDesc.m_MemoryType != MemoryType::kDevice && createMapped)
	{
		Map(m_BufferDesc.m_Size, 0);
	}

	if (pInitData)
	{
		if (m_BufferDesc.m_MemoryType == MemoryType::kDevice)
		{
			CopyInitDataToGPU(pInitData);
		}
		else
		{
			BV_ASSERT(m_BufferDesc.m_MemoryType != MemoryType::kReadBack
				&& m_BufferDesc.m_MemoryType != MemoryType::kReadBackNC, "Readback shouldn't have initial data");

			auto size = std::min(m_BufferDesc.m_Size, pInitData->m_Size);
			if (!createMapped)
			{
				Map(size, 0);
			}
			memcpy(m_pMapped, pInitData->m_pData, size);
			Flush(size, 0);
			if (!createMapped)
			{
				Unmap();
			}
		}
	}
}


void BvBufferVk::Destroy()
{
	auto device = m_pDevice->GetHandle();
	if (m_Buffer)
	{
		Unmap();
		vkDestroyBuffer(device, m_Buffer, nullptr);
		m_Buffer = VK_NULL_HANDLE;
		
		vmaFreeMemory(m_pDevice->GetAllocator(), m_VMAAllocation);
	}
}


void BvBufferVk::CopyInitDataToGPU(const BufferInitData* pInitData)
{
	if (!pInitData->m_pContext || !pInitData->m_Size || !pInitData->m_pData)
	{
		return;
	}

	BufferDesc bufferDesc;
	bufferDesc.m_MemoryType = MemoryType::kUpload;
	bufferDesc.m_Size = pInitData->m_Size;
	bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
	BvBufferVk srcBuffer(m_pDevice, bufferDesc, pInitData);
	
	VkBufferCopy copyRegion{ 0, 0, std::min(m_BufferDesc.m_Size, pInitData->m_Size) };
	auto pContext = static_cast<BvCommandContextVk*>(pInitData->m_pContext);
	pContext->NewCommandList();
	pContext->CopyBufferVk(&srcBuffer, this, copyRegion);
	pContext->ExecuteAndWait();
}