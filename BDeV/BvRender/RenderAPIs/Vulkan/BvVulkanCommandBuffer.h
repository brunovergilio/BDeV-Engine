#pragma once


#include "BvCore/Container/BvVector.h"
#include "BvRender/BvRenderDefines.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanRenderTarget.h"


class BvVulkanDevice;
class BvVulkanCommandBuffer;


class BvVulkanCommandPool
{
public:
	BvVulkanCommandPool(const BvVulkanDevice * const pDevice, const uint32_t queueFamilyIndex, const VkFlags flags = 0);
	~BvVulkanCommandPool();

	BvVulkanCommandBuffer & RequestCommandBuffer(const VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void Reset();

	BV_INLINE const VkCommandPool GetHandle() const { return m_CommandPool; }
	BV_INLINE const VkFlags GetFlags() const { return m_Flags; }

private:
	const BvVulkanDevice * m_pDevice = nullptr;
	VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	BvVector<BvVulkanCommandBuffer> m_CommandBuffers;
	uint32_t m_QueueFamilyIndex = 0;
	VkFlags m_Flags = 0;
};


class BvVulkanCommandBuffer
{
public:
	BvVulkanCommandBuffer(BvVulkanCommandPool * const pCommandPool, VkCommandBuffer commandBuffer,
		const VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	~BvVulkanCommandBuffer();

	void Reset();

	VkResult BeginCommandBuffer(const VkCommandBufferUsageFlags flags = 0, const VkCommandBufferInheritanceInfo * const pInheritanceInfo = nullptr) const;
	VkResult EndCommandBuffer() const;

	void BeginRenderPass(const BvVulkanRenderTarget & renderTarget, const uint32_t frameBufferIndex,
		const uint32_t clearValueCount, const VkClearValue * const pClearValues,
		const VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
	void EndRenderPass() const;


	void SetViewport(const float width, const float height, const float minDepth = 0.0f, const float maxDepth = 1.0f) const;
	void SetViewport(const uint32_t viewportCount, const VkViewport * const pViewports) const;
	void SetScissor(const uint32_t width, const uint32_t height, const int32_t x = 0, const int32_t y = 0) const;
	void SetScissor(const uint32_t scissorCount, const VkRect2D * const pScissors) const;

	void BindPipeline(const VkPipeline pipeline, const VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

	void BindDescriptorSets(const VkPipelineBindPoint pipelineBindPoint, const VkPipelineLayout layout, const uint32_t firstSet, const uint32_t descriptorSetCount,
		const VkDescriptorSet* pDescriptorSets, const uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr) const;

	void BindVertexBuffers(const uint32_t firstBinding, const uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const;
	void BindIndexBuffer(const VkBuffer buffer, const VkDeviceSize offset = 0, const VkIndexType indexType = VK_INDEX_TYPE_UINT32) const;

	void Draw(const uint32_t vertexCount, const uint32_t instanceCount = 1,
		const uint32_t firstVertex = 0, const uint32_t firstInstance = 0) const;
	void DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount = 1, const uint32_t firstIndex = 0,
		const int32_t vertexOffset = 0, const uint32_t firstInstance = 0) const;

	void CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size) const;
	void CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const uint32_t regionCount, const VkBufferCopy * const pRegions = nullptr) const;

	void CopyBufferToImage(const VkBuffer srcBuffer, const VkImage dstImage, const VkImageLayout dstImageLayout,
		const uint32_t regionCount, const VkBufferImageCopy* pRegions) const;

	void PipelineBarrier(const VkPipelineStageFlags srcStageMask,
		const VkPipelineStageFlags dstStageMask,
		const VkDependencyFlags dependencyFlags,
		const uint32_t memoryBarrierCount,
		const VkMemoryBarrier * pMemoryBarriers,
		const uint32_t bufferMemoryBarrierCount,
		const VkBufferMemoryBarrier * pBufferMemoryBarriers,
		const uint32_t imageMemoryBarrierCount,
		const VkImageMemoryBarrier * pImageMemoryBarriers) const;

	BV_INLINE const VkCommandBuffer GetHandle() const { return m_CommandBuffer; }
	BV_INLINE const VkCommandBufferLevel GetLevel() const { return m_Level; }

private:
	BvVulkanCommandPool * m_pCommandPool = nullptr;
	VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	VkCommandBufferLevel m_Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
};