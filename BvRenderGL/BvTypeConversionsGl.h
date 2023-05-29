#pragma once


#include "BvCommonGl.h"
#include "BDeV/RenderAPI/BvBuffer.h"
#include "BDeV/RenderAPI/BvTexture.h"
#include "BDeV/RenderAPI/BvSampler.h"


GLenum GetGlBufferUsageFlags(BufferUsage usageFlags);
GLenum GetGlBufferMemoryFlags(MemoryFlags memoryFlags);
GLenum GetGlBufferMappingFlags(MemoryFlags memoryFlags);
GLenum GetGlTextureTarget(const TextureDesc& textureDesc);
GLenum GetGlFormat(Format format);
GLint GetGlMinFilter(Filter filter, MipMapFilter mipMapFilter);
GLint GetGlMagFilter(Filter filter);
GLint GetGlAddressMode(AddressMode addressMode);
GLint GetGlCompareOp(CompareOp compareOp);