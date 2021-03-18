#pragma once


#include "BvCommonVk.h"
#include "BvRender/BvTexture.h"
#include "BvRender/BvTextureView.h"
#include "BvRender/BvBuffer.h"
#include "BvRender/BvSampler.h"
#include "BvRender/BvPipelineState.h"


VkImageType GetVkImageType(const TextureType type);
VkImageViewType GetVkImageViewType(const TextureViewType viewType);
VkFormat GetVkFormat(const Format format);
Format GetFormat(const VkFormat format);
VkBufferUsageFlags GetVkBufferUsageFlags(const BufferUsage usageFlags);
VkImageUsageFlags GetVkImageUsageFlags(const TextureUsage usageFlags);
VkMemoryPropertyFlags GetVkMemoryPropertyFlags(const MemoryFlags memoryFlags);
VkFilter GetVkFilter(const Filter filter);
VkSamplerMipmapMode GetVkSamplerMipmapMode(const MipMapFilter mipMapFilter);
VkSamplerAddressMode GetVkSamplerAddressMode(const AddressMode addressMode);
VkCompareOp GetVkCompareOp(const CompareOp compareOp);
VkCommandBufferLevel GetVkCommandBufferLevel(const CommandType commandLevel);
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
VkImageLayout GetVkImageLayout(const ResourceState resourceState);
VkAccessFlags GetVkAccessFlags(const ResourceAccess resourceAccess);
VkAccessFlags GetVkAccessFlags(const ResourceState resourceState);
VkPipelineStageFlags GetVkPipelineStageFlags(const VkAccessFlags accessFlags);
VkPipelineStageFlags GetVkPipelineStageFlags(const PipelineStage pipelineStage);