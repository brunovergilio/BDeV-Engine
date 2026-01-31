#include "BvRenderAPIUtils.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


u32 BvRenderUtils::CalcRowPitch(Format format, u32 width)
{
	auto fi = GetFormatInfo(format);
	u32 wAlign = fi.m_WidthAlignment;
	u32 numUnits;

	if (fi.m_IsCompressed)
	{
		numUnits = DivideAndRoundUp(width, wAlign);
	}
	else
	{
		numUnits = RoundToNearestPowerOf2(width, wAlign);
	}

	return (fi.m_BitsPerPixel * numUnits) >> 3;
}


void BvRenderUtils::AdjustPlaneSizeAndFormatForCopyableLayout(Format format, u32 width, u32 height, u32 planeSlice,
	Format& planeFormat, u32& planeWidth, u32& planeHeight, u32& minPlanePitchWidth)
{
	auto fi = GetFormatInfo(GetFormatInfo(format).m_ParentFormat);
	if (fi.m_IsPlanar)
	{
		switch (fi.m_ParentFormat)
		{
		case Format::kNV12:
			switch (planeSlice)
			{
			case 0:
				planeFormat = Format::kR8_Typeless;
				planeWidth = width;
				planeHeight = height;
				break;
			case 1:
				planeFormat = Format::kRG8_Typeless;
				planeWidth = (width + 1) >> 1;
				planeHeight = (height + 1) >> 1;
				break;
			};

			minPlanePitchWidth = planeWidth;
			break;
		case Format::kP010: [[fallthrough]];
		case Format::kP016:
			switch (planeSlice)
			{
			case 0:
				planeFormat = Format::kR16_Typeless;
				planeWidth = width;
				planeHeight = height;
				break;
			case 1:
				planeFormat = Format::kRG16_Typeless;
				planeWidth = (width + 1) >> 1;
				planeHeight = (height + 1) >> 1;
				break;
			};
			minPlanePitchWidth = planeWidth;
			break;
		case Format::kP208:
			switch (planeSlice)
			{
			case 0:
				planeFormat = Format::kR8_Typeless;
				planeWidth = width;
				planeHeight = height;
				break;
			case 1:
				planeFormat = Format::kRG8_Typeless;
				planeWidth = (width + 1) >> 1;
				planeHeight = height;
				break;
			};

			minPlanePitchWidth = planeWidth;
			break;
		case Format::kV208:
			planeFormat = Format::kR8_Typeless;
			switch (planeSlice)
			{
			case 0:
				planeWidth = width;
				planeHeight = height;
				break;
			case 1: [[fallthrough]];
			case 2:
				planeWidth = width;
				planeHeight = (height + 1) >> 1;
				break;
			}

			minPlanePitchWidth = planeWidth;
			break;
		case Format::kV408:
			planeFormat = Format::kR8_Typeless;
			planeWidth = width;
			planeHeight = height;
			minPlanePitchWidth = planeWidth;
			break;
		case Format::kNV11:
			switch (planeSlice)
			{
			case 0:
				planeFormat = Format::kR8_Typeless;
				planeWidth = width;
				planeHeight = height;
				minPlanePitchWidth = width;
				break;
			case 1:
				planeFormat = Format::kRG8_Typeless;
				planeWidth = (width + 3) >> 2;
				planeHeight = height;

				// NV11 has unused padding to the right of the chroma plane in the RowMajor (linear) copyable layout.
				minPlanePitchWidth = (width + 1) >> 1;
				break;
			}
			break;
		case Format::kR32G8X24_Typeless: [[fallthrough]];
		case Format::kR24G8_Typeless:
			switch (planeSlice)
			{
			case 0:
				planeFormat = Format::kR32_Typeless;
				planeWidth = width;
				planeHeight = height;
				minPlanePitchWidth = width;
				break;
			case 1:
				planeFormat = Format::kR8_Typeless;
				planeWidth = width;
				planeHeight = height;
				minPlanePitchWidth = width;
				break;
			}
			break;
		}
	}
	else
	{
		planeFormat = format;
		planeWidth = width;
		planeHeight = height;
		minPlanePitchWidth = planeWidth;
	}
}


u64 BvRenderUtils::GetCopyableFootprints(const TextureDesc& textureDesc, u32 placementAlignment, u32 pitchAlignment, u32 numSubresources,
	SubresourceFootprint* pSubresources, u64 baseOffset, u32 firstSubresource)
{
	auto fi = BvRenderUtils::GetFormatInfo(textureDesc.m_Format);

	const u32 wAlign = fi.m_WidthAlignment;
	const u32 hAlign = fi.m_HeightAlignment;
	const u32 dAlign = fi.m_DepthAlignment;
	u32 subRes = 0;

	u64 totalBytes = 0;
	u32 subresourceCount = textureDesc.m_MipLevels * textureDesc.m_ArraySize * fi.m_PlaneCount;
	for (; subRes < numSubresources; ++subRes)
	{
		u32 subresourceIndex = firstSubresource + subRes;
		if (subresourceIndex > subresourceCount)
		{
			break;
		}

		totalBytes = RoundToNearestPowerOf2(totalBytes, placementAlignment);
		u32 mipLevel, arraySlice, planeSlice;
		DecomposeSubresourceIndex(subresourceIndex, textureDesc.m_MipLevels, textureDesc.m_ArraySize, mipLevel, arraySlice, planeSlice);

		u32 width = RoundToNearestPowerOf2(textureDesc.m_Size.m_Width >> mipLevel, wAlign);
		u32 height = RoundToNearestPowerOf2(textureDesc.m_Size.m_Height >> mipLevel, hAlign);
		u32 depth = RoundToNearestPowerOf2(textureDesc.m_Size.m_Depth >> mipLevel, dAlign);

		Format planeFormat;
		u32 minPlanePitchWidth, planeWidth, planeHeight;
		AdjustPlaneSizeAndFormatForCopyableLayout(textureDesc.m_Format, width, height, planeSlice, planeFormat, planeWidth, planeHeight, minPlanePitchWidth);

		SubresourceFootprint localFootprint{};
		auto& footprint = pSubresources ? pSubresources[subRes] : localFootprint;
		footprint.m_Subresource.m_Format = planeFormat;
		footprint.m_Subresource.m_Width = planeWidth;
		footprint.m_Subresource.m_Height = planeHeight;
		footprint.m_Subresource.m_Detph = depth;

		// Calculate row pitch
		u32 minPlaneRowPitch = CalcRowPitch(planeFormat, minPlanePitchWidth);
		u32 numRows = fi.m_IsPlanar ? planeHeight : (height / hAlign);

		footprint.m_Subresource.m_NumRows = numRows;
		footprint.m_Subresource.m_RowPitch = RoundToNearestPowerOf2(minPlaneRowPitch, fi.m_IsPlanar ? placementAlignment : pitchAlignment);
		footprint.m_Subresource.m_RowSize = CalcRowPitch(planeFormat, planeWidth);
		footprint.m_Subresource.m_SlicePitch = footprint.m_Subresource.m_NumRows * footprint.m_Subresource.m_RowPitch;
		footprint.m_Offset = totalBytes + baseOffset;

		u64 subresourceSize = (numRows * depth - 1) * footprint.m_Subresource.m_RowPitch + minPlaneRowPitch;
		totalBytes += subresourceSize;
	}

	return totalBytes;
}


BV_INLINE void CopySubresourceRows(const SubresourceData& src, void* pDst, const TextureSubresourceInfo& dst)
{
	for (auto z = 0u; z < dst.m_Detph; ++z)
	{
		auto pDestSlice = static_cast<u8*>(pDst) + dst.m_SlicePitch * u64(z);
		auto pSrcSlice = static_cast<const u8*>(src.m_pData) + src.m_SlicePitch * u64(z);
		for (auto y = 0u; y < dst.m_NumRows; ++y)
		{
			memcpy(pDestSlice + dst.m_RowPitch * u64(y), pSrcSlice + src.m_RowPitch * u64(y), dst.m_RowSize);
		}
	}
};


void BvRenderUtils::UpdateSubresources(u32 numSubresources, const SubresourceData* pSubresources, const SubresourceFootprint* pFootprints, void* pDst)
{
	for (auto i = 0; i < numSubresources; ++i)
	{
		u8* pData = reinterpret_cast<u8*>(pDst) + pFootprints[i].m_Offset;
		CopySubresourceRows(pSubresources[i], pDst, pFootprints[i].m_Subresource);
	}
}