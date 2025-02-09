#include "BvRenderAPIUtils.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


void GetTextureSubresourceInfo(TextureSubresourceInfo& subresource, Format format, const Extent3D& size, u32 mipLevel)
{
	auto width = std::max(1u, size.width >> mipLevel);
	auto height = std::max(1u, size.height >> mipLevel);
	auto depth = std::max(1u, size.depth >> mipLevel);

	subresource.m_Width = width;
	subresource.m_Height = height;
	subresource.m_Detph = depth;

	auto formatInfo = GetFormatInfo(format);
	if (formatInfo.m_IsCompressed)
	{
		u64 numBlocksWide = 0;
		if (width > 0)
		{
			numBlocksWide = std::max(1ull, (u64(width) + 3ull) >> 2ull);
		}
		u64 numBlocksHigh = 0;
		if (height > 0)
		{
			numBlocksHigh = std::max(1ull, (u64(height) + 3ull) >> 2ull);
		}
		subresource.m_RowPitch = numBlocksWide * formatInfo.m_BlocksPerElement;
		subresource.m_NumRows = numBlocksHigh;
		subresource.m_SlicePitch = subresource.m_RowPitch * numBlocksHigh;
	}
	else if (formatInfo.m_IsPacked)
	{
		subresource.m_RowPitch = ((u64(width) + 1ull) >> 1ull) * formatInfo.m_BlocksPerElement;
		subresource.m_NumRows = u64(height);
		subresource.m_SlicePitch = subresource.m_RowPitch * height;
	}
	else
	{
		switch (format)
		{
		case Format::kNV11:
			subresource.m_RowPitch = ((u64(width) + 3ull) >> 2ull) * 4ull;
			subresource.m_NumRows = u64(height) * 2ull; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
			subresource.m_SlicePitch = subresource.m_RowPitch * subresource.m_NumRows;
			break;
		case Format::kNV12:
		case Format::k420_OPAQUE:
		case Format::kP208:
		case Format::kP010:
		case Format::kP016:
			subresource.m_RowPitch = ((u64(width) + 1ull) >> 1ull) * formatInfo.m_BlocksPerElement;
			subresource.m_NumRows = height + ((u64(height) + 1ull) >> 1ull);
			subresource.m_SlicePitch = (subresource.m_RowPitch * u64(height)) + ((subresource.m_RowPitch * u64(height) + 1ull) >> 1ull);
			break;
		default:
			BV_ASSERT(formatInfo.m_BitsPerPixel > 0, "Bits per pixel can't be 0");

			subresource.m_RowPitch = (u64(width) * formatInfo.m_BitsPerPixel + 7ull) >> 3ull; // round up to nearest byte
			subresource.m_NumRows = u64(height);
			subresource.m_SlicePitch = subresource.m_RowPitch * height;
			break;
		}
	}
	subresource.m_MipSize = subresource.m_SlicePitch * depth;
}


void AdjustPlaneSubresourceData(Format format, u32 height, u32 planeSlice, SubresourceData& subresource)
{
	switch (format)
	{
	case Format::kNV12:
	case Format::kP010:
	case Format::kP016:
		if (planeSlice == 0)
		{
			subresource.m_SlicePitch = subresource.m_RowPitch * u64(height);
		}
		else
		{
			subresource.m_pData += uintptr_t(subresource.m_RowPitch) * height;
			subresource.m_SlicePitch = subresource.m_RowPitch * ((u64(height) + 1ull) >> 1ull);
		}
		break;
	case Format::kNV11:
		if (planeSlice == 0)
		{
			subresource.m_SlicePitch = subresource.m_RowPitch * u64(height);
		}
		else
		{
			subresource.m_pData += uintptr_t(subresource.m_RowPitch) * height;
			subresource.m_RowPitch >>= 1;
			subresource.m_SlicePitch = subresource.m_RowPitch * height;
		}
		break;
	}
}


u64 GetBufferSizeForTexture(const TextureDesc& textureDesc, u32 alignment, u32 maxSubresourceCount, SubresourceFootprint* pSubresources)
{
	u64 totalSize = 0;
	TextureSubresourceInfo subresourceInfo{};
	u32 planeCount = GetFormatInfo(textureDesc.m_Format).m_PlaneCount;
	u32 subresourceIndex = 0;
	for (auto plane = 0u; plane < planeCount; ++plane)
	{
		for (auto array = 0u; array < textureDesc.m_ArraySize; ++array)
		{
			for (auto mip = 0u; mip < textureDesc.m_MipLevels && subresourceIndex < maxSubresourceCount; ++mip, ++subresourceIndex)
			{
				GetTextureSubresourceInfo(subresourceInfo, textureDesc.m_Format, textureDesc.m_Size, mip);
				SubresourceData subresourceData{ reinterpret_cast<const u8*>(totalSize), subresourceInfo.m_RowPitch, subresourceInfo.m_SlicePitch };
				AdjustPlaneSubresourceData(textureDesc.m_Format, subresourceInfo.m_Height, plane, subresourceData);
				subresourceInfo.m_RowPitch = subresourceData.m_RowPitch;
				subresourceInfo.m_SlicePitch = subresourceData.m_SlicePitch;
				subresourceInfo.m_MipSize = subresourceInfo.m_SlicePitch * subresourceInfo.m_Detph;

				if (pSubresources)
				{
					pSubresources[subresourceIndex].m_Offset = reinterpret_cast<u64>(subresourceData.m_pData);
					pSubresources[subresourceIndex].m_Subresource = subresourceInfo;
				}

				totalSize += RoundToNearestPowerOf2(subresourceInfo.m_MipSize, alignment);
			}
		}
	}

	return totalSize;
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