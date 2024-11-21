#include "BvRenderAPIUtils.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


void GetTextureSubresourceData(const TextureDesc& textureDesc, u32 mipLevel, TextureSubresource& subresource)
{
	subresource.m_Width = std::max(1u, textureDesc.m_Size.width >> mipLevel);
	subresource.m_Height = std::max(1u, textureDesc.m_Size.height >> mipLevel);
	subresource.m_Detph = std::max(1u, textureDesc.m_Size.depth >> mipLevel);
	
	auto formatInfo = GetFormatInfo(textureDesc.m_Format);
	if (formatInfo.m_IsCompressed)
	{
		u32 texelBlockWidth = std::max(1u, RoundToNearestPowerOf2(subresource.m_Width, formatInfo.m_BlockWidth));
		u32 texelBlockHeight = std::max(1u, RoundToNearestPowerOf2(subresource.m_Height, formatInfo.m_BlockHeight));
		subresource.m_NumRows = texelBlockHeight / (u32)formatInfo.m_BlockHeight;
		subresource.m_RowPitch = (u64)formatInfo.m_ElementSize * (texelBlockWidth / (u32)formatInfo.m_BlockWidth);
	}
	else
	{
		subresource.m_NumRows = subresource.m_Height;
		subresource.m_RowPitch = (u64)formatInfo.m_ElementCount * formatInfo.m_ElementSize * subresource.m_Width;
	}
	subresource.m_SlicePitch = subresource.m_RowPitch * subresource.m_NumRows;
	subresource.m_MipSize = subresource.m_SlicePitch * subresource.m_Detph;
}


u64 GetTextureDataOffset(const TextureDesc& textureDesc, u32 mipLevel, u32 arraySlice, u32 alignment)
{
	BV_ASSERT((arraySlice < textureDesc.m_LayerCount && mipLevel < textureDesc.m_MipLevels)
		|| (arraySlice == textureDesc.m_LayerCount && mipLevel == 0), "Invalid arguments");

	u64 offset = 0;
	if (arraySlice > 0)
	{
		u64 arraySliceSize = 0;
		for (auto mip = 0u; mip < textureDesc.m_MipLevels; ++mip)
		{
			TextureSubresource subresource;
			GetTextureSubresourceData(textureDesc, mip, subresource);
			arraySliceSize += RoundToNearestPowerOf2(subresource.m_SlicePitch, alignment);
		}

		offset = arraySliceSize;
		if (textureDesc.m_LayerCount > 0)
		{
			offset *= arraySlice;
		}
	}

	for (auto mip = 0u; mip < mipLevel; ++mip)
	{
		TextureSubresource subresource;
		GetTextureSubresourceData(textureDesc, mip, subresource);
		offset += RoundToNearestPowerOf2(subresource.m_SlicePitch, alignment);
	}

	return offset;
}


u64 GetBufferSizeForTextureData(const TextureDesc& textureDesc, u32 alignment)
{
	return GetTextureDataOffset(textureDesc, 0, textureDesc.m_LayerCount, alignment);
}


void CopySubresource(const void* pSrcData, void* pDstData, const SubresourceFootprint& srcSubresource, const SubresourceFootprint& dstSubresource)
{
	for (auto z = 0u; z < dstSubresource.m_Subresource.m_Detph; ++z)
	{
		for (auto y = 0u; y < dstSubresource.m_Subresource.m_NumRows; ++y)
		{
			auto pSrc = static_cast<const char*>(pSrcData) + y * srcSubresource.m_Subresource.m_RowPitch + srcSubresource.m_Subresource.m_SlicePitch * z;
			auto pDst = static_cast<char*>(pDstData) + dstSubresource.m_Offset + y * dstSubresource.m_Subresource.m_RowPitch + dstSubresource.m_Subresource.m_SlicePitch * z;
			memcpy(pDst, pSrc, dstSubresource.m_Subresource.m_RowPitch);
		}
	}
}


u32 GetMipCount(u32 width, u32 height, u32 depth)
{
	return static_cast<u32>(std::floorf(std::log2f(std::max(std::max(width, height), depth))) + 1);
}