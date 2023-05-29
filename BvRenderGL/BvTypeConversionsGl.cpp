#include "BvTypeConversionsGl.h"
#include "BDeV/Container/BvRobinMap.h"


static BvRobinMap<Format, GLenum> s_FormatToGlFormatMap =
{
	{ Format::kUnknown,				0 },

	{ Format::kRGBA32_Float,		GL_RGBA32F },
	{ Format::kRGBA32_UInt,			GL_RGBA32UI },
	{ Format::kRGBA32_SInt,			GL_RGBA32I },

	{ Format::kRGB32_Float,			GL_RGB32F },
	{ Format::kRGB32_UInt,			GL_RGB32UI },
	{ Format::kRGB32_SInt,			GL_RGB32I },

	{ Format::kRG32_Float,			GL_RG32F },
	{ Format::kRG32_UInt,			GL_RG32UI },
	{ Format::kRG32_SInt,			GL_RG32I },

	{ Format::kR32_Float,			GL_R32F },
	{ Format::kR32_UInt,			GL_R32UI },
	{ Format::kR32_SInt,			GL_R32I },

	{ Format::kRGBA16_Float,		GL_RGBA16F },
	{ Format::kRGBA16_UNorm,		GL_RGBA16 },
	{ Format::kRGBA16_UInt,			GL_RGBA16UI },
	{ Format::kRGBA16_SNorm,		GL_RGBA16_SNORM },
	{ Format::kRGBA16_SInt,			GL_RGBA16I },

	{ Format::kRG16_Float,			GL_RG16F },
	{ Format::kRG16_UNorm,			GL_RG16 },
	{ Format::kRG16_UInt,			GL_RG16UI },
	{ Format::kRG16_SNorm,			GL_RG16_SNORM },
	{ Format::kRG16_SInt,			GL_RG16I },

	{ Format::kR16_Float,			GL_R16F },
	{ Format::kR16_UNorm,			GL_R16 },
	{ Format::kR16_UInt,			GL_R16UI },
	{ Format::kR16_SNorm,			GL_R16_SNORM },
	{ Format::kR16_SInt,			GL_R16I },

	{ Format::kRGBA8_UNorm_SRGB,	GL_SRGB8_ALPHA8 },
	{ Format::kRGBA8_UNorm,			GL_RGBA8 },
	{ Format::kRGBA8_UInt,			GL_RGBA8UI },
	{ Format::kRGBA8_SNorm,			GL_RGBA8_SNORM },
	{ Format::kRGBA8_SInt,			GL_RGBA8I },

	{ Format::kBGRA8_UNorm_SRGB,	0 },
	{ Format::kBGRA8_UNorm,			0 },

	{ Format::kRG8_UNorm,			GL_RG8 },
	{ Format::kRG8_UInt,			GL_RG8UI },
	{ Format::kRG8_SNorm,			GL_RG8_SNORM },
	{ Format::kRG8_SInt,			GL_RG8I },

	{ Format::kR8_UNorm,			GL_R8 },
	{ Format::kR8_UInt,				GL_R8UI },
	{ Format::kR8_SNorm,			GL_R8_SNORM },
	{ Format::kR8_SInt,				GL_R8I },

	{ Format::kD32_Float_S8X24_UInt,GL_DEPTH32F_STENCIL8 },
	{ Format::kD32_Float,			GL_DEPTH_COMPONENT32F },
	{ Format::kD24_UNorm_S8_UInt,	GL_DEPTH24_STENCIL8 },
	{ Format::kD16_UNorm,			GL_DEPTH_COMPONENT16 },

	{ Format::kRGB10A2_UNorm,		GL_RGB10_A2 },
	{ Format::kRGB10A2_UInt,		GL_RGB10_A2UI },
	{ Format::kRG11B10_Float,		GL_R11F_G11F_B10F },

	{ Format::kBC1_UNorm,			GL_COMPRESSED_RGB_S3TC_DXT1_EXT },
	{ Format::kBC1_UNorm_SRGB,		GL_COMPRESSED_SRGB_S3TC_DXT1_EXT },

	{ Format::kBC2_UNorm,			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT },
	{ Format::kBC2_UNorm_SRGB,		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT },

	{ Format::kBC3_UNorm,			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT },
	{ Format::kBC3_UNorm_SRGB,		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT },

	{ Format::kBC4_UNorm,			GL_COMPRESSED_RED_RGTC1 },
	{ Format::kBC4_SNorm,			GL_COMPRESSED_SIGNED_RED_RGTC1 },

	{ Format::kBC5_UNorm,			GL_COMPRESSED_RG_RGTC2 },
	{ Format::kBC5_SNorm,			GL_COMPRESSED_SIGNED_RG_RGTC2 },

	{ Format::kBC6H_UF16,			GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT },
	{ Format::kBC6H_SF16,			GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT },

	{ Format::kBC7_UNorm,			GL_COMPRESSED_RGBA_BPTC_UNORM },
	{ Format::kBC7_UNorm_SRGB,		GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM },
};


GLenum GetGlBufferUsageFlags(BufferUsage usageFlags)
{
	GLenum target = 0;
	if ((usageFlags & BufferUsage::kVertexBuffer) == BufferUsage::kVertexBuffer)
	{
		target = GL_ARRAY_BUFFER;
	}
	else if ((usageFlags & BufferUsage::kIndexBuffer) == BufferUsage::kIndexBuffer)
	{
		target = GL_ELEMENT_ARRAY_BUFFER;
	}
	else if ((usageFlags & BufferUsage::kUniformBuffer) == BufferUsage::kUniformBuffer)
	{
		target = GL_UNIFORM_BUFFER;
	}
	else if ((usageFlags & BufferUsage::kIndirectBuffer) == BufferUsage::kIndirectBuffer)
	{
		// Requires GL_ARB_draw_indirect
		target = GL_DRAW_INDIRECT_BUFFER;
	}
	else if ((usageFlags & BufferUsage::kTransferSrc) == BufferUsage::kTransferSrc)
	{
		target = GL_COPY_READ_BUFFER;
	}
	else if ((usageFlags & BufferUsage::kTransferDst) == BufferUsage::kTransferDst)
	{
		target = GL_COPY_WRITE_BUFFER;
	}

	return target;
}


GLenum GetGlBufferMemoryFlags(MemoryFlags memoryFlags)
{
	switch (memoryFlags)
	{
	case MemoryFlags::kDeviceLocal:		return 0;
	case MemoryFlags::kHostVisible:		return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;
	case MemoryFlags::kHostCoherent:	return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT;
	case MemoryFlags::kUpload:			return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT;
	case MemoryFlags::kHostCached:		return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
	case MemoryFlags::kReadBack:		return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
	}

	return 0;
}


GLenum GetGlBufferMappingFlags(MemoryFlags memoryFlags)
{
	GLenum mappingFlags = GL_MAP_WRITE_BIT;
	if ((memoryFlags & MemoryFlags::kHostCached) == MemoryFlags::kHostCached)
	{
		mappingFlags |= GL_MAP_READ_BIT;
	}

	return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
}


GLenum GetGlTextureTarget(const TextureDesc& textureDesc)
{
	switch (textureDesc.m_ImageType)
	{
	case TextureType::kTexture1D:
		if (textureDesc.m_Size.height == 1)
		{
			return GL_TEXTURE_1D;
		}
		else
		{
			return GL_TEXTURE_1D_ARRAY;
		}
	case TextureType::kTexture2D:
		if (textureDesc.m_SampleCount == 1)
		{
			if (!textureDesc.m_UseAsCubeMap)
			{
				if (textureDesc.m_Size.depthOrLayerCount == 1)
				{
					return GL_TEXTURE_2D;
				}
				else
				{
					return GL_TEXTURE_2D_ARRAY;
				}
			}
			else
			{
				if (textureDesc.m_Size.depthOrLayerCount <= 6)
				{
					return GL_TEXTURE_CUBE_MAP;
				}
				else
				{
					return GL_TEXTURE_CUBE_MAP_ARRAY;
				}
			}
		}
		else
		{
			if (!textureDesc.m_UseAsCubeMap)
			{
				if (textureDesc.m_Size.depthOrLayerCount == 1)
				{
					return GL_TEXTURE_2D_MULTISAMPLE;
				}
				else
				{
					return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
				}
			}
			else
			{
				if (textureDesc.m_Size.depthOrLayerCount <= 6)
				{
					return GL_TEXTURE_CUBE_MAP;
				}
				else
				{
					return GL_TEXTURE_CUBE_MAP_ARRAY;
				}
			}
		}
	case TextureType::kTexture3D:
		return GL_TEXTURE_3D;
	}

	return GL_TEXTURE_2D;
}


GLenum GetGlFormat(Format format)
{
	auto it = s_FormatToGlFormatMap.FindKey(format);
	if (it != s_FormatToGlFormatMap.cend())
	{
		return it->second;
	}

	return 0;
}


GLint GetGlMinFilter(Filter filter, MipMapFilter mipMapFilter)
{
	if (mipMapFilter == MipMapFilter::kLinear)
	{
		return filter == Filter::kLinear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
	}
	else
	{
		return filter == Filter::kLinear ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
	}
}


GLint GetGlMagFilter(Filter filter)
{
	return filter == Filter::kLinear ? GL_LINEAR : GL_NEAREST;
}


GLint GetGlAddressMode(AddressMode addressMode)
{
	switch (addressMode)
	{
	case AddressMode::kWrap:		return GL_REPEAT;
	case AddressMode::kMirror:		return GL_MIRRORED_REPEAT;
	case AddressMode::kClamp:		return GL_CLAMP_TO_EDGE;
	case AddressMode::kBorder:		return GL_CLAMP_TO_BORDER;
	case AddressMode::kMirrorOnce:	return GL_MIRROR_CLAMP_TO_EDGE;
	}
	
	return 0;
}


GLint GetGlCompareOp(CompareOp compareOp)
{
	switch (compareOp)
	{
	case CompareOp::kNever:			return GL_NEVER;
	case CompareOp::kLess:			return GL_LESS;
	case CompareOp::kEqual:			return GL_EQUAL;
	case CompareOp::kLessEqual:		return GL_LEQUAL;
	case CompareOp::kGreater:		return GL_GREATER;
	case CompareOp::kNotEqual:		return GL_NOTEQUAL;
	case CompareOp::kGreaterEqual:	return GL_GEQUAL;
	case CompareOp::kAlways:		return GL_ALWAYS;
	}

	return 0;
}