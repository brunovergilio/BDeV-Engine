#pragma once


#include "BvCommonGl.h"
#include "BDeV/RenderAPI/BvBuffer.h"
#include "BDeV/RenderAPI/BvTexture.h"
#include "BDeV/RenderAPI/BvSampler.h"
#include "BDeV/RenderAPI/BvPipelineState.h"


struct FormatMapGl
{
	GLint componentCount;
	GLenum format;
	GLenum type;
	GLboolean normalized;
};


GLenum GetGlBufferUsageFlags(BufferUsage usageFlags);
GLenum GetGlBufferMemoryFlags(MemoryType memoryFlags);
GLenum GetGlBufferMappingFlags(MemoryType memoryFlags);
GLenum GetGlTextureTarget(const TextureDesc& textureDesc);
FormatMapGl GetGlFormatMap(Format format);
GLenum GetGlFormat(Format format);
GLint GetGlMinFilter(Filter filter, MipMapFilter mipMapFilter);
GLint GetGlMagFilter(Filter filter);
GLint GetGlAddressMode(AddressMode addressMode);
GLint GetGlCompareOp(CompareOp compareOp);
GLenum GetGlFillMode(FillMode fillMode);
GLenum GetGlCullMode(CullMode cullMode);
GLenum GetGlFrontFace(FrontFace frontFace);
GLenum GetGlStencilOp(StencilOp stencilOp);
GLenum GetGlBlendOp(BlendOp blendOp);
GLenum GetGlBlendFactor(BlendFactor blendFactor);