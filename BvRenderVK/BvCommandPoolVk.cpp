#include "BvCommandPoolVk.h"
#include "BvRenderVK/BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandBufferVk.h"


constexpr u32 kMmaxCommandBuffersPerAllocation = 16;


BvCommandPoolVk::BvCommandPoolVk(const BvRenderDeviceVk & device, const CommandPoolDesc & commandPoolDesc)
	: BvCommandPool(commandPoolDesc), m_Device(device)
{
}


BvCommandPoolVk::~BvCommandPoolVk()
{
	Destroy();
}


void BvCommandPoolVk::Create()
{
	u32 queueFamilyIndex = 0;
	switch (m_CommandPoolDesc.m_QueueFamilyType)
	{
	case QueueFamilyType::kGraphics:
		queueFamilyIndex = m_Device.GetGPUInfo().m_GraphicsQueueIndex;
		break;
	case QueueFamilyType::kCompute:
		queueFamilyIndex = m_Device.GetGPUInfo().m_ComputeQueueIndex;
		break;
	case QueueFamilyType::kTransfer:
		queueFamilyIndex = m_Device.GetGPUInfo().m_TransferQueueIndex;
		break;
	}

	VkCommandPoolCreateInfo commandPoolCI{};
	commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//commandPoolCI.pNext = nullptr;
	//commandPoolCI.flags = 0;
	commandPoolCI.queueFamilyIndex = queueFamilyIndex;

	auto result = m_Device.GetDeviceFunctions().vkCreateCommandPool(m_Device.GetHandle(), &commandPoolCI, nullptr, &m_CommandPool);
}


void BvCommandPoolVk::Destroy()
{
	for (auto&& pCommandBuffer : m_CommandBuffers)
	{
		BvDelete(pCommandBuffer);
	}

	if (m_CommandPool)
	{
		m_Device.GetDeviceFunctions().vkDestroyCommandPool(m_Device.GetHandle(), m_CommandPool, nullptr);
		m_CommandPool = VK_NULL_HANDLE;
	}
}


void BvCommandPoolVk::AllocateCommandBuffers(const u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers)
{
	if (commandBufferCount > kMmaxCommandBuffersPerAllocation)
	{
		AllocateCommandBuffers(commandBufferCount - kMmaxCommandBuffersPerAllocation, ppCommandBuffers + kMmaxCommandBuffersPerAllocation);
	}
	VkCommandBuffer commandBuffers[kMmaxCommandBuffersPerAllocation];

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = commandBufferCount > kMmaxCommandBuffersPerAllocation ? kMmaxCommandBuffersPerAllocation : commandBufferCount;
	allocateInfo.commandPool = m_CommandPool;
	allocateInfo.level = GetVkCommandBufferLevel(m_CommandPoolDesc.m_CommandType);

	auto result = m_Device.GetDeviceFunctions().vkAllocateCommandBuffers(m_Device.GetHandle(), &allocateInfo, commandBuffers);
	for (u32 i = 0; i < allocateInfo.commandBufferCount; i++)
	{
		auto pCommandBuffer = new BvCommandBufferVk(m_Device, this, commandBuffers[i]);
		m_CommandBuffers.EmplaceBack(pCommandBuffer);
		ppCommandBuffers[i] = pCommandBuffer;
	}
}


void BvCommandPoolVk::FreeCommandBuffers(const u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers)
{
	if (commandBufferCount > kMmaxCommandBuffersPerAllocation)
	{
		FreeCommandBuffers(commandBufferCount - kMmaxCommandBuffersPerAllocation, ppCommandBuffers + kMmaxCommandBuffersPerAllocation);
	}

	VkCommandBuffer commandBuffers[kMmaxCommandBuffersPerAllocation];
	u32 count = commandBufferCount > kMmaxCommandBuffersPerAllocation ? kMmaxCommandBuffersPerAllocation : commandBufferCount;
	for (auto i = 0u; i < count; i++)
	{
		auto pCbVk = reinterpret_cast<BvCommandBufferVk *>(ppCommandBuffers[i]);
		commandBuffers[i] = pCbVk->GetHandle();

		for (auto j = 0u; j < m_CommandBuffers.Size(); j++)
		{
			if (m_CommandBuffers[j] == pCbVk)
			{
				if (j != m_CommandBuffers.Size() - 1)
				{
					std::swap(m_CommandBuffers[j], m_CommandBuffers[m_CommandBuffers.Size() - 1]);
				}
				m_CommandBuffers.PopBack();
				BvDelete(pCbVk);
				break;
			}
		}
	}

	m_Device.GetDeviceFunctions().vkFreeCommandBuffers(m_Device.GetHandle(), m_CommandPool, count, commandBuffers);
}


void BvCommandPoolVk::Reset()
{
	m_Device.GetDeviceFunctions().vkResetCommandPool(m_Device.GetHandle(), m_CommandPool, 0);
}