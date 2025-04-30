#pragma once


#include "BvTexture.h"


void GetTextureSubresourceInfo(TextureSubresourceInfo& subresource, Format format, const Extent3D& size, u32 mipLevel = 0);
void AdjustPlaneSubresourceData(Format format, u32 height, u32 planeSlice, SubresourceData& subresource);
u64 GetBufferSizeForTexture(const TextureDesc& textureDesc, u32 alignment, u32 maxSubresourceCount, SubresourceFootprint* pSubresources = nullptr);
void CopySubresource(const void* pSrcData, void* pDstData, const SubresourceFootprint& srcSubresource, const SubresourceFootprint& dstSubresource);
u32 GetMipCount(u32 width, u32 height, u32 depth = 1);

BV_INLINE f32 ConvertD3D12DepthBiasToVulkan(i32 depthBiasInt, u32 depthBits)
{
	depthBits = std::clamp(depthBits, 1u, 24u); // clamp to safe range
	return depthBiasInt / f32(i32(1u << depthBits) - 1);
}

BV_INLINE i32 ConvertVulkanDepthBiasToD3D12(f32 depthBiasFloat, u32 depthBits)
{
	depthBits = std::clamp(depthBits, 1u, 24u); // clamp to safe range
	return static_cast<i32>(std::round(depthBiasFloat * i32((1u << depthBits) - 1)));
}