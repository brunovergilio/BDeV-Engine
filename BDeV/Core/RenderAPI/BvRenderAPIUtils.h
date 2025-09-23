#pragma once


#include "BvRenderCommon.h"


void GetTextureSubresourceInfo(TextureSubresourceInfo& subresource, Format format, const Extent3D& size, u32 mipLevel = 0);
void AdjustPlaneSubresourceData(Format format, u32 height, u32 planeSlice, SubresourceData& subresource);
u64 GetBufferSizeForTexture(const TextureDesc& textureDesc, u32 alignment, u32 maxSubresourceCount, SubresourceFootprint* pSubresources = nullptr);
void CopySubresource(const void* pSrcData, void* pDstData, const SubresourceFootprint& srcSubresource, const SubresourceFootprint& dstSubresource);
u32 GetMipCount(u32 width = 1, u32 height = 1, u32 depth = 1);

BV_INLINE f32 ConvertIntDepthBiasToFloat(i32 depthBiasInt, u32 depthBits)
{
	depthBits = std::clamp(depthBits, 1u, 24u); // clamp to safe range
	return depthBiasInt / f32(i32(1u << depthBits) - 1);
}

BV_INLINE i32 ConvertFloatDepthBiasToInt(f32 depthBiasFloat, u32 depthBits)
{
	depthBits = std::clamp(depthBits, 1u, 24u); // clamp to safe range
	return static_cast<i32>(std::round(depthBiasFloat * i32((1u << depthBits) - 1)));
}

namespace Internal
{
	BV_INLINE std::pair<f32, f32> GetDepthBiasMRDForVkAndD3D(Format format)
	{
		constexpr f32 k1Div24Minus1 = 1.0f / ((1u << 24) - 1);
		constexpr f32 k1Div24 = 1.0f / (1u << 24);
		constexpr f32 k1Div16Minus1 = 1.0f / ((1u << 16) - 1);
		constexpr f32 k1Div16 = 1.0f / (1u << 16);
		constexpr f32 k1Div23 = 1.0f / (1u << 23);

		switch (format)
		{
		case Format::kD32_Float_S8X24_UInt:
		case Format::kD32_Float:
			return { k1Div23, k1Div23 };
		case Format::kD24_UNorm_S8_UInt:
			return { k1Div24Minus1, k1Div24 };
		case Format::kD16_UNorm:
			return { k1Div16Minus1, k1Div16 };
		default:
			return { 1.0f, 1.0f };
		}
	}

	BV_INLINE i32 GetDepthBiasVkToD3D(Format format, f32 depthBiasConstantFactor)
	{
		auto[vk, d3d] = GetDepthBiasMRDForVkAndD3D(format);
		f32 ratio = vk / d3d;
		return static_cast<i32>(std::roundf(depthBiasConstantFactor * ratio));
	}

	BV_INLINE f32 GetDepthBiasD3DToVk(Format format, i32 depthBias)
	{
		auto[vk, d3d] = GetDepthBiasMRDForVkAndD3D(format);
		f32 ratio = d3d / vk;
		return static_cast<f32>(depthBias) * ratio;
	}
}