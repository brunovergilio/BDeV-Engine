#include "BvCommandPoolVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandBufferVk.h"


BvCommandPoolVk::BvCommandPoolVk(BvRenderDeviceVk* pDevice, u32 queueFamilyIndex)
	: m_pDevice(pDevice), m_QueueFamilyIndex(queueFamilyIndex)
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

	auto result = vkCreateCommandPool(m_pDevice->GetHandle(), &commandPoolCI, nullptr, &m_CommandPool);
}


void BvCommandPoolVk::Destroy()
{
	if (m_CommandPool)
	{
		for (auto pCB : m_CommandBuffers)
		{
			BV_DELETE(pCB);
		}

		vkDestroyCommandPool(m_pDevice->GetHandle(), m_CommandPool, nullptr);
		m_CommandPool = VK_NULL_HANDLE;
	}
}


BvCommandBufferVk* BvCommandPoolVk::GetCommandBuffer(BvFrameDataVk* pFrameData)
{
	if (m_ActiveCommandBufferCount < m_CommandBuffers.Size())
	{
		auto pCommandBuffer = m_CommandBuffers[m_ActiveCommandBufferCount++];
		pCommandBuffer->Reset();
		pCommandBuffer->Begin();
		return pCommandBuffer;
	}

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.commandPool = m_CommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer commandBuffer;
	auto result = vkAllocateCommandBuffers(m_pDevice->GetHandle(), &allocateInfo, &commandBuffer);
	if (result == VK_SUCCESS)
	{
		auto pCommandBuffer = m_CommandBuffers.EmplaceBack(BV_NEW(BvCommandBufferVk)(m_pDevice, commandBuffer, pFrameData));
		pCommandBuffer->Reset();
		pCommandBuffer->Begin();
		++m_ActiveCommandBufferCount;

		return pCommandBuffer;
	}

	return nullptr;
}


void BvCommandPoolVk::Reset()
{
	vkResetCommandPool(m_pDevice->GetHandle(), m_CommandPool, 0);
	m_ActiveCommandBufferCount = 0;
}