#include "BvTypeConversionsGl.h"
#include "BDeV/Container/BvRobinMap.h"


static constexpr FormatMapGl kFormatMapGl[] =
{
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ GL_RGBA32F,								0,					0, false }, // Format::kRGBA32_Typeless,
	{ GL_RGBA32F,								GL_FLOAT,			4, false }, // Format::kRGBA32_Float,
	{ GL_RGBA32UI,								GL_UNSIGNED_INT,	4, false }, // Format::kRGBA32_UInt,
	{ GL_RGBA32I,								GL_INT,				4, false }, // Format::kRGBA32_SInt,
	{ GL_RGB32F,								0,					0, false }, // Format::kRGB32_Typeless,
	{ GL_RGB32F,								GL_FLOAT,			3, false }, // Format::kRGB32_Float,
	{ GL_RGB32UI,								GL_UNSIGNED_INT,	3, false }, // Format::kRGB32_UInt,
	{ GL_RGB32I,								GL_INT,				3, false }, // Format::kRGB32_SInt,
	{ GL_RGBA16F,								0,					0, false }, // Format::kRGBA16_Typeless,
	{ GL_RGBA16F,								GL_HALF_FLOAT,		4, false }, // Format::kRGBA16_Float,
	{ GL_RGBA16,								GL_HALF_FLOAT,		4, true  }, // Format::kRGBA16_UNorm,
	{ GL_RGBA16UI,								GL_UNSIGNED_SHORT,	4, false }, // Format::kRGBA16_UInt,
	{ GL_RGBA16_SNORM,							GL_HALF_FLOAT,		4, true  }, // Format::kRGBA16_SNorm,
	{ GL_RGBA16I,								GL_SHORT,			4, false }, // Format::kRGBA16_SInt,
	{ GL_RG32F,									0,					0, false }, // Format::kRG32_Typeless,
	{ GL_RG32F,									GL_FLOAT,			2, false }, // Format::kRG32_Float,
	{ GL_RG32UI,								GL_UNSIGNED_INT,	2, false }, // Format::kRG32_UInt,
	{ GL_RG32I,									GL_INT,				2, false }, // Format::kRG32_SInt,
	{ GL_DEPTH32F_STENCIL8,						0,					0, false }, // Format::kR32G8X24_Typeless,
	{ GL_DEPTH32F_STENCIL8,						0,					0, false }, // Format::kD32_Float_S8X24_UInt,
	{ GL_DEPTH32F_STENCIL8,						0,					0, false }, // Format::kR32_Float_X8X24_Typeless,
	{ 0,										0,					0, false }, // Format::kX32_Typeless_G8X24_UInt,
	{ GL_RGB10_A2,								0,					0, false }, // Format::kRGB10A2_Typeless,
	{ GL_RGB10_A2,								0,					0, false }, // Format::kRGB10A2_UNorm,
	{ GL_RGB10_A2UI,							0,					0, false }, // Format::kRGB10A2_UInt,
	{ GL_R11F_G11F_B10F,						0,					0, false }, // Format::kRG11B10_Float,
	{ GL_RGBA8,									0,					0, false }, // Format::kRGBA8_Typeless,
	{ GL_RGBA8,									GL_UNSIGNED_BYTE,	4, true  }, // Format::kRGBA8_UNorm,
	{ GL_SRGB8_ALPHA8,							0,					0, false }, // Format::kRGBA8_UNorm_SRGB,
	{ GL_RGBA8UI,								GL_UNSIGNED_BYTE,	4, false }, // Format::kRGBA8_UInt,
	{ GL_RGBA8_SNORM,							GL_BYTE,			4, true  }, // Format::kRGBA8_SNorm,
	{ GL_RGBA8I,								GL_BYTE,			4, false }, // Format::kRGBA8_SInt,
	{ GL_RG16F,									0,					2, false }, // Format::kRG16_Typeless,
	{ GL_RG16F,									GL_HALF_FLOAT,		2, false }, // Format::kRG16_Float,
	{ GL_RG16,									GL_HALF_FLOAT,		2, true  }, // Format::kRG16_UNorm,
	{ GL_RG16UI,								GL_UNSIGNED_SHORT,	2, false }, // Format::kRG16_UInt,
	{ GL_RG16_SNORM,							GL_HALF_FLOAT,		2, true  }, // Format::kRG16_SNorm,
	{ GL_RG16I,									GL_SHORT,			2, false }, // Format::kRG16_SInt,
	{ GL_R32F,									0,					0, false }, // Format::kR32_Typeless,
	{ GL_DEPTH_COMPONENT32F,					0,					0, false }, // Format::kD32_Float,
	{ GL_R32F,									GL_FLOAT,			1, false }, // Format::kR32_Float,
	{ GL_R32UI,									GL_UNSIGNED_INT,	1, false }, // Format::kR32_UInt,
	{ GL_R32I,									GL_INT,				1, false }, // Format::kR32_SInt,
	{ GL_DEPTH24_STENCIL8,						0,					0, false }, // Format::kR24G8_Typeless,
	{ GL_DEPTH24_STENCIL8,						0,					0, false }, // Format::kD24_UNorm_S8_UInt,
	{ GL_DEPTH24_STENCIL8,						0,					0, false }, // Format::kR24_UNorm_X8_Typeless,
	{ 0,										0,					0, false }, // Format::kX24_Typeless_G8_UInt,
	{ GL_RG8,									0,					0, false }, // Format::kRG8_Typeless,
	{ GL_RG8,									GL_UNSIGNED_BYTE,	2, true  }, // Format::kRG8_UNorm,
	{ GL_RG8UI,									GL_UNSIGNED_BYTE,	2, false }, // Format::kRG8_UInt,
	{ GL_RG8_SNORM,								GL_BYTE,			2, true  }, // Format::kRG8_SNorm,
	{ GL_RG8I,									GL_BYTE,			2, false }, // Format::kRG8_SInt,
	{ GL_R16F,									0,					0, false }, // Format::kR16_Typeless,
	{ GL_R16F,									GL_HALF_FLOAT,		1, false }, // Format::kR16_Float,
	{ GL_DEPTH_COMPONENT16,						0,					0, false }, // Format::kD16_UNorm,
	{ GL_R16,									GL_HALF_FLOAT,		1, true  }, // Format::kR16_UNorm,
	{ GL_R16UI,									GL_UNSIGNED_SHORT,	1, false }, // Format::kR16_UInt,
	{ GL_R16_SNORM,								GL_HALF_FLOAT,		1, true  }, // Format::kR16_SNorm,
	{ GL_R16I,									GL_SHORT,			1, false }, // Format::kR16_SInt,
	{ GL_R8,									0,					0, false }, // Format::kR8_Typeless,
	{ GL_R8,									GL_UNSIGNED_BYTE,	1, true  }, // Format::kR8_UNorm,
	{ GL_R8UI,									GL_UNSIGNED_BYTE,	1, false }, // Format::kR8_UInt,
	{ GL_R8_SNORM,								GL_BYTE,			1, true	 }, // Format::kR8_SNorm,
	{ GL_R8I,									GL_BYTE,			1, false }, // Format::kR8_SInt,
	{ GL_R8,									0,					0, false }, // Format::kA8_UNorm,
	{ 0,										0,					0, false }, // Format::kR1_UNorm,
	{ GL_RGB9_E5,								0,					0, false }, // Format::kRGB9E5_SHAREDEXP,
	{ 0,										0,					0, false }, // Format::kRG8_BG8_UNorm,
	{ 0,										0,					0, false }, // Format::kGR8_GB8_UNorm,
	{ GL_COMPRESSED_RGB_S3TC_DXT1_EXT,			0,					0, false }, // Format::kBC1_Typeless,
	{ GL_COMPRESSED_RGB_S3TC_DXT1_EXT,			0,					0, false }, // Format::kBC1_UNorm,
	{ GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,			0,					0, false }, // Format::kBC1_UNorm_SRGB,
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,			0,					0, false }, // Format::kBC2_Typeless,
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,			0,					0, false }, // Format::kBC2_UNorm,
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,	0,					0, false }, // Format::kBC2_UNorm_SRGB,
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,			0,					0, false }, // Format::kBC3_Typeless,
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,			0,					0, false }, // Format::kBC3_UNorm,
	{ GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,	0,					0, false }, // Format::kBC3_UNorm_SRGB,
	{ GL_COMPRESSED_RED_RGTC1,					0,					0, false }, // Format::kBC4_Typeless,
	{ GL_COMPRESSED_RED_RGTC1,					0,					0, false }, // Format::kBC4_UNorm,
	{ GL_COMPRESSED_SIGNED_RED_RGTC1,			0,					0, false }, // Format::kBC4_SNorm,
	{ GL_COMPRESSED_RG_RGTC2,					0,					0, false }, // Format::kBC5_Typeless,
	{ GL_COMPRESSED_RG_RGTC2,					0,					0, false }, // Format::kBC5_UNorm,
	{ GL_COMPRESSED_SIGNED_RG_RGTC2,			0,					0, false }, // Format::kBC5_SNorm,
	{ 0,										0,					0, false }, // Format::kB5G6R5_UNorm,
	{ 0,										0,					0, false }, // Format::kBGR5A1_UNorm,
	{ GL_RGBA8,									0,					0, false }, // Format::kBGRA8_UNorm,
	{ 0,										0,					0, false }, // Format::kBGRX8_UNorm,
	{ 0,										0,					0, false }, // Format::kRGB10_XR_BIAS_A2_UNorm,
	{ 0,										0,					0, false }, // Format::kBGRA8_Typeless,
	{ 0,										0,					0, false }, // Format::kBGRA8_UNorm_SRGB,
	{ 0,										0,					0, false }, // Format::kBGRX8_Typeless,
	{ 0,										0,					0, false }, // Format::kBGRX8_UNorm_SRGB,
	{ GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,	0,					0, false }, // Format::kBC6H_Typeless,
	{ GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,	0,					0, false }, // Format::kBC6H_UF16,
	{ GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,		0,					0, false }, // Format::kBC6H_SF16,
	{ GL_COMPRESSED_RGBA_BPTC_UNORM,			0,					0, false }, // Format::kBC7_Typeless,
	{ GL_COMPRESSED_RGBA_BPTC_UNORM,			0,					0, false }, // Format::kBC7_UNorm,
	{ GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,		0,					0, false }, // Format::kBC7_UNorm_SRGB,
	{ 0,										0,					0, false }, // Format::kAYUV,
	{ 0,										0,					0, false }, // Format::kY410,
	{ 0,										0,					0, false }, // Format::kY416,
	{ 0,										0,					0, false }, // Format::kNV12,
	{ 0,										0,					0, false }, // Format::kP010,
	{ 0,										0,					0, false }, // Format::kP016,
	{ 0,										0,					0, false }, // Format::k420_OPAQUE,
	{ 0,										0,					0, false }, // Format::kYUY2,
	{ 0,										0,					0, false }, // Format::kY210,
	{ 0,										0,					0, false }, // Format::kY216,
	{ 0,										0,					0, false }, // Format::kNV11,
	{ 0,										0,					0, false }, // Format::kAI44,
	{ 0,										0,					0, false }, // Format::kIA44,
	{ 0,										0,					0, false }, // Format::kP8,
	{ 0,										0,					0, false }, // Format::kA8P8,
	{ 0,										0,					0, false }, // Format::kBGRA4_UNorm,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kUnknown,
	{ 0,										0,					0, false }, // Format::kP208,
	{ 0,										0,					0, false }, // Format::kV208,
	{ 0,										0,					0, false }, // Format::kV408,
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
	else if ((usageFlags & BufferUsage::kConstantBuffer) == BufferUsage::kConstantBuffer)
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


GLenum GetGlBufferMemoryFlags(MemoryType memoryFlags)
{
	switch (memoryFlags)
	{
	case MemoryType::kDeviceLocal:		return 0;
	case MemoryType::kHostVisible:		return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;
	case MemoryType::kHostCoherent:	return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
	case MemoryType::kUpload:			return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
	case MemoryType::kHostCached:		return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
	case MemoryType::kReadBack:		return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;
	}

	return 0;
}


GLenum GetGlBufferMappingFlags(MemoryType memoryFlags)
{
	GLenum mappingFlags = GL_MAP_WRITE_BIT;
	if ((memoryFlags & MemoryType::kHostCached) == MemoryType::kHostCached)
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
				if (textureDesc.m_Size.depth == 1)
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
				if (textureDesc.m_Size.depth <= 6)
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
				if (textureDesc.m_Size.depth == 1)
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
				if (textureDesc.m_Size.depth <= 6)
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


FormatMapGl GetGlFormatMap(Format format)
{
	return kFormatMapGl[(u8)format];
}


GLenum GetGlFormat(Format format)
{
	return kFormatMapGl[(u8)format].format;
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
	static constexpr GLint kFilters[] =
	{
		GL_NEAREST,
		GL_LINEAR
	};

	return kFilters[(u8)filter];
}


GLint GetGlAddressMode(AddressMode addressMode)
{
	static constexpr GLint kAddressModes[] =
	{
		GL_REPEAT,
		GL_MIRRORED_REPEAT,
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_BORDER,
		GL_MIRROR_CLAMP_TO_EDGE
	};

	return kAddressModes[(u8)addressMode];
}


GLint GetGlCompareOp(CompareOp compareOp)
{
	static constexpr GLint kCompareOps[] =
	{
		GL_NEVER,
		GL_LESS,
		GL_EQUAL,
		GL_LEQUAL,
		GL_GREATER,
		GL_NOTEQUAL,
		GL_GEQUAL,
		GL_ALWAYS
	};

	return kCompareOps[(u8)compareOp];
}

GLenum GetGlFillMode(FillMode fillMode)
{
	static constexpr GLenum kFillModes[] =
	{
		GL_FILL,
		GL_LINE
	};

	return kFillModes[(u8)fillMode];
}

GLenum GetGlCullMode(CullMode cullMode)
{
	static constexpr GLenum kCullModes[] =
	{
		0,
		GL_FRONT,
		GL_BACK,
		GL_FRONT_AND_BACK
	};

	return kCullModes[(u8)cullMode];
}

GLenum GetGlFrontFace(FrontFace frontFace)
{
	static constexpr GLenum kFrontFaces[] =
	{
		GL_CW,
		GL_CCW
	};

	return kFrontFaces[(u8)frontFace];
}


GLenum GetGlStencilOp(StencilOp stencilOp)
{
	static constexpr GLenum kStencilOps[] =
	{
		GL_KEEP,
		GL_ZERO,
		GL_REPLACE,
		GL_INCR,
		GL_DECR,
		GL_INVERT,
		GL_INCR_WRAP,
		GL_DECR_WRAP,
	};

	return kStencilOps[(u8)stencilOp];
}


GLenum GetGlBlendOp(BlendOp blendOp)
{
	static constexpr GLenum kBlendOps[] =
	{
		GL_FUNC_ADD,
		GL_FUNC_SUBTRACT,
		GL_FUNC_REVERSE_SUBTRACT,
		GL_MIN,
		GL_MAX,
	};

	return kBlendOps[(u8)blendOp];
}


GLenum GetGlBlendFactor(BlendFactor blendFactor)
{
	static constexpr GLenum kBlendFactors[] =
	{
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_CONSTANT_COLOR,
		GL_ONE_MINUS_CONSTANT_COLOR,
		GL_CONSTANT_ALPHA,
		GL_ONE_MINUS_CONSTANT_ALPHA,
		GL_SRC_ALPHA_SATURATE,
		GL_SRC1_COLOR,
		GL_ONE_MINUS_SRC1_COLOR,
		GL_SRC1_ALPHA,
		GL_ONE_MINUS_SRC1_ALPHA,
	};

	return kBlendFactors[(u8)blendFactor];
}


GLbitfield GetGlResourceState(ResourceState state)
{
	constexpr GLbitfield bits[] =
	{
		0,
		GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT,
		GL_ELEMENT_ARRAY_BARRIER_BIT,
		GL_COMMAND_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT,
		GL_UNIFORM_BARRIER_BIT,
		GL_TEXTURE_FETCH_BARRIER_BIT,
		GL_TEXTURE_FETCH_BARRIER_BIT,
		GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT,
		0,
		GL_BUFFER_UPDATE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	return bits[u32(state)];
}