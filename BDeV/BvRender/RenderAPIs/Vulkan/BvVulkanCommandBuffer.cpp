#include "BvVulkanCommandBuffer.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanFramebuffer.h"


constexpr uint32_t g_DefaultCommandBufferCount = 16;


BvVulkanCommandPool::BvVulkanCommandPool(const BvVulkanDevice * const pDevice, const uint32_t queueFamilyIndex, const VkFlags flags)
	: m_pDevice(pDevice), m_QueueFamilyIndex(queueFamilyIndex), m_Flags(flags)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = m_QueueFamilyIndex;
	createInfo.flags = m_Flags;

	vkCreateCommandPool(m_pDevice->GetLogical(), &createInfo, nullptr, &m_CommandPool);
}


BvVulkanCommandPool::~BvVulkanCommandPool()
{
	if (m_CommandPool)
	{
		vkDestroyCommandPool(m_pDevice->GetLogical(), m_CommandPool, nullptr);
	}
}


BvVulkanCommandBuffer & BvVulkanCommandPool::RequestCommandBuffer(const VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = m_CommandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = level;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	vkAllocateCommandBuffers(m_pDevice->GetLogical(), &allocateInfo, &commandBuffer);

	//m_CommandBuffers.EmplaceBack(BvVulkanCommandBuffer(this, commandBuffer, level));

	return m_CommandBuffers.Back();
}


void BvVulkanCommandPool::Reset()
{
	vkResetCommandPool(m_pDevice->GetLogical(), m_CommandPool, 0);
}


BvVulkanCommandBuffer::BvVulkanCommandBuffer(BvVulkanCommandPool * const pCommandPool, VkCommandBuffer commandBuffer,
	const VkCommandBufferLevel level)
	: m_CommandBuffer(commandBuffer), m_pCommandPool(pCommandPool), m_Level(level)
{
}


BvVulkanCommandBuffer::~BvVulkanCommandBuffer()
{
}


void BvVulkanCommandBuffer::Reset()
{
	vkResetCommandBuffer(m_CommandBuffer, 0);
}


VkResult BvVulkanCommandBuffer::BeginCommandBuffer(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo * const pInheritanceInfo) const
{
	assert(m_CommandBuffer != nullptr);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags;
	beginInfo.pInheritanceInfo = pInheritanceInfo;

	return vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
}


VkResult BvVulkanCommandBuffer::EndCommandBuffer() const
{
	assert(m_CommandBuffer != nullptr);

	return vkEndCommandBuffer(m_CommandBuffer);
}


void BvVulkanCommandBuffer::BeginRenderPass(const BvVulkanFramebuffer & framebuffer,
	const uint32_t clearValueCount, const VkClearValue * const pClearValues, const VkSubpassContents subpassContents) const
{
	assert(m_CommandBuffer != nullptr);

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = framebuffer.GetRenderPass();
	beginInfo.framebuffer = framebuffer.Get();
	beginInfo.renderArea.extent.width = framebuffer.GetDesc().m_Width;
	beginInfo.renderArea.extent.height = framebuffer.GetDesc().m_Height;
	beginInfo.clearValueCount = clearValueCount;
	beginInfo.pClearValues = pClearValues;

	vkCmdBeginRenderPass(m_CommandBuffer, &beginInfo, subpassContents);
}


void BvVulkanCommandBuffer::EndRenderPass() const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdEndRenderPass(m_CommandBuffer);
}


void BvVulkanCommandBuffer::SetViewport(const float width, const float height, const float minDepth, const float maxDepth) const
{
	assert(m_CommandBuffer != nullptr);

	VkViewport viewport{ 0.0f, 0.0f, width, height, minDepth, maxDepth };
	vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
}

void BvVulkanCommandBuffer::SetViewport(const uint32_t viewportCount, const VkViewport * const pViewports) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdSetViewport(m_CommandBuffer, 0, viewportCount, pViewports);
}

void BvVulkanCommandBuffer::SetScissor(const uint32_t width, const uint32_t height, const int32_t x, const int32_t y) const
{
	assert(m_CommandBuffer != nullptr);

	VkRect2D scissor{ x, y, width, height };

	vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
}


void BvVulkanCommandBuffer::SetScissor(const uint32_t scissorCount, const VkRect2D * const pScissors) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdSetScissor(m_CommandBuffer, 0, scissorCount, pScissors);
}


void BvVulkanCommandBuffer::BindPipeline(const VkPipeline pipeline, const VkPipelineBindPoint pipelineBindPoint) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdBindPipeline(m_CommandBuffer, pipelineBindPoint, pipeline);
}


void BvVulkanCommandBuffer::BindDescriptorSets(const VkPipelineBindPoint pipelineBindPoint, const VkPipelineLayout layout, const uint32_t firstSet,
	const uint32_t descriptorSetCount, const VkDescriptorSet * pDescriptorSets, const uint32_t dynamicOffsetCount, const uint32_t * pDynamicOffsets) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdBindDescriptorSets(m_CommandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}


void BvVulkanCommandBuffer::BindVertexBuffers(const uint32_t firstBinding, const uint32_t bindingCount, const VkBuffer * pBuffers, const VkDeviceSize * pOffsets) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdBindVertexBuffers(m_CommandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}


void BvVulkanCommandBuffer::BindIndexBuffer(const VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdBindIndexBuffer(m_CommandBuffer, buffer, offset, indexType);
}


void BvVulkanCommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void BvVulkanCommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void BvVulkanCommandBuffer::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size) const
{
	assert(m_CommandBuffer != nullptr);

	VkBufferCopy region{ 0, 0, size };
	vkCmdCopyBuffer(m_CommandBuffer, srcBuffer, dstBuffer, 1, &region);
}

void BvVulkanCommandBuffer::CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const uint32_t regionCount, const VkBufferCopy * const pRegions) const
{
	assert(m_CommandBuffer != nullptr);

	vkCmdCopyBuffer(m_CommandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void BvVulkanCommandBuffer::CopyBufferToImage(const VkBuffer srcBuffer, const VkImage dstImage, const VkImageLayout dstImageLayout, const uint32_t regionCount, const VkBufferImageCopy * pRegions) const
{
	vkCmdCopyBufferToImage(m_CommandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void BvVulkanCommandBuffer::PipelineBarrier(const VkPipelineStageFlags srcStageMask, const VkPipelineStageFlags dstStageMask,
	const VkDependencyFlags dependencyFlags,
	const uint32_t memoryBarrierCount, const VkMemoryBarrier * pMemoryBarriers,
	const uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier * pBufferMemoryBarriers,
	const uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier * pImageMemoryBarriers) const
{
	vkCmdPipelineBarrier(m_CommandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
		bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}