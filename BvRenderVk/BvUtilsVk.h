#pragma once


#include "BvCommonVk.h"


const char* const VkResultToString(const VkResult result);
bool IsDepthFormat(Format format);
bool IsStencilFormat(Format format);
bool IsDepthStencilFormat(Format format);
bool IsDepthOrStencilFormat(Format format);


class BvRenderDeviceVk;
class BvCommandContextVk;


namespace VkHelpers
{
	template<typename T>
	using VkObj = std::pair<VkResult, T>;

	struct VkSwapChainObj
	{
		VkSwapchainKHR m_SwapChain;
		VkSurfaceKHR m_Surface;
		BvVector<VkImage> m_Images;
		VkExtent2D m_Extents;
	};

	struct VkBufferObj
	{
		VkBuffer m_Buffer;
		VkDeviceAddress m_DeviceAddress;
		VmaAllocation m_Memory;
		void* m_pMappedMemory;
		bool m_NeedsFlush;
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

	struct VkASObj
	{
		VkAccelerationStructureKHR m_AS;
		VkDeviceAddress m_DeviceAddress;
		BvVector<VkAccelerationStructureGeometryKHR> m_Geometries;
		BvVector<u32> m_PrimitiveCounts;
		VkBufferObj m_BufferObj;
		RayTracingAccelerationStructureScratchSize m_ScratchSizes;
	};

	struct VkSBTObj
	{
		VkBufferObj m_BufferObj;
		VkStridedDeviceAddressRegionKHR m_Regions[u32(ShaderBindingTableGroupType::kCount)]{};
	};

	VkObj<VkSurfaceKHR> CreateSurface(VkInstance instance, BvRenderDeviceVk* pDevice, BvWindow* pWindow);
	VkObj<VkSwapChainObj> CreateSwapChain(BvRenderDeviceVk* pDevice, VkInstance instance, u32 queueFamilyIndex, VkSwapchainKHR oldSwapChain,
		VkSurfaceKHR surface, SwapChainDesc& swapChainDesc, BvWindow* pWindow);
	VkObj<VkBufferObj> CreateBuffer(BvRenderDeviceVk* pDevice, const BufferDesc& bufferDesc);
	VkObj<VkBufferView> CreateBufferView(BvRenderDeviceVk* pDevice, const BufferViewDesc& bufferViewDesc);
	VkObj<VkTextureObj> CreateTexture(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, u32 mutableFormatCount = 0, const Format* pMutableFormats = nullptr);
	VkObj<VkImageView> CreateTextureView(BvRenderDeviceVk* pDevice, const TextureViewDesc& textureViewDesc);
	VkObj<VkSampler> CreateSampler(BvRenderDeviceVk* pDevice, const SamplerDesc& samplerDesc);
	VkObj<VkRenderPass> CreateRenderPass(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc);
	VkObj<VkSRLObj> CreateShaderResourceLayout(BvRenderDeviceVk* pDevice, const ShaderResourceLayoutCreateDesc& shaderResourceLayoutDesc);
	VkObj<VkPipelineCache> CreatePipelineCache(BvRenderDeviceVk* pDevice, const PipelineCacheInitData* pInitData);
	VkObj<VkPipeline> CreateGraphicsPipeline(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc, VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	VkObj<VkPipeline> CreateComputePipeline(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc& pipelineStateDesc, VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	VkObj<VkPipeline> CreateRayTracingPipeline(BvRenderDeviceVk* pDevice, const RayTracingPipelineStateDesc& pipelineStateDesc, VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	VkObj<VkSemaphore> CreateSemaphore(BvRenderDeviceVk* pDevice, const GPUFenceDesc& fenceDesc = {}, bool isTimelineSemaphore = true);
	VkObj<VkQueryPool> CreateQueryPool(BvRenderDeviceVk* pDevice, QueryType queryType, u32 queryCount, bool meshPrimitivesPool = false);
	VkObj<VkASObj> CreateAccelerationStructure(BvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& asDesc);
	VkObj<VkSBTObj> CreateShaderBindingTable(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc, BvCommandContextVk* pContext);
	void UploadMemoryToGPU(BvRenderDeviceVk* pDevice, VkBuffer buffer, const BufferInitData& initData);
	void UploadMemoryToGPU(BvRenderDeviceVk* pDevice, VkImage texture, const TextureDesc& textureDesc, const TextureInitData& initData);

	template<typename T> void DestroyInstanceObject(VkInstance instance, T vkResource) = delete;
	template<typename T> void DestroyDeviceObject(VkDevice device, T vkResource) = delete;

#define BV_DESTROY_VK_INSTANCE_OBJ_FUNCTION(resource) \
	template<> BV_INLINE void DestroyInstanceObject(VkInstance instance, Vk##resource vkResource) \
	{ \
		vkDestroy##resource(instance, vkResource, nullptr); \
	}

#define BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(resource) \
	template<> BV_INLINE void DestroyDeviceObject(VkDevice device, Vk##resource vkResource) \
	{ \
		vkDestroy##resource(device, vkResource, nullptr); \
	}

	BV_DESTROY_VK_INSTANCE_OBJ_FUNCTION(SurfaceKHR);
	BV_DESTROY_VK_INSTANCE_OBJ_FUNCTION(DebugUtilsMessengerEXT);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(SwapchainKHR);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(Buffer);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(BufferView);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(Image);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(ImageView);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(Sampler);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(RenderPass);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(Framebuffer);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(DescriptorSetLayout);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(DescriptorPool);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(PipelineLayout);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(Pipeline);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(PipelineCache);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(ShaderModule);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(Semaphore);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(QueryPool);
	BV_DESTROY_VK_DEVICE_OBJ_FUNCTION(AccelerationStructureKHR);

#undef BV_DESTROY_VK_INSTANCE_OBJ_FUNCTION
#undef BV_DESTROY_VK_DEVICE_OBJ_FUNCTION

	BV_INLINE void DestroyDeviceObject(VkDevice device, VkBuffer vkResource, VmaAllocator allocator, VmaAllocation allocation)
	{
		if (allocation)
		{
			vmaDestroyBuffer(allocator, vkResource, allocation);
		}
		else
		{
			DestroyDeviceObject(device, vkResource);
		}
	}

	BV_INLINE void DestroyDeviceObject(VkDevice device, VkImage vkResource, VmaAllocator allocator, VmaAllocation allocation)
	{
		if (allocation)
		{
			vmaDestroyImage(allocator, vkResource, allocation);
		}
		else
		{
			DestroyDeviceObject(device, vkResource);
		}
	}

	template<typename T> constexpr VkObjectType GetObjectType() = delete; //{ return VK_OBJECT_TYPE_UNKNOWN; }
#define BV_VK_OBJ_TYPE(handleType, objType) template<> constexpr VkObjectType GetObjectType<handleType>() { return objType; }
BV_VK_OBJ_TYPE(VkQueue, VK_OBJECT_TYPE_QUEUE);
BV_VK_OBJ_TYPE(VkSemaphore, VK_OBJECT_TYPE_SEMAPHORE);
BV_VK_OBJ_TYPE(VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER);
BV_VK_OBJ_TYPE(VkDeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY);
BV_VK_OBJ_TYPE(VkBuffer, VK_OBJECT_TYPE_BUFFER);
BV_VK_OBJ_TYPE(VkImage, VK_OBJECT_TYPE_IMAGE);
BV_VK_OBJ_TYPE(VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL);
BV_VK_OBJ_TYPE(VkBufferView, VK_OBJECT_TYPE_BUFFER_VIEW);
BV_VK_OBJ_TYPE(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW);
BV_VK_OBJ_TYPE(VkPipelineCache, VK_OBJECT_TYPE_PIPELINE_CACHE);
BV_VK_OBJ_TYPE(VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT);
BV_VK_OBJ_TYPE(VkRenderPass, VK_OBJECT_TYPE_RENDER_PASS);
BV_VK_OBJ_TYPE(VkPipeline, VK_OBJECT_TYPE_PIPELINE);
BV_VK_OBJ_TYPE(VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
BV_VK_OBJ_TYPE(VkSampler, VK_OBJECT_TYPE_SAMPLER);
BV_VK_OBJ_TYPE(VkDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL);
BV_VK_OBJ_TYPE(VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET);
BV_VK_OBJ_TYPE(VkFramebuffer, VK_OBJECT_TYPE_FRAMEBUFFER);
BV_VK_OBJ_TYPE(VkCommandPool, VK_OBJECT_TYPE_COMMAND_POOL);
BV_VK_OBJ_TYPE(VkSurfaceKHR, VK_OBJECT_TYPE_SURFACE_KHR);
BV_VK_OBJ_TYPE(VkSwapchainKHR, VK_OBJECT_TYPE_SWAPCHAIN_KHR);
BV_VK_OBJ_TYPE(VkAccelerationStructureKHR, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR);
#undef BV_VK_OBJ_TYPE

	template<typename T, typename... Args>
	VkResult SetCustomObjectName(VkDevice device, T vkResource, const std::string_view& fmt, Args&&... args)
	{
		auto size = SPrint(nullptr, 0, fmt, args);
		auto pMem = BV_STACK_ALLOC(size + 1);
		auto pName = reinterpret_cast<char*>(pMem);
		SPrint(pName, size, fmt, std::forward<Args>(args)...);

		return SetObjectName(device, vkResource, pName);
	}

	template<typename T>
	VkResult SetObjectName(VkDevice device, T vkResource, const char* pName)
	{
		if (!vkSetDebugUtilsObjectNameEXT)
		{
			return;
		}

		VkDebugUtilsObjectNameInfoEXT nameInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr, GetObjectType<T>(), (u64)vkResource, pName};
		return vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
	}
}