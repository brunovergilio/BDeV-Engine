#pragma once


#include "BvTexture.h"


void GetTextureSubresourceData(const TextureDesc& textureDesc, u32 mipLevel, TextureSubresource& subresource);
u64 GetTextureDataOffset(const TextureDesc& textureDesc, u32 mipLevel, u32 arraySlice, u32 alignment);
u64 GetBufferSizeForTextureData(const TextureDesc& textureDesc, u32 alignment);
void CopySubresource(const void* pSrcData, void* pDstData, const SubresourceFootprint& srcSubresource, const SubresourceFootprint& dstSubresource);
u32 GetMipCount(u32 width, u32 height, u32 depth);