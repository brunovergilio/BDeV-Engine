#pragma once


#include "BvTexture.h"


void GetTextureSubresourceInfo(TextureSubresourceInfo& subresource, Format format, const Extent3D& size, u32 mipLevel = 0);
void AdjustPlaneSubresourceData(Format format, u32 height, u32 planeSlice, SubresourceData& subresource);
u64 GetBufferSizeForTexture(const TextureDesc& textureDesc, u32 alignment, u32 maxSubresourceCount, SubresourceFootprint* pSubresources = nullptr);
void CopySubresource(const void* pSrcData, void* pDstData, const SubresourceFootprint& srcSubresource, const SubresourceFootprint& dstSubresource);
u32 GetMipCount(u32 width, u32 height, u32 depth = 1);