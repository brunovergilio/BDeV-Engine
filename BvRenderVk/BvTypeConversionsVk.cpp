#include "BvTypeConversionsVk.h"
#include "BDeV/Core/Container/BvRobinMap.h"


constexpr FormatMapVk kFormats[] =
{
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_R32G32B32A32_SFLOAT,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA32_Typeless,
	{ VK_FORMAT_R32G32B32A32_SFLOAT,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA32_Float,
	{ VK_FORMAT_R32G32B32A32_UINT,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA32_UInt,
	{ VK_FORMAT_R32G32B32A32_SINT,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA32_SInt,
	{ VK_FORMAT_R32G32B32_SFLOAT,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB32_Typeless,
	{ VK_FORMAT_R32G32B32_SFLOAT,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB32_Float,
	{ VK_FORMAT_R32G32B32_UINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB32_UInt,
	{ VK_FORMAT_R32G32B32_SINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB32_SInt,
	{ VK_FORMAT_R16G16B16A16_SFLOAT,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA16_Typeless,
	{ VK_FORMAT_R16G16B16A16_SFLOAT,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA16_Float,
	{ VK_FORMAT_R16G16B16A16_UNORM,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA16_UNorm,
	{ VK_FORMAT_R16G16B16A16_UINT,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA16_UInt,
	{ VK_FORMAT_R16G16B16A16_SNORM,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA16_SNorm,
	{ VK_FORMAT_R16G16B16A16_SINT,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA16_SInt,
	{ VK_FORMAT_R32G32_SFLOAT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG32_Typeless,
	{ VK_FORMAT_R32G32_SFLOAT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG32_Float,
	{ VK_FORMAT_R32G32_UINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG32_UInt,
	{ VK_FORMAT_R32G32_SINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG32_SInt,
	{ VK_FORMAT_D32_SFLOAT_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, {} },	// Format::kR32G8X24_Typeless,
	{ VK_FORMAT_D32_SFLOAT_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, {} },	// Format::kD32_Float_S8X24_UInt,
	{ VK_FORMAT_D32_SFLOAT_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT, {} },	// Format::kR32_Float_X8X24_Typeless,
	{ VK_FORMAT_D32_SFLOAT_S8_UINT,			VK_IMAGE_ASPECT_STENCIL_BIT, {} },	// Format::kX32_Typeless_G8X24_UInt,
	{ VK_FORMAT_A2B10G10R10_UNORM_PACK32,	VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB10A2_Typeless,
	{ VK_FORMAT_A2B10G10R10_UNORM_PACK32,	VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB10A2_UNorm,
	{ VK_FORMAT_A2B10G10R10_UINT_PACK32,	VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB10A2_UInt,
	{ VK_FORMAT_B10G11R11_UFLOAT_PACK32,	VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG11B10_Float,
	{ VK_FORMAT_R8G8B8A8_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA8_Typeless,
	{ VK_FORMAT_R8G8B8A8_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA8_UNorm,
	{ VK_FORMAT_R8G8B8A8_SRGB,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA8_UNorm_SRGB,
	{ VK_FORMAT_R8G8B8A8_UINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA8_UInt,
	{ VK_FORMAT_R8G8B8A8_SNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA8_SNorm,
	{ VK_FORMAT_R8G8B8A8_SINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGBA8_SInt,
	{ VK_FORMAT_R16G16_SFLOAT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG16_Typeless,
	{ VK_FORMAT_R16G16_SFLOAT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG16_Float,
	{ VK_FORMAT_R16G16_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG16_UNorm,
	{ VK_FORMAT_R16G16_UINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG16_UInt,
	{ VK_FORMAT_R16G16_SNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG16_SNorm,
	{ VK_FORMAT_R16G16_SINT,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG16_SInt,
	{ VK_FORMAT_R32_SFLOAT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR32_Typeless,
	{ VK_FORMAT_D32_SFLOAT,					VK_IMAGE_ASPECT_DEPTH_BIT, {} },	// Format::kD32_Float,
	{ VK_FORMAT_R32_SFLOAT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR32_Float,
	{ VK_FORMAT_R32_UINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR32_UInt,
	{ VK_FORMAT_R32_SINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR32_SInt,
	{ VK_FORMAT_D24_UNORM_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, {} },	// Format::kR24G8_Typeless,
	{ VK_FORMAT_D24_UNORM_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, {} },	// Format::kD24_UNorm_S8_UInt,
	{ VK_FORMAT_D24_UNORM_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT, {} },	// Format::kR24_UNorm_X8_Typeless,
	{ VK_FORMAT_D24_UNORM_S8_UINT,			VK_IMAGE_ASPECT_STENCIL_BIT, {} },	// Format::kX24_Typeless_G8_UInt,
	{ VK_FORMAT_R8G8_UNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG8_Typeless,
	{ VK_FORMAT_R8G8_UNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG8_UNorm,
	{ VK_FORMAT_R8G8_UINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG8_UInt,
	{ VK_FORMAT_R8G8_SNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG8_SNorm,
	{ VK_FORMAT_R8G8_SINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG8_SInt,
	{ VK_FORMAT_R16_SFLOAT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR16_Typeless,
	{ VK_FORMAT_R16_SFLOAT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR16_Float,
	{ VK_FORMAT_D16_UNORM,					VK_IMAGE_ASPECT_DEPTH_BIT, {} },	// Format::kD16_UNorm,
	{ VK_FORMAT_R16_UNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR16_UNorm,
	{ VK_FORMAT_R16_UINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR16_UInt,
	{ VK_FORMAT_R16_SNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR16_SNorm,
	{ VK_FORMAT_R16_SINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR16_SInt,
	{ VK_FORMAT_R8_UNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR8_Typeless,
	{ VK_FORMAT_R8_UNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR8_UNorm,
	{ VK_FORMAT_R8_UINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR8_UInt,
	{ VK_FORMAT_R8_SNORM,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR8_SNorm,
	{ VK_FORMAT_R8_SINT,					VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kR8_SInt,
	{ VK_FORMAT_R8_UNORM,					VK_IMAGE_ASPECT_COLOR_BIT, { VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_R } },	// Format::kA8_UNorm,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kR1_UNorm,
	{ VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRGB9E5_SHAREDEXP,
	{ VK_FORMAT_B8G8R8G8_422_UNORM,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kRG8_BG8_UNorm,
	{ VK_FORMAT_G8B8G8R8_422_UNORM,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kGR8_GB8_UNorm,
	{ VK_FORMAT_BC1_RGBA_UNORM_BLOCK,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC1_Typeless,
	{ VK_FORMAT_BC1_RGBA_UNORM_BLOCK,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC1_UNorm,
	{ VK_FORMAT_BC1_RGBA_SRGB_BLOCK,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC1_UNorm_SRGB,
	{ VK_FORMAT_BC2_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC2_Typeless,
	{ VK_FORMAT_BC2_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC2_UNorm,
	{ VK_FORMAT_BC2_SRGB_BLOCK,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC2_UNorm_SRGB,
	{ VK_FORMAT_BC3_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC3_Typeless,
	{ VK_FORMAT_BC3_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC3_UNorm,
	{ VK_FORMAT_BC3_SRGB_BLOCK,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC3_UNorm_SRGB,
	{ VK_FORMAT_BC4_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC4_Typeless,
	{ VK_FORMAT_BC4_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC4_UNorm,
	{ VK_FORMAT_BC4_SNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC4_SNorm,
	{ VK_FORMAT_BC5_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC5_Typeless,
	{ VK_FORMAT_BC5_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC5_UNorm,
	{ VK_FORMAT_BC5_SNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC5_SNorm,
	{ VK_FORMAT_R5G6B5_UNORM_PACK16,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kB5G6R5_UNorm,
	{ VK_FORMAT_A1R5G5B5_UNORM_PACK16,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBGR5A1_UNorm,
	{ VK_FORMAT_B8G8R8A8_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBGRA8_UNorm,
	{ VK_FORMAT_B8G8R8A8_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE } },	// Format::kBGRX8_UNorm,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kRGB10_XR_BIAS_A2_UNorm,
	{ VK_FORMAT_B8G8R8A8_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBGRA8_Typeless,
	{ VK_FORMAT_B8G8R8A8_SRGB,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBGRA8_UNorm_SRGB,
	{ VK_FORMAT_B8G8R8A8_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBGRX8_Typeless,
	{ VK_FORMAT_B8G8R8A8_SRGB,				VK_IMAGE_ASPECT_COLOR_BIT, { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE } },	// Format::kBGRX8_UNorm_SRGB,
	{ VK_FORMAT_BC6H_UFLOAT_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC6H_Typeless,
	{ VK_FORMAT_BC6H_UFLOAT_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC6H_UF16,
	{ VK_FORMAT_BC6H_SFLOAT_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC6H_SF16,
	{ VK_FORMAT_BC7_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC7_Typeless,
	{ VK_FORMAT_BC7_UNORM_BLOCK,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC7_UNorm,
	{ VK_FORMAT_BC7_SRGB_BLOCK,				VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBC7_UNorm_SRGB,
	{ VK_FORMAT_R8G8B8A8_UNORM,				VK_IMAGE_ASPECT_COLOR_BIT, { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ONE } },	// Format::kAYUV,
	{ VK_FORMAT_A2B10G10R10_UNORM_PACK32,	VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kY410,
	{ VK_FORMAT_R16G16B16A16_UNORM,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kY416,
	{ VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,	VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT, {} },	// Format::kNV12,
	{ VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT, {} },	// Format::kP010,
	{ VK_FORMAT_G16_B16R16_2PLANE_420_UNORM, VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT, {} },	// Format::kP016,
	{ VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,	VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT, {} },	// Format::k420_OPAQUE,
	{ VK_FORMAT_G8B8G8R8_422_UNORM,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kYUY2,
	{ VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kY210,
	{ VK_FORMAT_G16B16G16R16_422_UNORM,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kY216,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kNV11,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kAI44,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kIA44,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kP8,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kA8P8,
	{ VK_FORMAT_A4R4G4B4_UNORM_PACK16,		VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kBGRA4_UNorm,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kUnknown,
	{ VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,	VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT, {} },	// Format::kP208,
	{ VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,	VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT, {} },	// Format::kV208,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kV408,
};


constexpr VkImageType kImageTypes[] =
{
	VK_IMAGE_TYPE_1D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_3D,
};


constexpr VkImageViewType kImageViewTypes[] =
{
	VK_IMAGE_VIEW_TYPE_1D,
	VK_IMAGE_VIEW_TYPE_1D_ARRAY,
	VK_IMAGE_VIEW_TYPE_2D,
	VK_IMAGE_VIEW_TYPE_2D_ARRAY,
	VK_IMAGE_VIEW_TYPE_CUBE,
	VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
	VK_IMAGE_VIEW_TYPE_3D,
};


constexpr VkFilter kFilters[] =
{
	VK_FILTER_NEAREST,
	VK_FILTER_LINEAR,
};


constexpr VkSamplerMipmapMode kMipMapModes[] =
{
	VK_SAMPLER_MIPMAP_MODE_NEAREST,
	VK_SAMPLER_MIPMAP_MODE_LINEAR,
};


constexpr VkSamplerAddressMode kSamplerAddressModes[] =
{
	VK_SAMPLER_ADDRESS_MODE_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};


constexpr VkCompareOp kCompareOps[] =
{
	VK_COMPARE_OP_NEVER,
	VK_COMPARE_OP_LESS,
	VK_COMPARE_OP_EQUAL,
	VK_COMPARE_OP_LESS_OR_EQUAL,
	VK_COMPARE_OP_GREATER,
	VK_COMPARE_OP_NOT_EQUAL,
	VK_COMPARE_OP_GREATER_OR_EQUAL,
	VK_COMPARE_OP_ALWAYS,
};


constexpr VkVertexInputRate kVertexInputRates[] =
{
	VK_VERTEX_INPUT_RATE_VERTEX,
	VK_VERTEX_INPUT_RATE_INSTANCE,
};


VkImageType GetVkImageType(const TextureType type)
{
	return kImageTypes[u32(type)];
}


VkImageViewType GetVkImageViewType(const TextureViewType viewType)
{
	return kImageViewTypes[u32(viewType)];
}


const FormatMapVk& GetVkFormatMap(Format format)
{
	return kFormats[(u8)format];
}

VkImageAspectFlags GetVkImageAspectFlags(Format format, u32 planeSlice)
{
	auto info = GetVkFormatMap(format);
	if (info.aspectFlags & VK_IMAGE_ASPECT_PLANE_0_BIT)
	{
		switch (planeSlice)
		{
		case 0: return VK_IMAGE_ASPECT_PLANE_0_BIT;
		case 1: return VK_IMAGE_ASPECT_PLANE_1_BIT;
		case 2: return VK_IMAGE_ASPECT_PLANE_2_BIT;
		default:
			break;
		}
	}
	
	return info.aspectFlags;
}


VkFormat GetVkFormat(const Format format)
{
	return kFormats[(u8)format].format;
}


VkBufferUsageFlags GetVkBufferUsageFlags(const BufferUsage usageFlags, bool formatted)
{
	VkBufferUsageFlags bufferUsageFlags = 0;
	if (EHasFlag(usageFlags, BufferUsage::kConstantBuffer		))	{ bufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kStructuredBuffer		))	{ bufferUsageFlags |= (!formatted ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT); }
	if (EHasFlag(usageFlags, BufferUsage::kRWStructuredBuffer	))	{ bufferUsageFlags |= (!formatted ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT); }
	if (EHasFlag(usageFlags, BufferUsage::kVertexBuffer			))	{ bufferUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kIndexBuffer			))	{ bufferUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kIndirectBuffer		))	{ bufferUsageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kRayTracing			))
	{
		bufferUsageFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR
			| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	return bufferUsageFlags | (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}


VkImageUsageFlags GetVkImageUsageFlags(const TextureUsage usageFlags)
{
	VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (EHasFlag(usageFlags, TextureUsage::kRenderTarget		))	{ imageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
	if (EHasFlag(usageFlags, TextureUsage::kDepthStencilTarget	))	{ imageUsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }
	if (EHasFlag(usageFlags, TextureUsage::kShaderResource		))	{ imageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT; }
	if (EHasFlag(usageFlags, TextureUsage::kUnorderedAccess		))	{ imageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT; }
	if (EHasFlag(usageFlags, TextureUsage::kInputAttachment		))	{ imageUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT; }
	if (EHasFlag(usageFlags, TextureUsage::kShadingRate			))	{ imageUsageFlags |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR; }

	return imageUsageFlags;
}


VkMemoryPropertyFlags GetVkMemoryPropertyFlags(const MemoryType memoryFlags)
{
	switch (memoryFlags)
	{
	case MemoryType::kDevice: return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	case MemoryType::kUpload: return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	case MemoryType::kReadBack: return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	case MemoryType::kShared: return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	default: return 0;
	}
}


VkMemoryPropertyFlags GetPreferredVkMemoryPropertyFlags(const MemoryType memoryFlags)
{
	switch (memoryFlags)
	{
	case MemoryType::kUpload: return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	case MemoryType::kReadBack: return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	case MemoryType::kShared: return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	case MemoryType::kReadBackNC: return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	case MemoryType::kUploadNC:
	case MemoryType::kDevice:
	default: return 0;
	}
}


VkFilter GetVkFilter(const Filter filter)
{
	return kFilters[u32(filter)];
}


VkSamplerMipmapMode GetVkSamplerMipmapMode(const MipMapFilter mipMapFilter)
{
	return kMipMapModes[u32(mipMapFilter)];
}


VkSamplerAddressMode GetVkSamplerAddressMode(const AddressMode addressMode)
{
	return kSamplerAddressModes[u32(addressMode)];
}


VkCompareOp GetVkCompareOp(const CompareOp compareOp)
{
	return kCompareOps[u32(compareOp)];
}


VkSamplerReductionMode GetVkSamplerReductionMode(ReductionMode reductionMode)
{
	switch (reductionMode)
	{
	case ReductionMode::kMin: return VK_SAMPLER_REDUCTION_MODE_MIN;
	case ReductionMode::kMax: return VK_SAMPLER_REDUCTION_MODE_MAX;
	default: return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
	}
}


VkVertexInputRate GetVkVertexInputRate(const InputRate inputRate)
{
	return kVertexInputRates[u32(inputRate)];
}


VkPrimitiveTopology GetVkPrimitiveTopology(const Topology topology)
{
	switch (topology)
	{
	case Topology::kPointList:			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case Topology::kLineList:			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case Topology::kLineStrip:			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case Topology::kTriangleList:		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case Topology::kTriangleStrip:		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case Topology::kLineListAdj:		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case Topology::kLineStripAdj:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
	case Topology::kTriangleListAdj:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
	case Topology::kTriangleStripAdj:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
	case Topology::kPatchList:			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}

	return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}


VkPolygonMode GetVkPolygonMode(const FillMode fillMode)
{
	switch (fillMode)
	{
	case FillMode::kSolid:		return VK_POLYGON_MODE_FILL;
	case FillMode::kWireframe:	return VK_POLYGON_MODE_LINE;
	}

	return VK_POLYGON_MODE_FILL;
}


VkCullModeFlags GetVkCullModeFlags(const CullMode cullMode)
{
	switch (cullMode)
	{
	case CullMode::kNone:			return VK_CULL_MODE_NONE;
	case CullMode::kFront:			return VK_CULL_MODE_FRONT_BIT;
	case CullMode::kBack:			return VK_CULL_MODE_BACK_BIT;
	case CullMode::kFrontAndBack:	return VK_CULL_MODE_FRONT_AND_BACK;
	}

	return VK_CULL_MODE_NONE;
}


VkFrontFace GetVkFrontFace(const FrontFace frontFace)
{
	switch (frontFace)
	{
	case FrontFace::kClockwise:			return VK_FRONT_FACE_CLOCKWISE;
	case FrontFace::kCounterClockwise:	return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	return VK_FRONT_FACE_CLOCKWISE;
}


VkStencilOp GetVkStencilOp(const StencilOp stencilOp)
{
	switch (stencilOp)
	{
	case StencilOp::kKeep:		return VK_STENCIL_OP_KEEP;
	case StencilOp::kZero:		return VK_STENCIL_OP_ZERO;
	case StencilOp::kReplace:	return VK_STENCIL_OP_REPLACE;
	case StencilOp::kIncrSat:	return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case StencilOp::kDecrSat:	return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case StencilOp::kInvert:	return VK_STENCIL_OP_INVERT;
	case StencilOp::kIncrWrap:	return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case StencilOp::kDecrWrap:	return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}

	return VK_STENCIL_OP_KEEP;
}


VkSampleCountFlagBits GetVkSampleCountFlagBits(const u8 sampleCount)
{
	BV_ASSERT((sampleCount & (sampleCount - 1)) == 0, "Sample count must be a power of 2");
	switch (sampleCount)
	{
	case 1:		return VK_SAMPLE_COUNT_1_BIT;
	case 2:		return VK_SAMPLE_COUNT_2_BIT;
	case 4:		return VK_SAMPLE_COUNT_4_BIT;
	case 8:		return VK_SAMPLE_COUNT_8_BIT;
	case 16:	return VK_SAMPLE_COUNT_16_BIT;
	case 32:	return VK_SAMPLE_COUNT_32_BIT;
	case 64:	return VK_SAMPLE_COUNT_64_BIT;
	}

	return VK_SAMPLE_COUNT_1_BIT;
}


VkBlendOp GetVkBlendOp(const BlendOp blendOp)
{
	switch (blendOp)
	{
	case BlendOp::kAdd:			return VK_BLEND_OP_ADD;
	case BlendOp::kSubtract:	return VK_BLEND_OP_SUBTRACT;
	case BlendOp::kRevSubtract:	return VK_BLEND_OP_REVERSE_SUBTRACT;
	case BlendOp::kMin:			return VK_BLEND_OP_MIN;
	case BlendOp::kMax:			return VK_BLEND_OP_MAX;
	}

	return VK_BLEND_OP_ADD;
}


VkBlendFactor GetVkBlendFactor(const BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case BlendFactor::kZero:			return VK_BLEND_FACTOR_ZERO;
	case BlendFactor::kOne:				return VK_BLEND_FACTOR_ONE;
	case BlendFactor::kSrcColor:		return VK_BLEND_FACTOR_SRC_COLOR;
	case BlendFactor::kInvSrcColor:		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case BlendFactor::kDstColor:		return VK_BLEND_FACTOR_DST_COLOR;
	case BlendFactor::kInvDstColor:		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case BlendFactor::kSrcAlpha:		return VK_BLEND_FACTOR_SRC_ALPHA;
	case BlendFactor::kInvkSrcAlpha:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::kDstAlpha:		return VK_BLEND_FACTOR_DST_ALPHA;
	case BlendFactor::kInvDstAlpha:		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case BlendFactor::kBlendFactor:		return VK_BLEND_FACTOR_CONSTANT_COLOR;
	case BlendFactor::kInvBlendFactor:	return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	case BlendFactor::kAlphaFactor:		return VK_BLEND_FACTOR_CONSTANT_ALPHA;
	case BlendFactor::kInvAlphaFactor:	return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
	case BlendFactor::kSrcAlphaSat:		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	case BlendFactor::kSrc1Color:		return VK_BLEND_FACTOR_SRC1_COLOR;
	case BlendFactor::kInvSrc1Color:	return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
	case BlendFactor::kSrc1Alpha:		return VK_BLEND_FACTOR_SRC1_ALPHA;
	case BlendFactor::kInvkSrc1Alpha:	return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}

	return VK_BLEND_FACTOR_ZERO;
}


VkLogicOp GetVkLogicOp(const LogicOp logicOp)
{
	switch (logicOp)
	{
	case LogicOp::kClear:			return VK_LOGIC_OP_CLEAR;
	case LogicOp::kSet:				return VK_LOGIC_OP_SET;
	case LogicOp::kCopy:			return VK_LOGIC_OP_COPY;
	case LogicOp::kCopyInverted:	return VK_LOGIC_OP_COPY_INVERTED;
	case LogicOp::kNoOp:			return VK_LOGIC_OP_NO_OP;
	case LogicOp::kInvert:			return VK_LOGIC_OP_INVERT;
	case LogicOp::kAnd:				return VK_LOGIC_OP_AND;
	case LogicOp::kNand:			return VK_LOGIC_OP_NAND;
	case LogicOp::kOr:				return VK_LOGIC_OP_OR;
	case LogicOp::kNor:				return VK_LOGIC_OP_NOR;
	case LogicOp::kXor:				return VK_LOGIC_OP_XOR;
	case LogicOp::kEquiv:			return VK_LOGIC_OP_EQUIVALENT;
	case LogicOp::kAndReverse:		return VK_LOGIC_OP_AND_REVERSE;
	case LogicOp::kAndInverted:		return VK_LOGIC_OP_AND_INVERTED;
	case LogicOp::kOrReverse:		return VK_LOGIC_OP_OR_REVERSE;
	case LogicOp::kOrInverted:		return VK_LOGIC_OP_OR_INVERTED;
	}

	return VkLogicOp::VK_LOGIC_OP_CLEAR;
}


VkShaderStageFlagBits GetVkShaderStageFlagBits(const ShaderStage shaderStage)
{
	switch (shaderStage)
	{
	case ShaderStage::kVertex:				return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderStage::kHullOrControl:		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderStage::kDomainOrEvaluation:	return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ShaderStage::kGeometry:			return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderStage::kPixelOrFragment:		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderStage::kCompute:				return VK_SHADER_STAGE_COMPUTE_BIT;
	case ShaderStage::kMesh:				return VK_SHADER_STAGE_MESH_BIT_EXT;
	case ShaderStage::kAmplificationOrTask:	return VK_SHADER_STAGE_TASK_BIT_EXT;
	case ShaderStage::kRayGen:				return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	case ShaderStage::kAnyHit:				return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	case ShaderStage::kClosestHit:			return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	case ShaderStage::kMiss:				return VK_SHADER_STAGE_MISS_BIT_KHR;
	case ShaderStage::kIntersection:		return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	case ShaderStage::kCallable:			return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
	}

	return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}


VkDescriptorType GetVkDescriptorType(const ShaderResourceType resourceType)
{
	switch (resourceType)
	{
	case ShaderResourceType::kConstantBuffer:			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case ShaderResourceType::kStructuredBuffer:			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case ShaderResourceType::kRWStructuredBuffer:		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case ShaderResourceType::kDynamicConstantBuffer:	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case ShaderResourceType::kDynamicStructuredBuffer:	return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	case ShaderResourceType::kDynamicRWStructuredBuffer:return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	case ShaderResourceType::kFormattedBuffer:			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case ShaderResourceType::kRWFormattedBuffer:		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case ShaderResourceType::kTexture:					return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case ShaderResourceType::kRWTexture:				return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case ShaderResourceType::kSampler:					return VK_DESCRIPTOR_TYPE_SAMPLER;
	case ShaderResourceType::kInputAttachment:			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	case ShaderResourceType::kAccelerationStructure:	return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	}

	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}


VkShaderStageFlags GetVkShaderStageFlags(const ShaderStage stages)
{
	VkShaderStageFlags shaderStages = 0;
	if (EHasFlag(stages, ShaderStage::kVertex				)) { shaderStages |= VK_SHADER_STAGE_VERTEX_BIT;					}
	if (EHasFlag(stages, ShaderStage::kHullOrControl		)) { shaderStages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;		}
	if (EHasFlag(stages, ShaderStage::kDomainOrEvaluation	)) { shaderStages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;	}
	if (EHasFlag(stages, ShaderStage::kGeometry				)) { shaderStages |= VK_SHADER_STAGE_GEOMETRY_BIT;					}
	if (EHasFlag(stages, ShaderStage::kPixelOrFragment		)) { shaderStages |= VK_SHADER_STAGE_FRAGMENT_BIT;					}
	if (EHasFlag(stages, ShaderStage::kCompute				)) { shaderStages |= VK_SHADER_STAGE_COMPUTE_BIT;					}
	if (EHasFlag(stages, ShaderStage::kMesh					)) { shaderStages |= VK_SHADER_STAGE_MESH_BIT_EXT;					}
	if (EHasFlag(stages, ShaderStage::kAmplificationOrTask	)) { shaderStages |= VK_SHADER_STAGE_TASK_BIT_EXT;					}
	if (EHasFlag(stages, ShaderStage::kRayGen				)) { shaderStages |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;				}
	if (EHasFlag(stages, ShaderStage::kAnyHit				)) { shaderStages |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;				}
	if (EHasFlag(stages, ShaderStage::kClosestHit			)) { shaderStages |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;			}
	if (EHasFlag(stages, ShaderStage::kIntersection			)) { shaderStages |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;			}
	if (EHasFlag(stages, ShaderStage::kMiss					)) { shaderStages |= VK_SHADER_STAGE_MISS_BIT_KHR;					}
	if (EHasFlag(stages, ShaderStage::kCallable				)) { shaderStages |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;				}

	return shaderStages;
}


VkAttachmentLoadOp GetVkAttachmentLoadOp(const LoadOp loadOp)
{
	switch (loadOp)
	{
	case LoadOp::kClear:	return VK_ATTACHMENT_LOAD_OP_CLEAR;
	case LoadOp::kLoad:		return VK_ATTACHMENT_LOAD_OP_LOAD;
	}

	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}


VkAttachmentStoreOp GetVkAttachmentStoreOp(const StoreOp storeOp)
{
	switch (storeOp)
	{
	case StoreOp::kStore:	return VK_ATTACHMENT_STORE_OP_STORE;
	}

	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}


VkIndexType GetVkIndexType(const IndexFormat indexFormat)
{
	switch (indexFormat)
	{
	case IndexFormat::kU16: return VK_INDEX_TYPE_UINT16;
	case IndexFormat::kU32:	return VK_INDEX_TYPE_UINT32;
	}

	return VK_INDEX_TYPE_NONE_KHR;
}


VkImageLayout GetVkImageLayout(const ResourceState resourceState, bool isDepthStencilFormat, bool isResolveRender)
{
	switch (resourceState)
	{
	case ResourceState::kTransferSrc:		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ResourceState::kTransferDst:		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case ResourceState::kPixelShaderResource:
	case ResourceState::kShaderResource:	return !isDepthStencilFormat ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case ResourceState::kRWResource:		return VK_IMAGE_LAYOUT_GENERAL;
	case ResourceState::kRenderTarget:		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ResourceState::kDepthStencilRead:	return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case ResourceState::kDepthStencilWrite:	return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case ResourceState::kPresent:			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	case ResourceState::kShadingRate:		return VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
	case ResourceState::kResolveSrc:		return isResolveRender ? VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ResourceState::kResolveDst:		return isResolveRender ? VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}

	return VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
}


VkAccessFlags2 GetVkAccessFlags(const ResourceAccess resourceAccess)
{
	VkAccessFlags2 accessFlags = 0;

	if (EHasFlag(resourceAccess, ResourceAccess::kIndirectRead			))	{ accessFlags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kIndexRead				))	{ accessFlags |= VK_ACCESS_2_INDEX_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kVertexInputRead		))	{ accessFlags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kUniformRead			))	{ accessFlags |= VK_ACCESS_2_UNIFORM_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kShaderRead			))	{ accessFlags |= VK_ACCESS_2_SHADER_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kShaderWrite			))	{ accessFlags |= VK_ACCESS_2_SHADER_WRITE_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kRenderTargetRead		))	{ accessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kRenderTargetWrite		))	{ accessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kDepthStencilRead		))	{ accessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kDepthStencilWrite		))	{ accessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kTransferRead			))	{ accessFlags |= VK_ACCESS_2_TRANSFER_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kTransferWrite			))	{ accessFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kHostRead				))	{ accessFlags |= VK_ACCESS_2_HOST_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kHostWrite				))	{ accessFlags |= VK_ACCESS_2_HOST_WRITE_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kMemoryRead			))	{ accessFlags |= VK_ACCESS_2_MEMORY_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kMemoryWrite			))	{ accessFlags |= VK_ACCESS_2_MEMORY_WRITE_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kInputAttachmentRead	))	{ accessFlags |= VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kShadingRateRead		))	{ accessFlags |= VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR; }
	if (EHasFlag(resourceAccess, ResourceAccess::kPredicationRead		))	{ accessFlags |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT; }
	if (EHasFlag(resourceAccess, ResourceAccess::kAccelerationStructureRead)) { accessFlags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR; }
	if (EHasFlag(resourceAccess, ResourceAccess::kAccelerationStructureWrite)) { accessFlags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR; }
	if (EHasFlag(resourceAccess, ResourceAccess::kShaderBindingTableRead)) { accessFlags |= VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR; }

	return accessFlags;
}


VkAccessFlags2 GetVkAccessFlags(const ResourceState resourceState)
{
	switch (resourceState)
	{
	case ResourceState::kVertexBuffer:		return VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
	case ResourceState::kIndexBuffer:		return VK_ACCESS_2_INDEX_READ_BIT;
	case ResourceState::kConstantBuffer:	return VK_ACCESS_2_UNIFORM_READ_BIT;
	case ResourceState::kIndirectBuffer:	return VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
	case ResourceState::kShaderResource:	return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	case ResourceState::kPixelShaderResource:	return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	case ResourceState::kRWResource:		return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
	case ResourceState::kTransferSrc:		return VK_ACCESS_2_TRANSFER_READ_BIT;
	case ResourceState::kTransferDst:		return VK_ACCESS_2_TRANSFER_WRITE_BIT;
	case ResourceState::kRenderTarget:		return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	case ResourceState::kDepthStencilRead:	return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	case ResourceState::kDepthStencilWrite:	return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case ResourceState::kPresent:			return VK_ACCESS_2_MEMORY_READ_BIT;
	case ResourceState::kShadingRate:		return VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
	case ResourceState::kASBuildRead:		return VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	case ResourceState::kASBuildWrite:		return VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	case ResourceState::kPredication:		return VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
	}

	return 0;
}


VkAccessFlags2 GetVkAccessFlags(BufferUsage usageFlags, bool includeTransfer)
{
	VkAccessFlags2 flags = includeTransfer ? (VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT) : 0;
	if (EHasFlag(usageFlags, BufferUsage::kConstantBuffer		)) { flags |= VK_ACCESS_2_UNIFORM_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kStructuredBuffer		)) { flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kRWStructuredBuffer	)) { flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kVertexBuffer			)) { flags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kIndexBuffer			)) { flags |= VK_ACCESS_2_INDEX_READ_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kIndirectBuffer		)) { flags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT; }
	if (EHasFlag(usageFlags, BufferUsage::kRayTracing			)) { flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR
		| VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR; }

	return flags;
}


VkPipelineStageFlags2 GetVkPipelineStageFlags(const VkAccessFlags2 accessFlags)
{
	if (accessFlags == 0)
	{
		return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
	}

	VkPipelineStageFlags2 stageFlags = 0;
	if (accessFlags & VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT)
	{
		stageFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_INDEX_READ_BIT | VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_UNIFORM_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT
			| VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT
			| VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT
			| VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT
			| VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
			| VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT
			| VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT
			| VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
	}
	if (accessFlags & (VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
			| VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_HOST_READ_BIT | VK_ACCESS_2_HOST_WRITE_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_HOST_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_MEMORY_READ_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	}
	if (accessFlags & (VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT;
	}
	if (accessFlags & (VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
	}
	if (accessFlags & (VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR))
	{
		stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
	}

	return stageFlags;
}


VkPipelineStageFlags2 GetVkPipelineStageFlags(const PipelineStage pipelineStage)
{
	VkPipelineStageFlags2 stageFlags = 0;

	if (EHasFlag(pipelineStage, PipelineStage::kBeginning				)) { stageFlags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kIndirectDraw			)) { stageFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kVertexInput				)) { stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kVertexShader			)) { stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kTessHullOrControlShader	)) { stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kTessDomainOrEvalShader	)) { stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kGeometryShader			)) { stageFlags |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kPixelOrFragmentShader	)) { stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kDepth					)) { stageFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kRenderTarget			)) { stageFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kComputeShader			)) { stageFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kTransfer				)) { stageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kEnd						)) { stageFlags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; }
	if (EHasFlag(pipelineStage, PipelineStage::kPredication				)) { stageFlags |= VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT; }
	if (EHasFlag(pipelineStage, PipelineStage::kShadingRate				)) { stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR; }
	if (EHasFlag(pipelineStage, PipelineStage::kMesh					)) { stageFlags |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT; }
	if (EHasFlag(pipelineStage, PipelineStage::kAmplificationOrTask		)) { stageFlags |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT; }
	if (EHasFlag(pipelineStage, PipelineStage::kAccelerationStructureBuild)) { stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR; }
	if (EHasFlag(pipelineStage, PipelineStage::kAccelerationStructureCopy)) { stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR; }
	if (EHasFlag(pipelineStage, PipelineStage::kRayTracing				)) { stageFlags |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR; }

	return stageFlags;
}


VkQueryType GetVkQueryType(QueryType queryHeapType)
{
	switch (queryHeapType)
	{
	case QueryType::kTimestamp: return VK_QUERY_TYPE_TIMESTAMP;
	case QueryType::kOcclusion:
	case QueryType::kOcclusionBinary: return VK_QUERY_TYPE_OCCLUSION;
	case QueryType::kPipelineStatistics:
	case QueryType::kMeshPipelineStatistics: return VK_QUERY_TYPE_PIPELINE_STATISTICS;
	}

	return VK_QUERY_TYPE_TIMESTAMP;
}


VkResolveModeFlagBits GetVkResolveMode(ResolveMode resolveMode)
{
	switch (resolveMode)
	{
	case ResolveMode::kAverage: return VK_RESOLVE_MODE_AVERAGE_BIT;
	case ResolveMode::kMin: return VK_RESOLVE_MODE_MIN_BIT;
	case ResolveMode::kMax: return VK_RESOLVE_MODE_MAX_BIT;
	default: return VK_RESOLVE_MODE_NONE;
	}
}


VkFragmentShadingRateCombinerOpKHR GetVkShadingRateCombinerOp(ShadingRateCombinerOp op)
{
	switch (op)
	{
	case ShadingRateCombinerOp::kKeep:		return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
	case ShadingRateCombinerOp::kReplace:	return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
	case ShadingRateCombinerOp::kMin:		return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR;
	case ShadingRateCombinerOp::kMax:		return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR;
	default:								return VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
	}
}


VkBuildAccelerationStructureFlagsKHR GetVkBuildAccelerationStructureFlags(RayTracingAccelerationStructureFlags flags)
{
	VkBuildAccelerationStructureFlagsKHR buildFlags = 0;
	if (EHasFlag(flags, RayTracingAccelerationStructureFlags::kAllowUpdate))		{ buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR; }
	if (EHasFlag(flags, RayTracingAccelerationStructureFlags::kAllowCompaction))	{ buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR; }
	if (EHasFlag(flags, RayTracingAccelerationStructureFlags::kPreferFastTrace))	{ buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR; }
	if (EHasFlag(flags, RayTracingAccelerationStructureFlags::kPreferFastBuild))	{ buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR; }
	if (EHasFlag(flags, RayTracingAccelerationStructureFlags::kLowMemory))			{ buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR; }

	return buildFlags;
}


VkRayTracingShaderGroupTypeKHR GetVkRayTracingShaderGroupType(ShaderGroupType type)
{
	switch (type)
	{
	case ShaderGroupType::kTriangles: return VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	case ShaderGroupType::kProcedural: return VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
	default: return VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	}
}


VkGeometryTypeKHR GetVkGeometryType(RayTracingGeometryType type)
{
	switch (type)
	{
	case RayTracingGeometryType::kTriangles: return VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	case RayTracingGeometryType::kAABB: return VK_GEOMETRY_TYPE_AABBS_KHR;
	}

	return VK_GEOMETRY_TYPE_MAX_ENUM_KHR;
}


VkGeometryFlagsKHR GetVkGeometryFlags(RayTracingGeometryFlags flags)
{
	VkGeometryFlagsKHR result = 0;
	if (EHasFlag(flags, RayTracingGeometryFlags::kOpaque)) { result |= VK_GEOMETRY_OPAQUE_BIT_KHR; }
	if (EHasFlag(flags, RayTracingGeometryFlags::kNoDuplicateAnyHitInvocation)) { result |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR; }

	return result;
}


VkGeometryInstanceFlagsKHR GetVkGeometryInstanceFlags(RayTracingInstanceFlags flags)
{
	VkGeometryInstanceFlagsKHR result = 0;
	if (EHasFlag(flags, RayTracingInstanceFlags::kTriangleCullDisable)) { result |= VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; }
	if (EHasFlag(flags, RayTracingInstanceFlags::kTriangleFrontCounterclockwise)) { result |= VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR; }
	if (EHasFlag(flags, RayTracingInstanceFlags::kForceOpaque)) { result |= VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR; }
	if (EHasFlag(flags, RayTracingInstanceFlags::kForceNonOpaque)) { result |= VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR; }

	return result;
}