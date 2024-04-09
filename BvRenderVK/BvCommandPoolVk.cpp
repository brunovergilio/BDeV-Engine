#include "BvCommandPoolVk.h"
#include "BvRenderVK/BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandBufferVk.h"


BvCommandPoolVk::BvCommandPoolVk(const BvRenderDeviceVk& device, u32 queueFamilyIndex)
	: m_Device(device), m_QueueFamilyIndex(queueFamilyIndex)
{
	Create();
}


BvCommandPoolVk::~BvCommandPoolVk()
{
	Destroy();
}


void BvCommandPoolVk::Create()
{
	VkCommandPoolCreateInfo commandPoolCI{};
	commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//commandPoolCI.pNext = nullptr;
	commandPoolCI.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCI.queueFamilyIndex = m_QueueFamilyIndex;

	auto result = vkCreateCommandPool(m_Device.GetHandle(), &commandPoolCI, nullptr, &m_CommandPool);
}


void BvCommandPoolVk::Destroy()
{
	for (auto&& pCommandBuffer : m_CommandBuffers)
	{
		delete pCommandBuffer;
	}
	m_CommandBuffers.Clear();

	if (m_CommandPool)
	{
		vkDestroyCommandPool(m_Device.GetHandle(), m_CommandPool, nullptr);
		m_CommandPool = VK_NULL_HANDLE;
	}
}


BvCommandBufferVk* BvCommandPoolVk::GetCommandBuffer()
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = m_CommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (m_ActiveCommandBufferCount < m_CommandBuffers.Size())
	{
		return m_CommandBuffers[m_ActiveCommandBufferCount++];
	}

	VkCommandBuffer commandBuffer;
	auto result = vkAllocateCommandBuffers(m_Device.GetHandle(), &allocateInfo, &commandBuffer);
	if (result == VK_SUCCESS)
	{
		auto pCommandBuffer = new BvCommandBufferVk(m_Device, this, commandBuffer);
		m_CommandBuffers.EmplaceBack(pCommandBuffer);
		++m_ActiveCommandBufferCount;

		return pCommandBuffer;
	}

	return nullptr;
}


void BvCommandPoolVk::Reset()
{
	vkResetCommandPool(m_Device.GetHandle(), m_CommandPool, 0);
}