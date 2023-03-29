#include "BvCommandPoolVk.h"
#include "BvRenderVK/BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandBufferVk.h"


constexpr u32 kMmaxCommandBuffersPerAllocation = 16;


BvCommandPoolVk::BvCommandPoolVk(const BvRenderDeviceVk & device, const CommandPoolDesc & commandPoolDesc)
	: BvCommandPool(commandPoolDesc), m_Device(device)
{
	Create();
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
	commandPoolCI.flags = GetVkCommandPoolCreateFlags(m_CommandPoolDesc.m_Flags);
	commandPoolCI.queueFamilyIndex = queueFamilyIndex;

	auto result = vkCreateCommandPool(m_Device.GetHandle(), &commandPoolCI, nullptr, &m_CommandPool);
}


void BvCommandPoolVk::Destroy()
{
	for (auto&& pCommandBuffer : m_UsedCommandBuffers)
	{
		delete pCommandBuffer;
	}
	for (auto&& pCommandBuffer : m_FreeCommandBuffers)
	{
		delete pCommandBuffer;
	}

	if (m_CommandPool)
	{
		vkDestroyCommandPool(m_Device.GetHandle(), m_CommandPool, nullptr);
		m_CommandPool = VK_NULL_HANDLE;
	}
}


void BvCommandPoolVk::AllocateCommandBuffers(u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers)
{
	// Check for free, recycled command buffers
	if ((m_CommandPoolDesc.m_Flags & CommandPoolFlags::kRecycleCommandBuffers) == CommandPoolFlags::kRecycleCommandBuffers)
	{
		u32 i = 0;
		auto freeCBCount = m_FreeCommandBuffers.Size();
		for (; i < commandBufferCount && i < freeCBCount; i++)
		{
			ppCommandBuffers[i] = m_FreeCommandBuffers.Back();
			m_FreeCommandBuffers.PopBack();
		}

		if (i > 0)
		{
			commandBufferCount -= i;
			ppCommandBuffers += i;

			if (commandBufferCount == 0)
			{
				return;
			}
		}
	}

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

	auto result = vkAllocateCommandBuffers(m_Device.GetHandle(), &allocateInfo, commandBuffers);
	for (u32 j = 0; j < allocateInfo.commandBufferCount; j++)
	{
		auto pCommandBuffer = new BvCommandBufferVk(m_Device, this, commandBuffers[j]);
		m_UsedCommandBuffers.EmplaceBack(pCommandBuffer);
		ppCommandBuffers[j] = pCommandBuffer;
	}
}


void BvCommandPoolVk::FreeCommandBuffers(const u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers)
{
	if ((m_CommandPoolDesc.m_Flags & CommandPoolFlags::kRecycleCommandBuffers) == CommandPoolFlags::kRecycleCommandBuffers)
	{
		for (auto i = 0u; i < commandBufferCount; i++)
		{
			auto pCbVk = reinterpret_cast<BvCommandBufferVk*>(ppCommandBuffers[i]);
			for (auto j = 0u; j < m_UsedCommandBuffers.Size(); j++)
			{
				if (m_UsedCommandBuffers[j] == pCbVk)
				{
					if (j != m_UsedCommandBuffers.Size() - 1)
					{
						std::swap(m_UsedCommandBuffers[j], m_UsedCommandBuffers[m_UsedCommandBuffers.Size() - 1]);
					}
					m_UsedCommandBuffers.PopBack();
					m_FreeCommandBuffers.EmplaceBack(pCbVk);
					break;
				}
			}
		}
	}
	else
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

			for (auto j = 0u; j < m_UsedCommandBuffers.Size(); j++)
			{
				if (m_UsedCommandBuffers[j] == pCbVk)
				{
					if (j != m_UsedCommandBuffers.Size() - 1)
					{
						std::swap(m_UsedCommandBuffers[j], m_UsedCommandBuffers[m_UsedCommandBuffers.Size() - 1]);
					}
					m_UsedCommandBuffers.PopBack();
					delete pCbVk;
					break;
				}
			}
		}

		vkFreeCommandBuffers(m_Device.GetHandle(), m_CommandPool, count, commandBuffers);
	}
}


void BvCommandPoolVk::Reset()
{
	vkResetCommandPool(m_Device.GetHandle(), m_CommandPool, 0);
}