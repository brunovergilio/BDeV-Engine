#include "BvTextureVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"
#include "BvCommandContextVk.h"
#include "BvBufferVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"
#include "BvCommandBufferVk.h"


BvTextureVk::BvTextureVk(BvRenderDeviceVk* pDevice, const TextureDesc& textureDesc, const TextureInitData* pInitData)
	: BvTexture(textureDesc), m_pDevice(pDevice)
{
	Create(pInitData);
}


BvTextureVk::BvTextureVk(BvRenderDeviceVk* pDevice, BvSwapChainVk* pSwapChain, const TextureDesc& textureDesc, VkImage image)
	: BvTexture(textureDesc), m_pDevice(pDevice), m_pSwapChain(pSwapChain), m_Image(image)
{
}


BvTextureVk::~BvTextureVk()
{
	Destroy();
}


BvRenderDevice* BvTextureVk::GetDevice()
{
	return m_pDevice;
}


void BvTextureVk::Create(const TextureInitData* pInitData)
{
	if (pInitData && m_TextureDesc.m_MemoryType != MemoryType::kDevice)
	{
		// TODO: Use a VkBuffer instead
		BvAssert(false, "Not supported");
	}

	VkImageCreateFlags imageCreateFlags = 0;
	if (EHasFlag(m_TextureDesc.m_CreateFlags, TextureCreateFlags::kCreateCubemap))
	{
		imageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	u32 mipCount = m_TextureDesc.m_MipLevels;
	if (m_TextureDesc.m_MipLevels == 1 && EHasFlag(m_TextureDesc.m_CreateFlags, TextureCreateFlags::kReserveMips))
	{
		m_TextureDesc.m_MipLevels = GetMipCount(m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height, m_TextureDesc.m_Size.depth);
	}

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = imageCreateFlags;
	imageCreateInfo.imageType = GetVkImageType(m_TextureDesc.m_ImageType);
	imageCreateInfo.format = GetVkFormat(m_TextureDesc.m_Format);
	imageCreateInfo.extent = { m_TextureDesc.m_Size.width, m_TextureDesc.m_Size.height, m_TextureDesc.m_Size.depth };
	imageCreateInfo.mipLevels = m_TextureDesc.m_MipLevels;
	imageCreateInfo.arrayLayers = m_TextureDesc.m_LayerCount;
	imageCreateInfo.samples = GetVkSampleCountFlagBits(m_TextureDesc.m_SampleCount);
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = GetVkImageUsageFlags(m_TextureDesc.m_UsageFlags);
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	auto device = m_pDevice->GetHandle();

	auto result = vkCreateImage(device, &imageCreateInfo, nullptr, &m_Image);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		return;
	}

	auto vma = m_pDevice->GetAllocator();
	VmaAllocationCreateInfo vmaACI = {};
	vmaACI.requiredFlags = GetVkMemoryPropertyFlags(m_TextureDesc.m_MemoryType);
	vmaACI.preferredFlags = GetPreferredVkMemoryPropertyFlags(m_TextureDesc.m_MemoryType);

	VmaAllocationInfo vmaAI;
	VmaAllocation vmaA;
	result = vmaAllocateMemoryForImage(vma, m_Image, &vmaACI, &vmaA, &vmaAI);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		vkDestroyImage(device, m_Image, nullptr);
		return;
	}

	result = vkBindImageMemory(device, m_Image, vmaAI.deviceMemory, vmaAI.offset);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
		vkDestroyImage(device, m_Image, nullptr);
		vmaFreeMemory(vma, m_VMAAllocation);
	}
	m_VMAAllocation = vmaA;

	if (m_TextureDesc.m_ResourceState != ResourceState::kCommon && pInitData)
	{
		BvAssert(pInitData->m_pContext != nullptr, "Invalid command context");
		CopyInitDataAndTransitionState(pInitData, mipCount);
	}
}


void BvTextureVk::Destroy()
{
	if (!m_pSwapChain)
	{
		auto device = m_pDevice->GetHandle();

		if (m_Image)
		{
			vkDestroyImage(device, m_Image, nullptr);
		}
		vmaFreeMemory(m_pDevice->GetAllocator(), m_VMAAllocation);
	}
}


void BvTextureVk::CopyInitDataAndTransitionState(const TextureInitData* pInitData, u32 mipCount)
{
	auto pContext = TO_VK(pInitData->m_pContext);

	ResourceState currState = ResourceState::kCommon;
	bool isValidTextureData = pInitData->m_SubresourceCount == (u32)m_TextureDesc.m_LayerCount * mipCount && pInitData->m_pSubresources != nullptr;
	if (isValidTextureData)
	{
		BvVector<VkBufferImageCopy> copyRegions(pInitData->m_SubresourceCount);
		u32 index = 0;
		u64 bufferSize = 0;
		// Calculate copy regions
		for (auto layer = 0u; layer < m_TextureDesc.m_LayerCount; ++layer)
		{
			for (auto mip = 0u; mip < mipCount; ++mip, ++index)
			{
				auto& copyRegion = copyRegions[index];
				copyRegion.bufferOffset = bufferSize;
				copyRegion.bufferImageHeight = 0;
				copyRegion.bufferRowLength = 0;

				TextureSubresource subresource;
				GetTextureSubresourceData(m_TextureDesc, mip, subresource);

				copyRegion.imageOffset = { 0, 0, 0 };
				copyRegion.imageExtent = { subresource.m_Width, subresource.m_Detph, subresource.m_Height };
				copyRegion.imageSubresource.aspectMask = GetVkFormatMap(m_TextureDesc.m_Format).aspectFlags;
				copyRegion.imageSubresource.mipLevel = mip;
				copyRegion.imageSubresource.baseArrayLayer = layer;
				copyRegion.imageSubresource.layerCount = 1;

				bufferSize += RoundToNearestMultiple(bufferSize, 4u);
			}
		}

		BufferDesc bufferDesc;
		bufferDesc.m_MemoryType = MemoryType::kUpload;
		bufferDesc.m_Size = bufferSize;
		bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;

		BvBufferVk buffer(m_pDevice, bufferDesc, nullptr);
		auto pDstData = static_cast<u8*>(buffer.GetMappedData());

		// Copy initial data to staging buffer
		index = 0;
		for (auto layer = 0u; layer < m_TextureDesc.m_LayerCount; ++layer)
		{
			for (auto mip = 0u; mip < mipCount; ++mip, ++index)
			{
				auto& copyRegion = copyRegions[index];
				auto& srcSubresource = pInitData->m_pSubresources[index];
				
				TextureSubresource dstSubresource;
				GetTextureSubresourceData(m_TextureDesc, mip, dstSubresource);

				for (auto z = 0u; z < dstSubresource.m_Detph; ++z)
				{
					for (auto y = 0u; y < dstSubresource.m_NumRows; ++y)
					{
						auto pSrc = srcSubresource.m_pData + y * srcSubresource.m_RowPitch + srcSubresource.m_SlicePitch * z;
						auto pDst = pDstData + copyRegion.bufferOffset + y * dstSubresource.m_RowPitch + dstSubresource.m_SlicePitch * z;
						memcpy(pDst, pSrc, dstSubresource.m_RowPitch);
					}
				}
			}
		}

		ResourceBarrierDesc copyDstBarrier;
		copyDstBarrier.m_pTexture = this;
		copyDstBarrier.m_DstLayout = ResourceState::kTransferDst;
		copyDstBarrier.m_Subresource.mipCount = mipCount;

		pContext->ResourceBarrier(1, &copyDstBarrier);
		pContext->CopyBufferToTexture(&buffer, this, (u32)copyRegions.Size(), copyRegions.Data());

		currState = ResourceState::kTransferDst;

		if (EHasFlag(m_TextureDesc.m_CreateFlags, TextureCreateFlags::kGenerateMips) && mipCount == 1)
		{
			ResourceBarrierDesc copySrcBarrier;
			copySrcBarrier.m_pTexture = this;
			copySrcBarrier.m_SrcLayout = ResourceState::kTransferDst;
			copySrcBarrier.m_DstLayout = ResourceState::kTransferSrc;
			copySrcBarrier.m_Subresource.firstLayer = 0;
			copySrcBarrier.m_Subresource.layerCount = m_TextureDesc.m_LayerCount;
			copySrcBarrier.m_Subresource.firstMip = 0;
			copySrcBarrier.m_Subresource.mipCount = 1;

			pContext->ResourceBarrier(1, &copySrcBarrier);
			GenerateMips(pContext);
			currState = ResourceState::kTransferSrc;
		}
	}

	if (currState != m_TextureDesc.m_ResourceState)
	{
		ResourceBarrierDesc barrier;
		barrier.m_pTexture = this;
		barrier.m_SrcLayout = currState;
		barrier.m_DstLayout = m_TextureDesc.m_ResourceState;

		pContext->ResourceBarrier(1, &barrier);
	}

	pContext->Signal();
	pContext->WaitForGPU();
}


void BvTextureVk::GenerateMips(BvCommandContextVk* pContext)
{
	auto aspectMask = GetVkFormatMap(m_TextureDesc.m_Format).aspectFlags;
	for (auto i = 1; i < m_TextureDesc.m_MipLevels; ++i)
	{
		VkImageBlit2 imageBlit{ VK_STRUCTURE_TYPE_IMAGE_BLIT_2 };

		// Source
		imageBlit.srcSubresource.aspectMask = aspectMask;
		imageBlit.srcSubresource.layerCount = m_TextureDesc.m_LayerCount;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcOffsets[1].x = i32(std::max(m_TextureDesc.m_Size.width >> (i - 1), 1u));
		imageBlit.srcOffsets[1].y = i32(std::max(m_TextureDesc.m_Size.height >> (i - 1), 1u));
		imageBlit.srcOffsets[1].z = i32(std::max(m_TextureDesc.m_Size.depth >> (i - 1), 1u));

		// Destination
		imageBlit.dstSubresource.aspectMask = aspectMask;
		imageBlit.dstSubresource.layerCount = m_TextureDesc.m_LayerCount;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstOffsets[1].x = i32(std::max(m_TextureDesc.m_Size.width >> i, 1u));
		imageBlit.dstOffsets[1].y = i32(std::max(m_TextureDesc.m_Size.height >> i, 1u));
		imageBlit.dstOffsets[1].z = i32(std::max(m_TextureDesc.m_Size.depth >> i, 1u));

		ResourceBarrierDesc copyDstBarrier;
		copyDstBarrier.m_pTexture = this;
		copyDstBarrier.m_DstLayout = ResourceState::kTransferDst;
		copyDstBarrier.m_Subresource.firstLayer = 0;
		copyDstBarrier.m_Subresource.layerCount = m_TextureDesc.m_LayerCount;
		copyDstBarrier.m_Subresource.firstMip = i;
		copyDstBarrier.m_Subresource.mipCount = 1;

		pContext->ResourceBarrier(1, &copyDstBarrier);

		VkBlitImageInfo2 blitInfo{ VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2 };
		blitInfo.srcImage = m_Image;
		blitInfo.dstImage = m_Image;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &imageBlit;
		blitInfo.filter = VK_FILTER_LINEAR;

		// Blitting isn't a function I'm providing right now, so I do it separately here
		vkCmdBlitImage2(pContext->GetCurrentCommandBuffer()->GetHandle(), &blitInfo);

		ResourceBarrierDesc copySrcBarrier;
		copySrcBarrier.m_pTexture = this;
		copySrcBarrier.m_SrcLayout = ResourceState::kTransferDst;
		copySrcBarrier.m_DstLayout = ResourceState::kTransferSrc;
		copySrcBarrier.m_Subresource.firstLayer = 0;
		copySrcBarrier.m_Subresource.layerCount = m_TextureDesc.m_LayerCount;
		copySrcBarrier.m_Subresource.firstMip = i;
		copySrcBarrier.m_Subresource.mipCount = 1;

		pContext->ResourceBarrier(1, &copySrcBarrier);
	}
}