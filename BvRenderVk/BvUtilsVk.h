#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


const char* const VkResultToString(const VkResult result);
bool IsDepthFormat(Format format);
bool IsStencilFormat(Format format);
bool IsDepthStencilFormat(Format format);
bool IsDepthOrStencilFormat(Format format);
bool QueueSupportsPresent(VkPhysicalDevice physicalDevice, u32 index);


template<typename T>
using VkObj = std::pair<VkResult, T>;

class BvWindow;
class BvRenderDeviceVk;
class BvCommandContextVk;

namespace VkHelpers
{
	struct VkBufferObj
	{
		VkBuffer m_Buffer;
		VmaAllocation m_Memory;
	};

	struct VkTextureObj
	{
		VkImage m_Texture;
		VmaAllocation m_Memory;
	};

	struct VkSRLObj
	{
		BvVector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
		VkPipelineLayout m_PipelineLayout;
	};

	VkObj<VkSurfaceKHR> CreateSurface(VkInstance instance, BvRenderDeviceVk* pDevice, BvWindow* pWindow);
	VkObj<VkSwapchainKHR> CreateSwapChain(BvRenderDeviceVk* pDevice, u32 queueFamilyIndex, VkSwapchainKHR oldSwapChain,
		VkSurfaceKHR surface, SwapChainDesc& swapChainDesc, BvWindow* pWindow);
	VkObj<VkBufferObj> CreateBuffer(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc);
	VkObj<VkBufferView> CreateBufferView(BvRenderDeviceVk* pDevice, VkBuffer buffer, const BufferViewDesc& bufferViewDesc);
	VkObj<VkTextureObj> CreateTexture(BvRenderDeviceVk* pDevice, TextureDesc& textureDesc);
	VkObj<VkImageView> CreateTextureView(BvRenderDeviceVk* pDevice, VkImage texture, TextureViewDesc& textureViewDesc);
	VkObj<VkSampler> CreateSampler(BvRenderDeviceVk* pDevice, const SamplerDesc& samplerDesc);
	VkObj<VkRenderPass> CreateRenderPass(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc);
	VkObj<VkSRLObj> CreateShaderResourceLayout(BvRenderDeviceVk* pDevice, const ShaderResourceLayoutDesc& shaderResourceLayoutDesc);
	VkObj<VkPipeline> CreateGraphicsPipeline(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc, VkPipelineCache pipelineCache = VK_NULL_HANDLE);
}