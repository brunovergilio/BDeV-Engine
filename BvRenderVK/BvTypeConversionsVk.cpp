#include "BvTypeConversionsVk.h"
#include "BDeV/Container/BvRobinMap.h"


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
	{ VK_FORMAT_D32_SFLOAT_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,	{} },	// Format::kR32G8X24_Typeless,
	{ VK_FORMAT_D32_SFLOAT_S8_UINT,			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,	{} },	// Format::kD32_Float_S8X24_UInt,
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
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kY410,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kY416,
	{ VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,	VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kNV12,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kP010,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kP016,
	{ VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,	VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::k420_OPAQUE,
	{ VK_FORMAT_G8B8G8R8_422_UNORM,			VK_IMAGE_ASPECT_COLOR_BIT, {} },	// Format::kYUY2,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kY210,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kY216,
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
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kP208,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kV208,
	{ VK_FORMAT_UNDEFINED,					0, {} },	// Format::kV408,
};


static BvRobinMap<VkFormat, Format> s_VkFormatToFormatMap =
{
	{ VK_FORMAT_UNDEFINED,					Format::kUnknown },
	{ VK_FORMAT_R32G32B32A32_SFLOAT,		Format::kRGBA32_Float },
	{ VK_FORMAT_R32G32B32A32_UINT,			Format::kRGBA32_UInt },
	{ VK_FORMAT_R32G32B32A32_SINT,			Format::kRGBA32_SInt },
	{ VK_FORMAT_R32G32B32_SFLOAT,			Format::kRGB32_Float },
	{ VK_FORMAT_R32G32B32_UINT,				Format::kRGB32_UInt },
	{ VK_FORMAT_R32G32B32_SINT,				Format::kRGB32_SInt },
	{ VK_FORMAT_R16G16B16A16_SFLOAT,		Format::kRGBA16_Float },
	{ VK_FORMAT_R16G16B16A16_UNORM,			Format::kRGBA16_UNorm },
	{ VK_FORMAT_R16G16B16A16_UINT,			Format::kRGBA16_UInt },
	{ VK_FORMAT_R16G16B16A16_SNORM,			Format::kRGBA16_SNorm },
	{ VK_FORMAT_R16G16B16A16_SINT,			Format::kRGBA16_SInt },
	{ VK_FORMAT_R32G32_SFLOAT,				Format::kRG32_Float },
	{ VK_FORMAT_R32G32_UINT,				Format::kRG32_UInt },
	{ VK_FORMAT_R32G32_SINT,				Format::kRG32_SInt },
	{ VK_FORMAT_D32_SFLOAT_S8_UINT,			Format::kD32_Float_S8X24_UInt },
	{ VK_FORMAT_A2B10G10R10_UNORM_PACK32,	Format::kRGB10A2_UNorm },
	{ VK_FORMAT_A2B10G10R10_UINT_PACK32,	Format::kRGB10A2_UInt },
	{ VK_FORMAT_B10G11R11_UFLOAT_PACK32,	Format::kRG11B10_Float },
	{ VK_FORMAT_R8G8B8A8_UNORM,				Format::kRGBA8_UNorm },
	{ VK_FORMAT_R8G8B8A8_SRGB,				Format::kRGBA8_UNorm_SRGB },
	{ VK_FORMAT_R8G8B8A8_UINT,				Format::kRGBA8_UInt },
	{ VK_FORMAT_R8G8B8A8_SNORM,				Format::kRGBA8_SNorm },
	{ VK_FORMAT_R8G8B8A8_SINT,				Format::kRGBA8_SInt },
	{ VK_FORMAT_R16G16_SFLOAT,				Format::kRG16_Float },
	{ VK_FORMAT_R16G16_UNORM,				Format::kRG16_UNorm },
	{ VK_FORMAT_R16G16_UINT,				Format::kRG16_UInt },
	{ VK_FORMAT_R16G16_SNORM,				Format::kRG16_SNorm },
	{ VK_FORMAT_R16G16_SINT,				Format::kRG16_SInt },
	{ VK_FORMAT_R32_SFLOAT,					Format::kR32_Float },
	{ VK_FORMAT_D32_SFLOAT,					Format::kD32_Float },
	{ VK_FORMAT_R32_UINT,					Format::kR32_UInt },
	{ VK_FORMAT_R32_SINT,					Format::kR32_SInt },
	{ VK_FORMAT_D24_UNORM_S8_UINT,			Format::kD24_UNorm_S8_UInt },
	{ VK_FORMAT_R8G8_UNORM,					Format::kRG8_UNorm },
	{ VK_FORMAT_R8G8_UINT,					Format::kRG8_UInt },
	{ VK_FORMAT_R8G8_SNORM,					Format::kRG8_SNorm },
	{ VK_FORMAT_R8G8_SINT,					Format::kRG8_SInt },
	{ VK_FORMAT_R16_SFLOAT,					Format::kR16_Float },
	{ VK_FORMAT_D16_UNORM,					Format::kD16_UNorm },
	{ VK_FORMAT_R16_UNORM,					Format::kR16_UNorm },
	{ VK_FORMAT_R16_UINT,					Format::kR16_UInt },
	{ VK_FORMAT_R16_SNORM,					Format::kR16_SNorm },
	{ VK_FORMAT_R16_SINT,					Format::kR16_SInt },
	{ VK_FORMAT_R8_UNORM,					Format::kR8_UNorm },
	{ VK_FORMAT_R8_UINT,					Format::kR8_UInt },
	{ VK_FORMAT_R8_SNORM,					Format::kR8_SNorm },
	{ VK_FORMAT_R8_SINT,					Format::kR8_SInt },
	{ VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,		Format::kRGB9E5_SHAREDEXP },
	{ VK_FORMAT_B8G8R8G8_422_UNORM,			Format::kRG8_BG8_UNorm },
	{ VK_FORMAT_G8B8G8R8_422_UNORM,			Format::kGR8_GB8_UNorm },
	{ VK_FORMAT_BC1_RGBA_UNORM_BLOCK,		Format::kBC1_UNorm },
	{ VK_FORMAT_BC1_RGBA_SRGB_BLOCK,		Format::kBC1_UNorm_SRGB },
	{ VK_FORMAT_BC2_UNORM_BLOCK,			Format::kBC2_UNorm },
	{ VK_FORMAT_BC2_SRGB_BLOCK,				Format::kBC2_UNorm_SRGB },
	{ VK_FORMAT_BC3_UNORM_BLOCK,			Format::kBC3_UNorm },
	{ VK_FORMAT_BC3_SRGB_BLOCK,				Format::kBC3_UNorm_SRGB },
	{ VK_FORMAT_BC4_UNORM_BLOCK,			Format::kBC4_UNorm },
	{ VK_FORMAT_BC4_SNORM_BLOCK,			Format::kBC4_SNorm },
	{ VK_FORMAT_BC5_UNORM_BLOCK,			Format::kBC5_UNorm },
	{ VK_FORMAT_BC5_SNORM_BLOCK,			Format::kBC5_SNorm },
	{ VK_FORMAT_R5G6B5_UNORM_PACK16,		Format::kB5G6R5_UNorm },
	{ VK_FORMAT_A1R5G5B5_UNORM_PACK16,		Format::kBGR5A1_UNorm },
	{ VK_FORMAT_B8G8R8A8_UNORM,				Format::kBGRA8_UNorm },
	{ VK_FORMAT_B8G8R8A8_SRGB,				Format::kBGRA8_UNorm_SRGB },
	{ VK_FORMAT_BC6H_UFLOAT_BLOCK,			Format::kBC6H_UF16 },
	{ VK_FORMAT_BC6H_SFLOAT_BLOCK,			Format::kBC6H_SF16 },
	{ VK_FORMAT_BC7_UNORM_BLOCK,			Format::kBC7_UNorm },
	{ VK_FORMAT_BC7_SRGB_BLOCK,				Format::kBC7_UNorm_SRGB },
	{ VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,	Format::k420_OPAQUE },
	{ VK_FORMAT_A4R4G4B4_UNORM_PACK16,		Format::kBGRA4_UNorm },
};


VkImageType GetVkImageType(const TextureType type)
{
	switch (type)
	{
	case TextureType::kTexture1D: return VkImageType::VK_IMAGE_TYPE_1D;
	case TextureType::kTexture2D: return VkImageType::VK_IMAGE_TYPE_2D;
	case TextureType::kTexture3D: return VkImageType::VK_IMAGE_TYPE_3D;
	}

	return VkImageType::VK_IMAGE_TYPE_2D;
}


VkImageViewType GetVkImageViewType(const TextureViewType viewType)
{
	switch (viewType)
	{
	case TextureViewType::kTexture1D:			return VkImageViewType::VK_IMAGE_VIEW_TYPE_1D;
	case TextureViewType::kTexture1DArray:		return VkImageViewType::VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case TextureViewType::kTexture2D:			return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	case TextureViewType::kTexture2DArray:		return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case TextureViewType::kTextureCube:			return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
	case TextureViewType::kTextureCubeArray:	return VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	case TextureViewType::kTexture3D:			return VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
	}

	return VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
}


const FormatMapVk& GetVkFormatMap(Format format)
{
	return kFormats[(u8)format];
}


VkFormat GetVkFormat(const Format format)
{
	return kFormats[(u8)format].format;
}


Format GetFormat(const VkFormat format)
{
	auto it = s_VkFormatToFormatMap.FindKey(format);
	if (it != s_VkFormatToFormatMap.cend())
	{
		return it->second;
	}

	return Format::kUnknown;
}


VkBufferUsageFlags GetVkBufferUsageFlags(const BufferUsage usageFlags)
{
	VkBufferUsageFlags bufferUsageFlags = 0;
	if ((usageFlags & BufferUsage::kTransferSrc			) == BufferUsage::kTransferSrc			)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT; }
	if ((usageFlags & BufferUsage::kTransferDst			) == BufferUsage::kTransferDst			)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT; }
	if ((usageFlags & BufferUsage::kUniformBuffer		) == BufferUsage::kUniformBuffer		)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
	if ((usageFlags & BufferUsage::kStorageBuffer		) == BufferUsage::kStorageBuffer		)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }
	if ((usageFlags & BufferUsage::kUniformTexelBuffer	) == BufferUsage::kUniformTexelBuffer	)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT; }
	if ((usageFlags & BufferUsage::kStorageTexelBuffer	) == BufferUsage::kStorageTexelBuffer	)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT; }
	if ((usageFlags & BufferUsage::kVertexBuffer		) == BufferUsage::kVertexBuffer			)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
	if ((usageFlags & BufferUsage::kIndexBuffer			) == BufferUsage::kIndexBuffer			)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
	if ((usageFlags & BufferUsage::kIndirectBuffer		) == BufferUsage::kIndirectBuffer		)	{ bufferUsageFlags |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT; }

	return bufferUsageFlags;
}


VkImageUsageFlags GetVkImageUsageFlags(const TextureUsage usageFlags)
{
	VkImageUsageFlags imageUsageFlags = 0;
	if ((usageFlags & TextureUsage::kTransferSrc		) == TextureUsage::kTransferSrc			)	{ imageUsageFlags |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT; }
	if ((usageFlags & TextureUsage::kTransferDst		) == TextureUsage::kTransferDst			)	{ imageUsageFlags |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT; }
	if ((usageFlags & TextureUsage::kShaderResource		) == TextureUsage::kShaderResource		)	{ imageUsageFlags |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT; }
	if ((usageFlags & TextureUsage::kUnorderedAccess	) == TextureUsage::kUnorderedAccess		)	{ imageUsageFlags |= VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT; }
	if ((usageFlags & TextureUsage::kColorTarget		) == TextureUsage::kColorTarget			)	{ imageUsageFlags |= VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
	if ((usageFlags & TextureUsage::kDepthStencilTarget	) == TextureUsage::kDepthStencilTarget	)	{ imageUsageFlags |= VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }

	return imageUsageFlags;
}


VkMemoryPropertyFlags GetVkMemoryPropertyFlags(const MemoryFlags memoryFlags)
{
	VkMemoryPropertyFlags memoryPropertyFlags = 0;
	if ((memoryFlags & MemoryFlags::kDeviceLocal	) == MemoryFlags::kDeviceLocal	)	{ memoryPropertyFlags |= VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; }
	if ((memoryFlags & MemoryFlags::kHostVisible	) == MemoryFlags::kHostVisible	)	{ memoryPropertyFlags |= VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }
	if ((memoryFlags & MemoryFlags::kHostCoherent	) == MemoryFlags::kHostCoherent	)	{ memoryPropertyFlags |= VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; }
	if ((memoryFlags & MemoryFlags::kHostCached		) == MemoryFlags::kHostCached	)	{ memoryPropertyFlags |= VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_CACHED_BIT; }

	return memoryPropertyFlags;
}


VkFilter GetVkFilter(const Filter filter)
{
	switch (filter)
	{
	case Filter::kPoint:	return VkFilter::VK_FILTER_NEAREST;
	case Filter::kLinear:	return VkFilter::VK_FILTER_LINEAR;
	}

	return VkFilter::VK_FILTER_LINEAR;
}


VkSamplerMipmapMode GetVkSamplerMipmapMode(const MipMapFilter mipMapFilter)
{
	switch (mipMapFilter)
	{
	case MipMapFilter::kPoint:	return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case MipMapFilter::kLinear:	return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
}


VkSamplerAddressMode GetVkSamplerAddressMode(const AddressMode addressMode)
{
	switch (addressMode)
	{
	case AddressMode::kWrap:		return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case AddressMode::kMirror:		return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case AddressMode::kClamp:		return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case AddressMode::kBorder:		return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case AddressMode::kMirrorOnce:	return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}

	return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
}


VkCompareOp GetVkCompareOp(const CompareOp compareOp)
{
	switch (compareOp)
	{
	case CompareOp::kNever:			return VkCompareOp::VK_COMPARE_OP_NEVER;
	case CompareOp::kLess:			return VkCompareOp::VK_COMPARE_OP_LESS;
	case CompareOp::kEqual:			return VkCompareOp::VK_COMPARE_OP_EQUAL;
	case CompareOp::kLessEqual:		return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
	case CompareOp::kGreater:		return VkCompareOp::VK_COMPARE_OP_GREATER;
	case CompareOp::kNotEqual:		return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
	case CompareOp::kGreaterEqual:	return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
	case CompareOp::kAlways:		return VkCompareOp::VK_COMPARE_OP_ALWAYS;
	}

	return VkCompareOp::VK_COMPARE_OP_NEVER;
}


VkCommandBufferLevel GetVkCommandBufferLevel(const CommandType commandLevel)
{
	switch (commandLevel)
	{
	case CommandType::kPrimary:	return VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	case CommandType::kSecondary:	return VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	}

	return VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}


VkVertexInputRate GetVkVertexInputRate(const InputRate inputRate)
{
	switch (inputRate)
	{
	case InputRate::kPerVertex:		return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
	case InputRate::kPerInstance:	return VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;
	}

	return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
}


VkPrimitiveTopology GetVkPrimitiveTopology(const Topology topology)
{
	switch (topology)
	{
	case Topology::kPointList:			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case Topology::kLineList:			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case Topology::kLineStrip:			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case Topology::kTriangleList:		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case Topology::kTriangleStrip:		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case Topology::kLineListAdj:		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case Topology::kLineStripAdj:		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
	case Topology::kTriangleListAdj:	return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
	case Topology::kTriangleStripAdj:	return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
	}

	return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}


VkPolygonMode GetVkPolygonMode(const FillMode fillMode)
{
	switch (fillMode)
	{
	case FillMode::kSolid:		return VkPolygonMode::VK_POLYGON_MODE_FILL;
	case FillMode::kWireframe:	return VkPolygonMode::VK_POLYGON_MODE_LINE;
	}

	return VkPolygonMode::VK_POLYGON_MODE_FILL;
}


VkCullModeFlags GetVkCullModeFlags(const CullMode cullMode)
{
	switch (cullMode)
	{
	case CullMode::kNone:			return VkCullModeFlagBits::VK_CULL_MODE_NONE;
	case CullMode::kFront:			return VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
	case CullMode::kBack:			return VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
	case CullMode::kFrontAndBack:	return VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK;
	}

	return VkCullModeFlagBits::VK_CULL_MODE_NONE;
}


VkFrontFace GetVkFrontFace(const FrontFace frontFace)
{
	switch (frontFace)
	{
	case FrontFace::kClockwise:			return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
	case FrontFace::kCounterClockwise:	return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
}


VkStencilOp GetVkStencilOp(const StencilOp stencilOp)
{
	switch (stencilOp)
	{
	case StencilOp::kKeep:		return VkStencilOp::VK_STENCIL_OP_KEEP;
	case StencilOp::kZero:		return VkStencilOp::VK_STENCIL_OP_ZERO;
	case StencilOp::kReplace:	return VkStencilOp::VK_STENCIL_OP_REPLACE;
	case StencilOp::kIncrSat:	return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case StencilOp::kDecrSat:	return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case StencilOp::kInvert:	return VkStencilOp::VK_STENCIL_OP_INVERT;
	case StencilOp::kIncrWrap:	return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case StencilOp::kDecrWrap:	return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}

	return VkStencilOp::VK_STENCIL_OP_KEEP;
}


VkSampleCountFlagBits GetVkSampleCountFlagBits(const u8 sampleCount)
{
	BvAssert((sampleCount & (sampleCount - 1)) == 0, "Sample count must be a power of 2");
	switch (sampleCount)
	{
	case 1:		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	case 2:		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT;
	case 4:		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT;
	case 8:		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT;
	case 16:	return VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT;
	case 32:	return VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT;
	//case 64:	return VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT; Supporting up to 32
	}

	return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
}


VkBlendOp GetVkBlendOp(const BlendOp blendOp)
{
	switch (blendOp)
	{
	case BlendOp::kAdd:			return VkBlendOp::VK_BLEND_OP_ADD;
	case BlendOp::kSubtract:	return VkBlendOp::VK_BLEND_OP_SUBTRACT;
	case BlendOp::kRevSubtract:	return VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT;
	case BlendOp::kMin:			return VkBlendOp::VK_BLEND_OP_MIN;
	case BlendOp::kMax:			return VkBlendOp::VK_BLEND_OP_MAX;
	}

	return VkBlendOp::VK_BLEND_OP_ADD;
}


VkBlendFactor GetVkBlendFactor(const BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case BlendFactor::kZero:			return VkBlendFactor::VK_BLEND_FACTOR_ZERO;
	case BlendFactor::kOne:				return VkBlendFactor::VK_BLEND_FACTOR_ONE;
	case BlendFactor::kSrcColor:		return VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR;
	case BlendFactor::kInvSrcColor:		return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case BlendFactor::kDstColor:		return VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR;
	case BlendFactor::kInvDstColor:		return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case BlendFactor::kSrcAlpha:		return VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;
	case BlendFactor::kInvkSrcAlpha:	return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::kDstAlpha:		return VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA;
	case BlendFactor::kInvDstAlpha:		return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case BlendFactor::kBlendFactor:		return VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR;
	case BlendFactor::kInvBlendFactor:	return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	case BlendFactor::kAlphaFactor:		return VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_ALPHA;
	case BlendFactor::kInvAlphaFactor:	return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
	case BlendFactor::kSrcAlphaSat:		return VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	case BlendFactor::kSrc1Color:		return VkBlendFactor::VK_BLEND_FACTOR_SRC1_COLOR;
	case BlendFactor::kInvSrc1Color:	return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
	case BlendFactor::kSrc1Alpha:		return VkBlendFactor::VK_BLEND_FACTOR_SRC1_ALPHA;
	case BlendFactor::kInvkSrc1Alpha:	return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}

	return VkBlendFactor::VK_BLEND_FACTOR_ZERO;
}


VkLogicOp GetVkLogicOp(const LogicOp logicOp)
{
	switch (logicOp)
	{
	case LogicOp::kClear:			return VkLogicOp::VK_LOGIC_OP_CLEAR;
	case LogicOp::kSet:				return VkLogicOp::VK_LOGIC_OP_SET;
	case LogicOp::kCopy:			return VkLogicOp::VK_LOGIC_OP_COPY;
	case LogicOp::kCopyInverted:	return VkLogicOp::VK_LOGIC_OP_COPY_INVERTED;
	case LogicOp::kNoOp:			return VkLogicOp::VK_LOGIC_OP_NO_OP;
	case LogicOp::kInvert:			return VkLogicOp::VK_LOGIC_OP_INVERT;
	case LogicOp::kAnd:				return VkLogicOp::VK_LOGIC_OP_AND;
	case LogicOp::kNand:			return VkLogicOp::VK_LOGIC_OP_NAND;
	case LogicOp::kOr:				return VkLogicOp::VK_LOGIC_OP_OR;
	case LogicOp::kNor:				return VkLogicOp::VK_LOGIC_OP_NOR;
	case LogicOp::kXor:				return VkLogicOp::VK_LOGIC_OP_XOR;
	case LogicOp::kEquiv:			return VkLogicOp::VK_LOGIC_OP_EQUIVALENT;
	case LogicOp::kAndReverse:		return VkLogicOp::VK_LOGIC_OP_AND_REVERSE;
	case LogicOp::kAndInverted:		return VkLogicOp::VK_LOGIC_OP_AND_INVERTED;
	case LogicOp::kOrReverse:		return VkLogicOp::VK_LOGIC_OP_OR_REVERSE;
	case LogicOp::kOrInverted:		return VkLogicOp::VK_LOGIC_OP_OR_INVERTED;
	}

	return VkLogicOp::VK_LOGIC_OP_CLEAR;
}


VkShaderStageFlagBits GetVkShaderStageFlagBits(const ShaderStage shaderStage)
{
	switch (shaderStage)
	{
	case ShaderStage::kVertex:				return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderStage::kHullOrControl:		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderStage::kDomainOrEvaluation:	return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ShaderStage::kGeometry:			return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderStage::kPixelOrFragment:				return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderStage::kCompute:				return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
	}

	return VkShaderStageFlagBits::VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}


VkDescriptorType GetVkDescriptorType(const ShaderResourceType resourceType)
{
	switch (resourceType)
	{
	case ShaderResourceType::kConstantBuffer:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case ShaderResourceType::kStructuredBuffer:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case ShaderResourceType::kRWStructuredBuffer:	return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case ShaderResourceType::kFormattedBuffer:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case ShaderResourceType::kRWFormattedBuffer:	return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case ShaderResourceType::kTexture:				return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case ShaderResourceType::kRWTexture:			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case ShaderResourceType::kSampler:				return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;
	}

	return VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
}


VkShaderStageFlags GetVkShaderStageFlags(const ShaderStage stages)
{
	VkShaderStageFlags shaderStages = 0;
	if ((stages & ShaderStage::kVertex				) == ShaderStage::kVertex				) { shaderStages |= VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;					}
	if ((stages & ShaderStage::kHullOrControl		) == ShaderStage::kHullOrControl		) { shaderStages |= VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;	}
	if ((stages & ShaderStage::kDomainOrEvaluation	) == ShaderStage::kDomainOrEvaluation	) { shaderStages |= VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; }
	if ((stages & ShaderStage::kGeometry			) == ShaderStage::kGeometry				) { shaderStages |= VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;				}
	if ((stages & ShaderStage::kPixelOrFragment				) == ShaderStage::kPixelOrFragment				) { shaderStages |= VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;				}
	if ((stages & ShaderStage::kCompute				) == ShaderStage::kCompute				) { shaderStages |= VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;					}

	return shaderStages;
}


VkAttachmentLoadOp GetVkAttachmentLoadOp(const LoadOp loadOp)
{
	switch (loadOp)
	{
	case LoadOp::kClear:	return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
	case LoadOp::kLoad:		return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
	}

	return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}


VkAttachmentStoreOp GetVkAttachmentStoreOp(const StoreOp storeOp)
{
	switch (storeOp)
	{
	case StoreOp::kStore:	return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
	}

	return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
}


VkIndexType GetVkIndexType(const IndexFormat indexFormat)
{
	switch (indexFormat)
	{
	//case IndexFormat::kU16:
	case IndexFormat::kU32:	return VkIndexType::VK_INDEX_TYPE_UINT32;
	}

	return VkIndexType::VK_INDEX_TYPE_UINT16;
}


VkImageLayout GetVkImageLayout(const ResourceState resourceState)
{
	switch (resourceState)
	{
	case ResourceState::kTransferSrc:		return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ResourceState::kTransferDst:		return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case ResourceState::kShaderResource:	return VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ResourceState::kRWResource:		return VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
	case ResourceState::kRenderTarget:		return VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ResourceState::kDepthStencilRead:	return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case ResourceState::kDepthStencilWrite:	return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case ResourceState::kPresent:			return VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	return VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
}


VkAccessFlags GetVkAccessFlags(const ResourceAccess resourceAccess)
{
	VkAccessFlags accessFlags = 0;

	if ((resourceAccess & ResourceAccess::kIndirectRead		) == ResourceAccess::kIndirectRead		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_INDIRECT_COMMAND_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kIndexRead		) == ResourceAccess::kIndexRead			)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_INDEX_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kVertexInputRead	) == ResourceAccess::kVertexInputRead	)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kUniformRead		) == ResourceAccess::kUniformRead		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_UNIFORM_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kShaderRead		) == ResourceAccess::kShaderRead		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kShaderWrite		) == ResourceAccess::kShaderWrite		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT; }
	if ((resourceAccess & ResourceAccess::kRenderTargetRead	) == ResourceAccess::kRenderTargetRead	)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kRenderTargetWrite) == ResourceAccess::kRenderTargetWrite	)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; }
	if ((resourceAccess & ResourceAccess::kDepthStencilRead	) == ResourceAccess::kDepthStencilRead	)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kDepthStencilWrite) == ResourceAccess::kDepthStencilWrite	)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; }
	if ((resourceAccess & ResourceAccess::kTransferRead		) == ResourceAccess::kTransferRead		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kTransferWrite	) == ResourceAccess::kTransferWrite		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT; }
	if ((resourceAccess & ResourceAccess::kHostRead			) == ResourceAccess::kHostRead			)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_HOST_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kHostWrite		) == ResourceAccess::kHostWrite			)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_HOST_WRITE_BIT; }
	if ((resourceAccess & ResourceAccess::kMemoryRead		) == ResourceAccess::kMemoryRead		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT; }
	if ((resourceAccess & ResourceAccess::kMemoryWrite		) == ResourceAccess::kMemoryWrite		)	{ accessFlags |= VkAccessFlagBits::VK_ACCESS_MEMORY_WRITE_BIT; }

	return accessFlags;
}


VkAccessFlags GetVkAccessFlags(const ResourceState resourceState)
{
	switch (resourceState)
	{
	case ResourceState::kVertexBuffer:		return VkAccessFlagBits::VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	case ResourceState::kIndexBuffer:		return VkAccessFlagBits::VK_ACCESS_INDEX_READ_BIT;
	case ResourceState::kIndirectBuffer:	return VkAccessFlagBits::VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	case ResourceState::kUniformBuffer:		return VkAccessFlagBits::VK_ACCESS_UNIFORM_READ_BIT;
	case ResourceState::kShaderResource:	return VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
	case ResourceState::kRWResource:		return VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT | VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT;
	case ResourceState::kTransferSrc:		return VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
	case ResourceState::kTransferDst:		return VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
	case ResourceState::kRenderTarget:		return VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case ResourceState::kDepthStencilRead:	return VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	case ResourceState::kDepthStencilWrite:	return VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case ResourceState::kPresent:			return VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
	}

	return 0;
}


VkPipelineStageFlags GetVkPipelineStageFlags(const VkAccessFlags accessFlags)
{
	if (accessFlags == 0)
	{
		return VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	VkPipelineStageFlags stageFlags = 0;
	if (accessFlags & VkAccessFlagBits::VK_ACCESS_INDIRECT_COMMAND_READ_BIT)
	{
		stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}
	if (accessFlags & (VkAccessFlagBits::VK_ACCESS_INDEX_READ_BIT | VkAccessFlagBits::VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT))
	{
		stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	}
	if (accessFlags & (VkAccessFlagBits::VK_ACCESS_UNIFORM_READ_BIT | VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT | VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT))
	{
		stageFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
			| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
			| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
			| VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
			| VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	if (accessFlags & (VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT))
	{
		stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	if (accessFlags & (VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT))
	{
		stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
			| VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	if (accessFlags & (VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT | VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT))
	{
		stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	if (accessFlags & (VkAccessFlagBits::VK_ACCESS_HOST_READ_BIT | VkAccessFlagBits::VK_ACCESS_HOST_WRITE_BIT))
	{
		stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_HOST_BIT;
	}
	if (accessFlags & (VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT))
	{
		stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}

	return stageFlags;
}


VkPipelineStageFlags GetVkPipelineStageFlags(const PipelineStage pipelineStage)
{
	VkPipelineStageFlags stageFlags = 0;

	if ((pipelineStage & PipelineStage::kBeginning				) == PipelineStage::kBeginning				) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; }
	if ((pipelineStage & PipelineStage::kIndirectDraw			) == PipelineStage::kIndirectDraw			) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT; }
	if ((pipelineStage & PipelineStage::kVertexInput			) == PipelineStage::kVertexInput			) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_INPUT_BIT; }
	if ((pipelineStage & PipelineStage::kVertexShader			) == PipelineStage::kVertexShader			) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT; }
	if ((pipelineStage & PipelineStage::kTessHullOrControlShader) == PipelineStage::kTessHullOrControlShader) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT; }
	if ((pipelineStage & PipelineStage::kTessDomainOrEvalShader	) == PipelineStage::kTessDomainOrEvalShader	) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT; }
	if ((pipelineStage & PipelineStage::kGeometryShader			) == PipelineStage::kGeometryShader			) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT; }
	if ((pipelineStage & PipelineStage::kPixelOrFragmentShader	) == PipelineStage::kPixelOrFragmentShader	) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; }
	if ((pipelineStage & PipelineStage::kDepth					) == PipelineStage::kDepth					) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; }
	if ((pipelineStage & PipelineStage::kRenderTarget			) == PipelineStage::kRenderTarget			) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; }
	if ((pipelineStage & PipelineStage::kComputeShader			) == PipelineStage::kComputeShader			) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; }
	if ((pipelineStage & PipelineStage::kTransfer				) == PipelineStage::kTransfer				) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT; }
	if ((pipelineStage & PipelineStage::kEnd					) == PipelineStage::kEnd					) { stageFlags |= VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; }

	return stageFlags;
}


VkCommandPoolCreateFlags GetVkCommandPoolCreateFlags(const CommandPoolFlags commandPoolFlags)
{
	VkCommandPoolCreateFlags flags = 0;
	if ((commandPoolFlags & CommandPoolFlags::kTransient) == CommandPoolFlags::kTransient)
	{
		flags |= VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	}

	return flags;
}


VkDescriptorPoolCreateFlags GetVkDescriptorPoolCreateFlags(const ShaderResourcePoolFlags poolFlags)
{
	VkDescriptorPoolCreateFlags flags = 0;
	if ((poolFlags & ShaderResourcePoolFlags::kRecycleDescriptors) == ShaderResourcePoolFlags::kNone)
	{
		flags |= VkDescriptorPoolCreateFlagBits::VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	}

	return flags;
}