#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvTexture.h"
#include "BDeV/Core/RenderAPI/BvTextureView.h"
#include "BDeV/Core/RenderAPI/BvBuffer.h"
#include "BDeV/Core/RenderAPI/BvSampler.h"
#include "BDeV/Core/RenderAPI/BvPipelineState.h"


struct FormatMapVk
{
	VkFormat format;
	VkImageAspectFlags aspectFlags;
	VkComponentMapping componentMapping;
};


VkImageType GetVkImageType(const TextureType type);
VkImageViewType GetVkImageViewType(const TextureViewType viewType);
const FormatMapVk& GetVkFormatMap(Format format);
VkImageAspectFlags GetVkImageAspectFlags(Format format, u32 planeSlice = 0);
VkFormat GetVkFormat(const Format format);
Format GetFormat(const VkFormat format);
VkBufferUsageFlags GetVkBufferUsageFlags(const BufferUsage usageFlags);
VkImageUsageFlags GetVkImageUsageFlags(const TextureUsage usageFlags);
VkMemoryPropertyFlags GetVkMemoryPropertyFlags(const MemoryType memoryFlags);
VkMemoryPropertyFlags GetPreferredVkMemoryPropertyFlags(const MemoryType memoryFlags);
VkFilter GetVkFilter(const Filter filter);
VkSamplerMipmapMode GetVkSamplerMipmapMode(const MipMapFilter mipMapFilter);
VkSamplerAddressMode GetVkSamplerAddressMode(const AddressMode addressMode);
VkCompareOp GetVkCompareOp(const CompareOp compareOp);
VkVertexInputRate GetVkVertexInputRate(const InputRate inputRate);
VkPrimitiveTopology GetVkPrimitiveTopology(const Topology topology);
VkPolygonMode GetVkPolygonMode(const FillMode fillMode);
VkCullModeFlags GetVkCullModeFlags(const CullMode cullMode);
VkFrontFace GetVkFrontFace(const FrontFace frontFace);
VkStencilOp GetVkStencilOp(const StencilOp stencilOp);
VkSampleCountFlagBits GetVkSampleCountFlagBits(const u8 sampleCount);
VkBlendOp GetVkBlendOp(const BlendOp blendOp);
VkBlendFactor GetVkBlendFactor(const BlendFactor blendFactor);
VkLogicOp GetVkLogicOp(const LogicOp logicOp);
VkShaderStageFlagBits GetVkShaderStageFlagBits(const ShaderStage shaderStage);
VkDescriptorType GetVkDescriptorType(const ShaderResourceType resourceType);
VkShaderStageFlags GetVkShaderStageFlags(const ShaderStage stages);
VkAttachmentLoadOp GetVkAttachmentLoadOp(const LoadOp loadOp);
VkAttachmentStoreOp GetVkAttachmentStoreOp(const StoreOp storeOp);
VkIndexType GetVkIndexType(const IndexFormat indexFormat);
VkImageLayout GetVkImageLayout(const ResourceState resourceState, bool isDepthStencilFormat = false);
VkAccessFlags2 GetVkAccessFlags(const ResourceAccess resourceAccess);
VkAccessFlags2 GetVkAccessFlags(const ResourceState resourceState);
VkAccessFlags2 GetVkAccessFlags(BufferUsage usageFlags);
VkPipelineStageFlags2 GetVkPipelineStageFlags(const VkAccessFlags2 accessFlags);
VkPipelineStageFlags2 GetVkPipelineStageFlags(const PipelineStage pipelineStage);
VkQueryType GetVkQueryType(QueryType queryHeapType);
VkResolveModeFlagBits GetVkResolveMode(ResolveMode resolveMode);
VkFragmentShadingRateCombinerOpKHR GetVkShadingRateCombinerOp(ShadingRateCombinerOp op);