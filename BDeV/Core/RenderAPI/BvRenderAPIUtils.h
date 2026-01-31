#pragma once


#include "BvRenderCommon.h"


struct FormatInfo
{
	const Format* m_pCastableFormats;
	u32 m_BitsPerPixel : 8;
	u32 m_ElementCount : 3;
	u32 m_WidthAlignment : 4;
	u32 m_HeightAlignment : 4;
	u32 m_DepthAlignment : 1;
	u32 m_PlaneCount : 2;
	Format m_ParentFormat;
	Format m_SRGBOrLinearVariant;
	bool m_IsSRGBFormat : 1;
	bool m_IsDepth : 1;
	bool m_IsStencil : 1;
	bool m_IsDepthStencil : 1;
	bool m_IsCompressed : 1;
	bool m_IsPacked : 1;
	bool m_IsPlanar : 1;
	bool m_IsTypeless : 1;
};


namespace Internal
{
	BV_INLINE constexpr std::pair<f32, f32> GetDepthBiasMRDForVkAndD3D(Format format)
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
		auto [vk, d3d] = GetDepthBiasMRDForVkAndD3D(format);
		f32 ratio = vk / d3d;
		return static_cast<i32>(std::roundf(depthBiasConstantFactor * ratio));
	}

	BV_INLINE f32 GetDepthBiasD3DToVk(Format format, i32 depthBias)
	{
		auto [vk, d3d] = GetDepthBiasMRDForVkAndD3D(format);
		f32 ratio = d3d / vk;
		return static_cast<f32>(depthBias) * ratio;
	}

	constexpr Format kNoCast[] =
	{
		Format::kUnknown
	};

	constexpr Format kRGBA32Cast[] =
	{
		Format::kRGBA32_Typeless,
		Format::kRGBA32_Float,
		Format::kRGBA32_UInt,
		Format::kRGBA32_SInt,
		Format::kUnknown
	};

	constexpr Format kRGB32Cast[] =
	{
		Format::kRGB32_Typeless,
		Format::kRGB32_Float,
		Format::kRGB32_UInt,
		Format::kRGB32_SInt,
		Format::kUnknown
	};

	constexpr Format kRGBA16Cast[] =
	{
		Format::kRGBA16_Typeless,
		Format::kRGBA16_Float,
		Format::kRGBA16_UNorm,
		Format::kRGBA16_UInt,
		Format::kRGBA16_SNorm,
		Format::kRGBA16_SInt,
		Format::kUnknown
	};

	constexpr Format kRG32Cast[] =
	{
		Format::kRG32_Typeless,
		Format::kRG32_Float,
		Format::kRG32_UInt,
		Format::kRG32_SInt,
		Format::kUnknown
	};

	constexpr Format kR32G8X24Cast[] =
	{
		Format::kR32G8X24_Typeless,
		Format::kD32_Float_S8X24_UInt,
		Format::kR32_Float_X8X24_Typeless,
		Format::kX32_Typeless_G8X24_UInt,
		Format::kUnknown
	};

	constexpr Format kRGB10A2Cast[] =
	{
		Format::kRGB10A2_Typeless,
		Format::kRGB10A2_UNorm,
		Format::kRGB10A2_UInt,
		Format::kRGB10_XR_BIAS_A2_UNorm,
		Format::kUnknown
	};

	constexpr Format kRGBA8Cast[] =
	{
		Format::kRGBA8_Typeless,
		Format::kRGBA8_UNorm,
		Format::kRGBA8_UNorm_SRGB,
		Format::kRGBA8_UInt,
		Format::kRGBA8_SNorm,
		Format::kRGBA8_SInt,
		Format::kUnknown
	};

	constexpr Format kRG16Cast[] =
	{
		Format::kRG16_Typeless,
		Format::kRG16_Float,
		Format::kRG16_UNorm,
		Format::kRG16_UInt,
		Format::kRG16_SNorm,
		Format::kRG16_SInt,
		Format::kUnknown
	};

	constexpr Format kR32Cast[] =
	{
		Format::kR32_Typeless,
		Format::kD32_Float,
		Format::kR32_Float,
		Format::kR32_UInt,
		Format::kR32_SInt,
		Format::kUnknown
	};

	constexpr Format kR24G8Cast[] =
	{
		Format::kR24G8_Typeless,
		Format::kD24_UNorm_S8_UInt,
		Format::kR24_UNorm_X8_Typeless,
		Format::kX24_Typeless_G8_UInt,
		Format::kUnknown
	};

	constexpr Format kRG8Cast[] =
	{
		Format::kRG8_Typeless,
		Format::kRG8_UNorm,
		Format::kRG8_UInt,
		Format::kRG8_SNorm,
		Format::kRG8_SInt,
		Format::kUnknown
	};

	constexpr Format kR16Cast[] =
	{
		Format::kR16_Typeless,
		Format::kR16_Float,
		Format::kD16_UNorm,
		Format::kR16_UNorm,
		Format::kR16_UInt,
		Format::kR16_SNorm,
		Format::kR16_SInt,
		Format::kUnknown
	};

	constexpr Format kR8Cast[] =
	{
		Format::kR8_Typeless,
		Format::kR8_UNorm,
		Format::kR8_UInt,
		Format::kR8_SNorm,
		Format::kR8_SInt,
		Format::kUnknown
	};

	constexpr Format kBC1Cast[] =
	{
		Format::kBC1_Typeless,
		Format::kBC1_UNorm,
		Format::kBC1_UNorm_SRGB,
		Format::kUnknown
	};

	constexpr Format kBC2Cast[] =
	{
		Format::kBC2_Typeless,
		Format::kBC2_UNorm,
		Format::kBC2_UNorm_SRGB,
		Format::kUnknown
	};

	constexpr Format kBC3Cast[] =
	{
		Format::kBC3_Typeless,
		Format::kBC3_UNorm,
		Format::kBC3_UNorm_SRGB,
		Format::kUnknown
	};

	constexpr Format kBC4Cast[] =
	{
		Format::kBC4_Typeless,
		Format::kBC4_UNorm,
		Format::kBC4_SNorm,
		Format::kUnknown
	};

	constexpr Format kBC5Cast[] =
	{
		Format::kBC5_Typeless,
		Format::kBC5_UNorm,
		Format::kBC5_SNorm,
		Format::kUnknown
	};

	constexpr Format kBGRA8Cast[] =
	{
		Format::kBGRA8_Typeless,
		Format::kBGRA8_UNorm,
		Format::kBGRA8_UNorm_SRGB,
		Format::kUnknown
	};

	constexpr Format kBGRX8Cast[] =
	{
		Format::kBGRX8_Typeless,
		Format::kBGRX8_UNorm,
		Format::kBGRX8_UNorm_SRGB,
		Format::kUnknown
	};

	constexpr Format kBC6Cast[] =
	{
		Format::kBC6H_Typeless,
		Format::kBC6H_UF16,
		Format::kBC6H_SF16,
		Format::kUnknown
	};

	constexpr Format kBC7Cast[] =
	{
		Format::kBC7_Typeless,
		Format::kBC7_UNorm,
		Format::kBC7_UNorm_SRGB,
		Format::kUnknown
	};

	constexpr FormatInfo kFormatInfos[] =
	{
		// Castable list	BPP		Comps/Align/PC Parent format				SRGB/Linear Variant Format	SRGB	Depth	Stencil	D + S	Compr	Packed	Planar	Typeless
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kUnknown
		{ kRGBA32Cast,		128,	4, 1, 1, 1, 1, Format::kRGBA32_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRGBA32_Typeless
		{ kRGBA32Cast,		128,	4, 1, 1, 1, 1, Format::kRGBA32_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA32_Float
		{ kRGBA32Cast,		128,	4, 1, 1, 1, 1, Format::kRGBA32_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA32_UInt
		{ kRGBA32Cast,		128,	4, 1, 1, 1, 1, Format::kRGBA32_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA32_SInt
		{ kRGB32Cast,		96,		3, 1, 1, 1, 1, Format::kRGB32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRGB32_Typeless
		{ kRGB32Cast,		96,		3, 1, 1, 1, 1, Format::kRGB32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGB32_Float
		{ kRGB32Cast,		96,		3, 1, 1, 1, 1, Format::kRGB32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGB32_UInt
		{ kRGB32Cast,		96,		3, 1, 1, 1, 1, Format::kRGB32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGB32_SInt
		{ kRGBA16Cast,		64,		4, 1, 1, 1, 1, Format::kRGBA16_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRGBA16_Typeless
		{ kRGBA16Cast,		64,		4, 1, 1, 1, 1, Format::kRGBA16_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA16_Float
		{ kRGBA16Cast,		64,		4, 1, 1, 1, 1, Format::kRGBA16_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA16_UNorm
		{ kRGBA16Cast,		64,		4, 1, 1, 1, 1, Format::kRGBA16_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA16_UInt
		{ kRGBA16Cast,		64,		4, 1, 1, 1, 1, Format::kRGBA16_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA16_SNorm
		{ kRGBA16Cast,		64,		4, 1, 1, 1, 1, Format::kRGBA16_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA16_SInt
		{ kRG32Cast,		64,		2, 1, 1, 1, 1, Format::kRG32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRG32_Typeless
		{ kRG32Cast,		64,		2, 1, 1, 1, 1, Format::kRG32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG32_Float
		{ kRG32Cast,		64,		2, 1, 1, 1, 1, Format::kRG32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG32_UInt
		{ kRG32Cast,		64,		2, 1, 1, 1, 1, Format::kRG32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG32_SInt
		{ kR32G8X24Cast,	64,		2, 1, 1, 1, 2, Format::kR32G8X24_Typeless,	Format::kUnknown,			false,	false,	false,	true,	false,	false,	true,	true	},// kR32G8X24_Typeless
		{ kR32G8X24Cast,	64,		2, 1, 1, 1, 2, Format::kR32G8X24_Typeless,	Format::kUnknown,			false,	true,	true,	true,	false,	false,	true,	false	},// kD32_Float_S8X24_UInt
		{ kR32G8X24Cast,	64,		1, 1, 1, 1, 2, Format::kR32G8X24_Typeless,	Format::kUnknown,			false,	false,	false,	true,	false,	false,	true,	true	},// kR32_Float_X8X24_Typeless
		{ kR32G8X24Cast,	64,		1, 1, 1, 1, 2, Format::kR32G8X24_Typeless,	Format::kUnknown,			false,	false,	false,	true,	false,	false,	true,	true	},// kX32_Typeless_G8X24_UInt
		{ kRGB10A2Cast,		32,		4, 1, 1, 1, 1, Format::kRGB10A2_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRGB10A2_Typeless
		{ kRGB10A2Cast,		32,		4, 1, 1, 1, 1, Format::kRGB10A2_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGB10A2_UNorm
		{ kRGB10A2Cast,		32,		4, 1, 1, 1, 1, Format::kRGB10A2_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGB10A2_UInt
		{ kNoCast,			32,		3, 1, 1, 1, 1, Format::kRG11B10_Float,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG11B10_Float
		{ kRGBA8Cast,		32,		4, 1, 1, 1, 1, Format::kRGBA8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRGBA8_Typeless
		{ kRGBA8Cast,		32,		4, 1, 1, 1, 1, Format::kRGBA8_Typeless,		Format::kRGBA8_UNorm_SRGB,	false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA8_UNorm
		{ kRGBA8Cast,		32,		4, 1, 1, 1, 1, Format::kRGBA8_Typeless,		Format::kRGBA8_UNorm,		true,	false,	false,	false,	false,	false,	false,	false	},// kRGBA8_UNorm_SRGB
		{ kRGBA8Cast,		32,		4, 1, 1, 1, 1, Format::kRGBA8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA8_UInt
		{ kRGBA8Cast,		32,		4, 1, 1, 1, 1, Format::kRGBA8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA8_SNorm
		{ kRGBA8Cast,		32,		4, 1, 1, 1, 1, Format::kRGBA8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGBA8_SInt
		{ kRG16Cast,		32,		2, 1, 1, 1, 1, Format::kRG16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRG16_Typeless
		{ kRG16Cast,		32,		2, 1, 1, 1, 1, Format::kRG16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG16_Float
		{ kRG16Cast,		32,		2, 1, 1, 1, 1, Format::kRG16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG16_UNorm
		{ kRG16Cast,		32,		2, 1, 1, 1, 1, Format::kRG16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG16_UInt
		{ kRG16Cast,		32,		2, 1, 1, 1, 1, Format::kRG16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG16_SNorm
		{ kRG16Cast,		32,		2, 1, 1, 1, 1, Format::kRG16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG16_SInt
		{ kR32Cast,			32,		1, 1, 1, 1, 1, Format::kR32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kR32_Typeless
		{ kR32Cast,			32,		1, 1, 1, 1, 1, Format::kR32_Typeless,		Format::kUnknown,			false,	true,	false,	false,	false,	false,	false,	false	},// kD32_Float
		{ kR32Cast,			32,		1, 1, 1, 1, 1, Format::kR32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR32_Float
		{ kR32Cast,			32,		1, 1, 1, 1, 1, Format::kR32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR32_UInt
		{ kR32Cast,			32,		1, 1, 1, 1, 1, Format::kR32_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR32_SInt
		{ kR24G8Cast,		32,		2, 1, 1, 1, 2, Format::kR24G8_Typeless,		Format::kUnknown,			false,	false,	false,	true,	false,	false,	true,	true	},// kR24G8_Typeless
		{ kR24G8Cast,		32,		2, 1, 1, 1, 2, Format::kR24G8_Typeless,		Format::kUnknown,			false,	true,	true,	true,	false,	false,	true,	false	},// kD24_UNorm_S8_UInt
		{ kR24G8Cast,		32,		1, 1, 1, 1, 2, Format::kR24G8_Typeless,		Format::kUnknown,			false,	false,	false,	true,	false,	false,	true,	true	},// kR24_UNorm_X8_Typeless
		{ kR24G8Cast,		32,		1, 1, 1, 1, 2, Format::kR24G8_Typeless,		Format::kUnknown,			false,	false,	false,	true,	false,	false,	true,	true	},// kX24_Typeless_G8_UInt
		{ kRG8Cast,			16,		2, 1, 1, 1, 1, Format::kRG8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kRG8_Typeless
		{ kRG8Cast,			16,		2, 1, 1, 1, 1, Format::kRG8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG8_UNorm
		{ kRG8Cast,			16,		2, 1, 1, 1, 1, Format::kRG8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG8_UInt
		{ kRG8Cast,			16,		2, 1, 1, 1, 1, Format::kRG8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG8_SNorm
		{ kRG8Cast,			16,		2, 1, 1, 1, 1, Format::kRG8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRG8_SInt
		{ kR16Cast,			16,		1, 1, 1, 1, 1, Format::kR16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kR16_Typeless
		{ kR16Cast,			16,		1, 1, 1, 1, 1, Format::kR16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR16_Float
		{ kR16Cast,			16,		1, 1, 1, 1, 1, Format::kR16_Typeless,		Format::kUnknown,			false,	true,	false,	false,	false,	false,	false,	false	},// kD16_UNorm
		{ kR16Cast,			16,		1, 1, 1, 1, 1, Format::kR16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR16_UNorm
		{ kR16Cast,			16,		1, 1, 1, 1, 1, Format::kR16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR16_UInt
		{ kR16Cast,			16,		1, 1, 1, 1, 1, Format::kR16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR16_SNorm
		{ kR16Cast,			16,		1, 1, 1, 1, 1, Format::kR16_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR16_SInt
		{ kR8Cast,			8,		1, 1, 1, 1, 1, Format::kR8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	true	},// kR8_Typeless
		{ kR8Cast,			8,		1, 1, 1, 1, 1, Format::kR8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR8_UNorm
		{ kR8Cast,			8,		1, 1, 1, 1, 1, Format::kR8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR8_UInt
		{ kR8Cast,			8,		1, 1, 1, 1, 1, Format::kR8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR8_SNorm
		{ kR8Cast,			8,		1, 1, 1, 1, 1, Format::kR8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR8_SInt
		{ kNoCast,			8,		1, 1, 1, 1, 1, Format::kA8_UNorm,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kA8_UNorm
		{ kNoCast,			1,		1, 8, 1, 1, 1, Format::kR1_UNorm,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kR1_UNorm
		{ kNoCast,			32,		3, 1, 1, 1, 1, Format::kRGB9E5_SHAREDEXP,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGB9E5_SHAREDEXP
		{ kNoCast,			16,		3, 2, 1, 1, 1, Format::kRG8_BG8_UNorm,		Format::kUnknown,			false,	false,	false,	false,	false,	true,	false,	false	},// kRG8_BG8_UNorm
		{ kNoCast,			16,		3, 2, 1, 1, 1, Format::kGR8_GB8_UNorm,		Format::kUnknown,			false,	false,	false,	false,	false,	true,	false,	false	},// kGR8_GB8_UNorm
		{ kBC1Cast,			64,		4, 4, 4, 1, 1, Format::kBC1_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	true	},// kBC1_Typeless
		{ kBC1Cast,			64,		4, 4, 4, 1, 1, Format::kBC1_Typeless,		Format::kBC1_UNorm_SRGB,	false,	false,	false,	false,	true,	false,	false,	false	},// kBC1_UNorm
		{ kBC1Cast,			64,		4, 4, 4, 1, 1, Format::kBC1_Typeless,		Format::kBC1_UNorm,			true,	false,	false,	false,	true,	false,	false,	false	},// kBC1_UNorm_SRGB
		{ kBC2Cast,			128,	4, 4, 4, 1, 1, Format::kBC2_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	true	},// kBC2_Typeless
		{ kBC2Cast,			128,	4, 4, 4, 1, 1, Format::kBC2_Typeless,		Format::kBC2_UNorm_SRGB,	false,	false,	false,	false,	true,	false,	false,	false	},// kBC2_UNorm
		{ kBC2Cast,			128,	4, 4, 4, 1, 1, Format::kBC2_Typeless,		Format::kBC2_UNorm,			true,	false,	false,	false,	true,	false,	false,	false	},// kBC2_UNorm_SRGB
		{ kBC3Cast,			128,	4, 4, 4, 1, 1, Format::kBC3_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	true	},// kBC3_Typeless
		{ kBC3Cast,			128,	4, 4, 4, 1, 1, Format::kBC3_Typeless,		Format::kBC3_UNorm_SRGB,	false,	false,	false,	false,	true,	false,	false,	false	},// kBC3_UNorm
		{ kBC3Cast,			128,	4, 4, 4, 1, 1, Format::kBC3_Typeless,		Format::kBC3_UNorm,			true,	false,	false,	false,	true,	false,	false,	false	},// kBC3_UNorm_SRGB
		{ kBC4Cast,			64,		1, 4, 4, 1, 1, Format::kBC4_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	true	},// kBC4_Typeless
		{ kBC4Cast,			64,		1, 4, 4, 1, 1, Format::kBC4_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	false	},// kBC4_UNorm
		{ kBC4Cast,			64,		1, 4, 4, 1, 1, Format::kBC4_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	false	},// kBC4_SNorm
		{ kBC5Cast,			128,	2, 4, 4, 1, 1, Format::kBC5_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	true	},// kBC5_Typeless
		{ kBC5Cast,			128,	2, 4, 4, 1, 1, Format::kBC5_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	false	},// kBC5_UNorm
		{ kBC5Cast,			128,	2, 4, 4, 1, 1, Format::kBC5_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	false	},// kBC5_SNorm
		{ kNoCast,			16,		3, 1, 1, 1, 1, Format::kB5G6R5_UNorm,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kB5G6R5_UNorm
		{ kNoCast,			16,		4, 1, 1, 1, 1, Format::kBGR5A1_UNorm,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kBGR5A1_UNorm
		{ kBGRA8Cast,		32,		4, 1, 1, 1, 1, Format::kBGRA8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kBGRA8_UNorm
		{ kBGRX8Cast,		32,		3, 1, 1, 1, 1, Format::kBGRX8_Typeless,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kBGRX8_UNorm
		{ kRGB10A2Cast,		32,		4, 1, 1, 1, 1, Format::kRGB10A2_Typeless,	Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kRGB10_XR_BIAS_A2_UNorm
		{ kBGRA8Cast,		32,		4, 1, 1, 1, 1, Format::kBGRA8_Typeless,		Format::kBGRA8_UNorm_SRGB,	false,	false,	false,	false,	false,	false,	false,	true	},// kBGRA8_Typeless
		{ kBGRA8Cast,		32,		4, 1, 1, 1, 1, Format::kBGRA8_Typeless,		Format::kBGRA8_Typeless,	true,	false,	false,	false,	false,	false,	false,	false	},// kBGRA8_UNorm_SRGB
		{ kBGRX8Cast,		32,		3, 1, 1, 1, 1, Format::kBGRX8_Typeless,		Format::kBGRX8_UNorm_SRGB,	false,	false,	false,	false,	false,	false,	false,	true	},// kBGRX8_Typeless
		{ kBGRX8Cast,		32,		3, 1, 1, 1, 1, Format::kBGRX8_Typeless,		Format::kBGRX8_Typeless,	true,	false,	false,	false,	false,	false,	false,	false	},// kBGRX8_UNorm_SRGB
		{ kBC6Cast,			128,	3, 4, 4, 1, 1, Format::kBC6H_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	true	},// kBC6H_Typeless
		{ kBC6Cast,			128,	3, 4, 4, 1, 1, Format::kBC6H_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	false	},// kBC6H_UF16
		{ kBC6Cast,			128,	3, 4, 4, 1, 1, Format::kBC6H_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	false	},// kBC6H_SF16
		{ kBC7Cast,			128,	4, 4, 4, 1, 1, Format::kBC7_Typeless,		Format::kUnknown,			false,	false,	false,	false,	true,	false,	false,	true	},// kBC7_Typeless
		{ kBC7Cast,			128,	4, 4, 4, 1, 1, Format::kBC7_Typeless,		Format::kBC7_UNorm_SRGB,	false,	false,	false,	false,	true,	false,	false,	false	},// kBC7_UNorm
		{ kBC7Cast,			128,	4, 4, 4, 1, 1, Format::kBC7_Typeless,		Format::kBC7_UNorm,			true,	false,	false,	false,	true,	false,	false,	false	},// kBC7_UNorm_SRGB
		{ kNoCast,			32,		4, 1, 1, 1, 1, Format::kAYUV,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kAYUV
		{ kNoCast,			32,		4, 1, 1, 1, 1, Format::kY410,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kY410
		{ kNoCast,			64,		4, 1, 1, 1, 1, Format::kY416,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kY416
		{ kNoCast,			8,		1, 2, 2, 1, 2, Format::kNV12,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// kNV12
		{ kNoCast,			16,		1, 2, 2, 1, 2, Format::kP010,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// kP010
		{ kNoCast,			16,		1, 2, 2, 1, 2, Format::kP016,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// kP016
		{ kNoCast,			8,		1, 2, 2, 1, 2, Format::k420_OPAQUE,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// k420_OPAQUE
		{ kNoCast,			16,		3, 2, 1, 1, 1, Format::kYUY2,				Format::kUnknown,			false,	false,	false,	false,	false,	true,	false,	false	},// kYUY2
		{ kNoCast,			32,		3, 2, 1, 1, 1, Format::kY210,				Format::kUnknown,			false,	false,	false,	false,	false,	true,	false,	false	},// kY210
		{ kNoCast,			32,		3, 2, 1, 1, 1, Format::kY216,				Format::kUnknown,			false,	false,	false,	false,	false,	true,	false,	false	},// kY216
		{ kNoCast,			8,		1, 4, 1, 1, 2, Format::kNV11,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// kNV11
		{ kNoCast,			8,		1, 1, 1, 1, 1, Format::kAI44,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kAI44
		{ kNoCast,			8,		1, 1, 1, 1, 1, Format::kIA44,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kIA44
		{ kNoCast,			8,		1, 1, 1, 1, 1, Format::kP8,					Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kP8
		{ kNoCast,			16,		1, 1, 1, 1, 1, Format::kA8P8,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kA8P8
		{ kNoCast,			16,		4, 1, 1, 1, 1, Format::kBGRA4_UNorm,		Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// kBGRA4_UNorm
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			0,		0, 1, 1, 1, 0, Format::kUnknown,			Format::kUnknown,			false,	false,	false,	false,	false,	false,	false,	false	},// Undefined
		{ kNoCast,			8,		1, 2, 1, 1, 2, Format::kP208,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// kP208
		{ kNoCast,			8,		1, 1, 2, 1, 3, Format::kV208,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// kV208
		{ kNoCast,			8,		1, 1, 1, 1, 3, Format::kV408,				Format::kUnknown,			false,	false,	false,	false,	false,	false,	true,	false	},// kV408
	};
}


class BvRenderUtils
{
public:
	BV_INLINE static u32 CalcSubresourceIndex(u32 mipSlice, u32 arraySlice, u32 planeSlice, u32 mipLevels, u32 arraySize)
	{
		return mipSlice + arraySlice * mipLevels + planeSlice * mipLevels * arraySize;
	}

	template<typename T, typename U, typename V>
	BV_INLINE static void DecomposeSubresourceIndex(u32 subresourceIndex, u32 mipLevels, u32 arraySize, T& mipSlice, U& arraySlice, V& planeSlice)
	{
		mipSlice = static_cast<T>(subresourceIndex % mipLevels);
		arraySlice = static_cast<U>((subresourceIndex / mipLevels) % arraySize);
		planeSlice = static_cast<V>(subresourceIndex / (mipLevels * arraySize));
	}
	
	BV_INLINE static u32 GetMipLevels(u32 width, u32 height, u32 depth)
	{
		return static_cast<u32>(std::floorf(std::log2f(std::max(std::max(width, height), depth))) + 1);
	}

	BV_INLINE static f32 ConvertIntDepthBiasToFloat(i32 depthBiasInt, u32 depthBits)
	{
		depthBits = std::clamp(depthBits, 1u, 24u); // clamp to safe range
		return depthBiasInt / f32(i32(1u << depthBits) - 1);
	}

	BV_INLINE static i32 ConvertFloatDepthBiasToInt(f32 depthBiasFloat, u32 depthBits)
	{
		depthBits = std::clamp(depthBits, 1u, 24u); // clamp to safe range
		return static_cast<i32>(std::round(depthBiasFloat * i32((1u << depthBits) - 1)));
	}

	static constexpr FormatInfo GetFormatInfo(Format format)
	{
		return Internal::kFormatInfos[u32(format)];
	}

	static u32 CalcRowPitch(Format format, u32 width);
	static void AdjustPlaneSizeAndFormatForCopyableLayout(Format format, u32 width, u32 height, u32 planeSlice,
		Format& planeFormat, u32& planeWidth, u32& planeHeight, u32& minPlanePitchWidth);
	static u64 GetCopyableFootprints(const TextureDesc& textureDesc, u32 placementAlignment, u32 pitchAlignment, u32 numSubresources,
		SubresourceFootprint* pSubresources, u64 baseOffset = 0, u32 firstSubresource = 0);
	static void UpdateSubresources(u32 numSubresources, const SubresourceData* pSubresources, const SubresourceFootprint* pFootprints, void* pDst);
};