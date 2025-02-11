#pragma once


//BV_IBVOBJECT_DEFINE_ID(, "11ee119e-5ecb-4675-9b95-f13439e1a3aa");
//BV_IBVOBJECT_DEFINE_ID(, "70617509-5657-4c47-9a4e-dc318b535076");
//BV_IBVOBJECT_DEFINE_ID(, "e04f4570-bb75-4ff2-b9e2-02ba4affb9de");
//BV_IBVOBJECT_DEFINE_ID(, "5ab21c28-09a4-4a86-a001-776e28fa3324");
//BV_IBVOBJECT_DEFINE_ID(, "9f90a079-c378-4b59-9522-958685fdea9c");
//BV_IBVOBJECT_DEFINE_ID(, "0bb5abad-c53b-47bd-b939-989cb9d2e58d");
//BV_IBVOBJECT_DEFINE_ID(, "8a9c658d-963e-49a2-9d67-7e5316f1a8fc");
//BV_IBVOBJECT_DEFINE_ID(, "8740fae9-74bb-4a0f-bf07-b4ff7179e6e4");


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include <algorithm>


class BvBuffer;
class BvTexture;
class BvTextureView;
class BvCommandContext;


constexpr u32 kMaxRenderTargets = 8;
constexpr u32 kMaxRenderTargetsWithDepth = kMaxRenderTargets + 1;
constexpr u32 kMaxShaderStages = 5;
constexpr u32 kMaxVertexAttributes = 16;
constexpr u32 kMaxVertexBindings = 16;
constexpr u32 kMaxVertexBuffers = 16;


enum RenderDeviceCapabilities : u32
{
	kNone							= 0,
	kWireframe						= BvBit(0),
	kGeometryShader					= BvBit(1),
	kTesselationShader				= BvBit(2),
	kDepthBoundsTest				= BvBit(3),
	kTimestampQueries				= BvBit(4),
	kIndirectDrawCount				= BvBit(5),
	kCustomBorderColor				= BvBit(6),
	kPredication					= BvBit(7),
	kConservativeRasterization		= BvBit(8),
	kShadingRate					= BvBit(9),
	kMeshShader						= BvBit(10),
	kRayTracing						= BvBit(11),
	kRayQuery						= BvBit(12),
};
BV_USE_ENUM_CLASS_OPERATORS(RenderDeviceCapabilities);


enum class CommandType : u8
{
	kGraphics,
	kCompute,
	kTransfer,
};


enum class IndexFormat : u8
{
	kU16,
	kU32
};


enum class MemoryType : u8
{
	kDevice,
	kUpload,
	kUploadNC, // Non-coherent; requires flushing buffer
	kReadBack,
	kReadBackNC, // Non-coherent; requires invalidating buffer
	kShared
};


// Copy of DXGI formats, so mapping to DirectX is simpler and also because of DDS textures
// Will do a VkFormat mapping from this
enum class Format : u8
{
	kUnknown = 0,
	kRGBA32_Typeless = 1,
	kRGBA32_Float = 2,
	kRGBA32_UInt = 3,
	kRGBA32_SInt = 4,
	kRGB32_Typeless = 5,
	kRGB32_Float = 6,
	kRGB32_UInt = 7,
	kRGB32_SInt = 8,
	kRGBA16_Typeless = 9,
	kRGBA16_Float = 10,
	kRGBA16_UNorm = 11,
	kRGBA16_UInt = 12,
	kRGBA16_SNorm = 13,
	kRGBA16_SInt = 14,
	kRG32_Typeless = 15,
	kRG32_Float = 16,
	kRG32_UInt = 17,
	kRG32_SInt = 18,
	kR32G8X24_Typeless = 19,
	kD32_Float_S8X24_UInt = 20,
	kR32_Float_X8X24_Typeless = 21,
	kX32_Typeless_G8X24_UInt = 22,
	kRGB10A2_Typeless = 23,
	kRGB10A2_UNorm = 24,
	kRGB10A2_UInt = 25,
	kRG11B10_Float = 26,
	kRGBA8_Typeless = 27,
	kRGBA8_UNorm = 28,
	kRGBA8_UNorm_SRGB = 29,
	kRGBA8_UInt = 30,
	kRGBA8_SNorm = 31,
	kRGBA8_SInt = 32,
	kRG16_Typeless = 33,
	kRG16_Float = 34,
	kRG16_UNorm = 35,
	kRG16_UInt = 36,
	kRG16_SNorm = 37,
	kRG16_SInt = 38,
	kR32_Typeless = 39,
	kD32_Float = 40,
	kR32_Float = 41,
	kR32_UInt = 42,
	kR32_SInt = 43,
	kR24G8_Typeless = 44,
	kD24_UNorm_S8_UInt = 45,
	kR24_UNorm_X8_Typeless = 46,
	kX24_Typeless_G8_UInt = 47,
	kRG8_Typeless = 48,
	kRG8_UNorm = 49,
	kRG8_UInt = 50,
	kRG8_SNorm = 51,
	kRG8_SInt = 52,
	kR16_Typeless = 53,
	kR16_Float = 54,
	kD16_UNorm = 55,
	kR16_UNorm = 56,
	kR16_UInt = 57,
	kR16_SNorm = 58,
	kR16_SInt = 59,
	kR8_Typeless = 60,
	kR8_UNorm = 61,
	kR8_UInt = 62,
	kR8_SNorm = 63,
	kR8_SInt = 64,
	kA8_UNorm = 65,
	kR1_UNorm = 66,
	kRGB9E5_SHAREDEXP = 67,
	kRG8_BG8_UNorm = 68,
	kGR8_GB8_UNorm = 69,
	kBC1_Typeless = 70,
	kBC1_UNorm = 71,
	kBC1_UNorm_SRGB = 72,
	kBC2_Typeless = 73,
	kBC2_UNorm = 74,
	kBC2_UNorm_SRGB = 75,
	kBC3_Typeless = 76,
	kBC3_UNorm = 77,
	kBC3_UNorm_SRGB = 78,
	kBC4_Typeless = 79,
	kBC4_UNorm = 80,
	kBC4_SNorm = 81,
	kBC5_Typeless = 82,
	kBC5_UNorm = 83,
	kBC5_SNorm = 84,
	kB5G6R5_UNorm = 85,
	kBGR5A1_UNorm = 86,
	kBGRA8_UNorm = 87,
	kBGRX8_UNorm = 88,
	kRGB10_XR_BIAS_A2_UNorm = 89,
	kBGRA8_Typeless = 90,
	kBGRA8_UNorm_SRGB = 91,
	kBGRX8_Typeless = 92,
	kBGRX8_UNorm_SRGB = 93,
	kBC6H_Typeless = 94,
	kBC6H_UF16 = 95,
	kBC6H_SF16 = 96,
	kBC7_Typeless = 97,
	kBC7_UNorm = 98,
	kBC7_UNorm_SRGB = 99,
	kAYUV = 100,
	kY410 = 101,
	kY416 = 102,
	kNV12 = 103, // 2 planes (Y plane and interleaved UV plane)
	kP010 = 104, // 2 planes (Y plane and interleaved UV plane, 10 bits per channel)
	kP016 = 105, // 2 planes (Y plane and interleaved UV plane, 16 bits per channel)
	k420_OPAQUE = 106, // 2 planes (Y plane and interleaved UV plane, similar to NV12 but opaque for additional processing)
	kYUY2 = 107,
	kY210 = 108,
	kY216 = 109,
	kNV11 = 110, // 2 planes (Y plane and interleaved UV plane, with UV in 10-bit format)
	kAI44 = 111,
	kIA44 = 112,
	kP8 = 113,
	kA8P8 = 114,
	kBGRA4_UNorm = 115,
	kP208 = 130, // 2 planes (Y plane and interleaved UV plane)
	kV208 = 131, // 3 planes (Y plane, V plane, and U plane)
	kV408 = 132, // 3 planes (Y plane, V plane, and U plane)
};


enum class ColorSpace
{
	kAuto,
	kSRGBNonLinear,
	kExtendedLinear,
	kHDR10
};


struct FormatInfo
{
	u8 m_BitsPerPixel;
	u8 m_ElementCount;
	u8 m_ElementSize;
	u8 m_BlockWidth;
	u8 m_BlockHeight;
	u8 m_BlocksPerElement;
	Format m_SRGBOrLinearVariant;
	bool m_IsSRGBFormat : 1;
	bool m_IsDepthStencil : 1;
	bool m_IsCompressed : 1;
	bool m_IsPacked : 1;
	bool m_IsPlanar : 1;
	u8 m_PlaneCount : 3;
};


constexpr FormatInfo GetFormatInfo(Format format)
{
	constexpr FormatInfo kFormatInfos[] =
	{
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// kUnknown
		{ 128, 4, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA32_Typeless
		{ 128, 4, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA32_Float
		{ 128, 4, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA32_UInt
		{ 128, 4, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA32_SInt
		{ 96, 3, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB32_Typeless
		{ 96, 3, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB32_Float
		{ 96, 3, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB32_UInt
		{ 96, 3, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB32_SInt
		{ 64, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA16_Typeless
		{ 64, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA16_Float
		{ 64, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA16_UNorm
		{ 64, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA16_UInt
		{ 64, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA16_SNorm
		{ 64, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA16_SInt
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG32_Typeless
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG32_Float
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG32_UInt
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG32_SInt
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kR32G8X24_Typeless
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kD32_Float_S8X24_UInt
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kR32_Float_X8X24_Typeless
		{ 64, 2, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kX32_Typeless_G8X24_UInt
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB10A2_Typeless
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB10A2_UNorm
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB10A2_UInt
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG11B10_Float
		{ 32, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA8_Typeless
		{ 32, 4, 1, 1, 1, 1, Format::kRGBA8_UNorm_SRGB, false, false, false, false, false, 1 },// kRGBA8_UNorm
		{ 32, 4, 1, 1, 1, 1, Format::kRGBA8_UNorm, true, false, false, false, false, 1 },// kRGBA8_UNorm_SRGB
		{ 32, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA8_UInt
		{ 32, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA8_SNorm
		{ 32, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGBA8_SInt
		{ 32, 2, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG16_Typeless
		{ 32, 2, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG16_Float
		{ 32, 2, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG16_UNorm
		{ 32, 2, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG16_UInt
		{ 32, 2, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG16_SNorm
		{ 32, 2, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG16_SInt
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR32_Typeless
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kD32_Float
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR32_Float
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR32_UInt
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR32_SInt
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kR24G8_Typeless
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kD24_UNorm_S8_UInt
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kR24_UNorm_X8_Typeless
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kX24_Typeless_G8_UInt
		{ 16, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG8_Typeless
		{ 16, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG8_UNorm
		{ 16, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG8_UInt
		{ 16, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG8_SNorm
		{ 16, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRG8_SInt
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR16_Typeless
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR16_Float
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, true, false, false, false, 1 },// kD16_UNorm
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR16_UNorm
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR16_UInt
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR16_SNorm
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR16_SInt
		{ 8, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR8_Typeless
		{ 8, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR8_UNorm
		{ 8, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR8_UInt
		{ 8, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR8_SNorm
		{ 8, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR8_SInt
		{ 8, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kA8_UNorm
		{ 1, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kR1_UNorm
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB9E5_SHAREDEXP
		{ 32, 4, 1, 1, 1, 4, Format::kUnknown, false, false, false, true, false, 1 },// kRG8_BG8_UNorm
		{ 32, 4, 1, 1, 1, 4, Format::kUnknown, false, false, false, true, false, 1 },// kGR8_GB8_UNorm
		{ 4, 3, 8, 4, 4, 8, Format::kUnknown, false, false, true, false, false, 1 },// kBC1_Typeless
		{ 4, 3, 8, 4, 4, 8, Format::kBC1_UNorm_SRGB, false, false, true, false, false, 1 },// kBC1_UNorm
		{ 4, 3, 8, 4, 4, 8, Format::kBC1_UNorm, true, false, true, false, false, 1 },// kBC1_UNorm_SRGB
		{ 8, 4, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC2_Typeless
		{ 8, 4, 16, 4, 4, 16, Format::kBC2_UNorm_SRGB, false, false, true, false, false, 1 },// kBC2_UNorm
		{ 8, 4, 16, 4, 4, 16, Format::kBC2_UNorm, true, false, true, false, false, 1 },// kBC2_UNorm_SRGB
		{ 8, 4, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC3_Typeless
		{ 8, 4, 16, 4, 4, 16, Format::kBC3_UNorm_SRGB, false, false, true, false, false, 1 },// kBC3_UNorm
		{ 8, 4, 16, 4, 4, 16, Format::kBC3_UNorm, true, false, true, false, false, 1 },// kBC3_UNorm_SRGB
		{ 4, 1, 8, 4, 4, 8, Format::kUnknown, false, false, true, false, false, 1 },// kBC4_Typeless
		{ 4, 1, 8, 4, 4, 8, Format::kUnknown, false, false, true, false, false, 1 },// kBC4_UNorm
		{ 4, 1, 8, 4, 4, 8, Format::kUnknown, false, false, true, false, false, 1 },// kBC4_SNorm
		{ 8, 2, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC5_Typeless
		{ 8, 2, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC5_UNorm
		{ 8, 2, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC5_SNorm
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kB5G6R5_UNorm
		{ 16, 1, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kBGR5A1_UNorm
		{ 32, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kBGRA8_UNorm
		{ 32, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kBGRX8_UNorm
		{ 32, 1, 4, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kRGB10_XR_BIAS_A2_UNorm
		{ 32, 4, 1, 1, 1, 1, Format::kBGRA8_UNorm_SRGB, false, false, false, false, false, 1 },// kBGRA8_Typeless
		{ 32, 4, 1, 1, 1, 1, Format::kBGRA8_Typeless, true, false, false, false, false, 1 },// kBGRA8_UNorm_SRGB
		{ 32, 4, 1, 1, 1, 1, Format::kBGRX8_UNorm_SRGB, false, false, false, false, false, 1 },// kBGRX8_Typeless
		{ 32, 4, 1, 1, 1, 1, Format::kBGRX8_Typeless, true, false, false, false, false, 1 },// kBGRX8_UNorm_SRGB
		{ 8, 3, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC6H_Typeless
		{ 8, 3, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC6H_UF16
		{ 8, 3, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC6H_SF16
		{ 8, 4, 16, 4, 4, 16, Format::kUnknown, false, false, true, false, false, 1 },// kBC7_Typeless
		{ 8, 4, 16, 4, 4, 16, Format::kBC7_UNorm_SRGB, false, false, true, false, false, 1 },// kBC7_UNorm
		{ 8, 4, 16, 4, 4, 16, Format::kBC7_UNorm, true, false, true, false, false, 1 },// kBC7_UNorm_SRGB
		{ 32, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kAYUV
		{ 32, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kY410
		{ 64, 4, 2, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kY416
		{ 12, 3, 1, 2, 2, 2, Format::kUnknown, false, false, false, false, true, 2 },// kNV12
		{ 24, 3, 2, 2, 2, 4, Format::kUnknown, false, false, false, false, true, 2 },// kP010
		{ 24, 3, 2, 2, 2, 4, Format::kUnknown, false, false, false, false, true, 2 },// kP016
		{ 12, 3, 1, 1, 1, 2, Format::kUnknown, false, false, false, false, true, 2 },// k420_OPAQUE
		{ 32, 3, 1, 2, 1, 4, Format::kUnknown, false, false, false, true, false, 1 },// kYUY2
		{ 64, 3, 2, 2, 1, 8, Format::kUnknown, false, false, false, true, false, 1 },// kY210
		{ 64, 3, 2, 2, 1, 8, Format::kUnknown, false, false, false, true, false, 1 },// kY216
		{ 12, 3, 1, 4, 2, 1, Format::kUnknown, false, false, false, false, true, 2 },// kNV11
		{ 8, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kAI44
		{ 8, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kIA44
		{ 8, 1, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kP8
		{ 16, 2, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kA8P8
		{ 16, 4, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, false, 1 },// kBGRA4_UNorm
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 0, 0, 0, 0, 0, 1, Format::kUnknown, false, false, false, false, false, 0 },// Undefined
		{ 16, 3, 1, 2, 2, 2, Format::kUnknown, false, false, false, false, true, 2 },// kP208
		{ 16, 3, 1, 2, 2, 1, Format::kUnknown, false, false, false, false, true, 3 },// kV208
		{ 24, 3, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, true, 4 },// kV408
	};

	return kFormatInfos[static_cast<u32>(format)];
}


enum class FormatFeatures : u16
{
	kNone =						0,
	kTexture1D =				BvBit(0),
	kTexture2D =				BvBit(1),
	kTexture3D =				BvBit(2),
	kTextureCube =				BvBit(3),
	kBuffer =					BvBit(4),
	kRWBuffer =					BvBit(5),
	kVertexBuffer =				BvBit(6),
	kRenderTarget =				BvBit(7),
	kBlendable =				BvBit(8),
	kDepthStencil =				BvBit(9),
	kSampling =					BvBit(10),
	kComparisonSampling =		BvBit(11),
	kRWTexture =				BvBit(12),
	kResolve =					BvBit(13),
	kMultisampleRenderTarget =	BvBit(14),
};
BV_USE_ENUM_CLASS_OPERATORS(FormatFeatures)


enum class CompareOp : u8
{
	kNever,
	kLess,
	kEqual,
	kLessEqual,
	kGreater,
	kNotEqual,
	kGreaterEqual,
	kAlways,
};


enum class ShaderStage : u16
{
	kUnknown				= 0,
	kVertex					= BvBit(0),
	kHullOrControl			= BvBit(1),
	kDomainOrEvaluation		= BvBit(2),
	kGeometry				= BvBit(3),
	kPixelOrFragment		= BvBit(4),
	kCompute				= BvBit(5),
	kMesh					= BvBit(6),
	kAmplificationOrTask	= BvBit(7),
	kAllStages				= BvBit(8) - 1,
};
BV_USE_ENUM_CLASS_OPERATORS(ShaderStage)


enum class ResourceState : u8
{
	// Initial state
	kCommon,

	// States only used by Buffers
	kVertexBuffer,
	kIndexBuffer,
	kIndirectBuffer,
	kUniformBuffer,

	// States used by buffers and textures
	kShaderResource,
	kRWResource,
	kTransferSrc,
	kTransferDst,

	// States only used by textures
	kRenderTarget,
	kDepthStencilRead,
	kDepthStencilWrite,
	kPresent,

	kShadingRate,
};


enum class ResourceAccess : u32
{
	kNone					= 0,
	kIndirectRead			= BvBit(0),
	kIndexRead				= BvBit(1),
	kVertexInputRead		= BvBit(2),
	kUniformRead			= BvBit(3),
	kShaderRead				= BvBit(4),
	kShaderWrite			= BvBit(5),
	kRenderTargetRead		= BvBit(6),
	kRenderTargetWrite		= BvBit(7),
	kDepthStencilRead		= BvBit(8),
	kDepthStencilWrite		= BvBit(9),
	kTransferRead			= BvBit(10),
	kTransferWrite			= BvBit(11),
	kHostRead				= BvBit(12),
	kHostWrite				= BvBit(13),
	kMemoryRead				= BvBit(14),
	kMemoryWrite			= BvBit(15),
	kInputAttachmentRead	= BvBit(16),
	kShadingRateRead		= BvBit(17),
	kAuto					= BvBit(18)
};
BV_USE_ENUM_CLASS_OPERATORS(ResourceAccess);


enum class PipelineStage : u32
{
	kNone						= 0,
	kBeginning					= BvBit(0),
	kIndirectDraw				= BvBit(1),
	kVertexInput				= BvBit(2),
	kVertexShader				= BvBit(3),
	kTessHullOrControlShader	= BvBit(4),
	kTessDomainOrEvalShader		= BvBit(5),
	kGeometryShader				= BvBit(6),
	kPixelOrFragmentShader		= BvBit(7),
	kDepth						= BvBit(8),
	kRenderTarget				= BvBit(9),
	kComputeShader				= BvBit(10),
	kTransfer					= BvBit(11),
	kHost						= BvBit(12),
	kEnd						= BvBit(13),
	kAllShaderStages			= kVertexShader | kTessHullOrControlShader | kTessDomainOrEvalShader
		| kGeometryShader | kPixelOrFragmentShader | kComputeShader,
	kShadingRate				= BvBit(14),
	kAuto						= BvBit(15)
};
BV_USE_ENUM_CLASS_OPERATORS(PipelineStage);


enum class LoadOp : u8
{
	kDontCare = 0x0,
	kLoad = 0x1,
	kClear = 0x2,
};


enum class StoreOp : u8
{
	kDontCare = 0x0,
	kStore = 0x1,
};


struct Extent3D
{
	union
	{
		struct
		{
			u32 width;
			u32 height;
			u32 depth;
		};
		struct
		{
			u32 x;
			u32 y;
			u32 z;
		};
	};

	friend constexpr bool operator==(const Extent3D& lhs, const Extent3D& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}

	friend constexpr bool operator!=(const Extent3D& lhs, const Extent3D& rhs)
	{
		return !(lhs == rhs);
	}
};


struct Offset3D
{
	i32 x;
	i32 y;
	i32 z;
};


enum class BufferUsage : u16
{
	kNone = 0,
	kUniformBuffer =		BvBit(0),
	kStorageBuffer =		BvBit(1),
	kUniformTexelBuffer =	BvBit(2),
	kStorageTexelBuffer =	BvBit(3),
	kIndexBuffer =			BvBit(4),
	kVertexBuffer =			BvBit(5),
	kIndirectBuffer =		BvBit(6),
};
BV_USE_ENUM_CLASS_OPERATORS(BufferUsage);


enum class BufferCreateFlags : u8
{
	kNone = 0,
	kCreateMapped = BvBit(0)
};
BV_USE_ENUM_CLASS_OPERATORS(BufferCreateFlags);


struct BufferDesc
{
	u64 m_Size = 0;
	BufferCreateFlags m_CreateFlags = BufferCreateFlags::kNone;
	BufferUsage m_UsageFlags = BufferUsage::kNone;
	MemoryType m_MemoryType = MemoryType::kDevice;
	ResourceState m_ResourceState = ResourceState::kCommon;
};


struct BufferViewDesc
{
	BvBuffer* m_pBuffer = nullptr;
	u64 m_Offset = 0;
	u64 m_ElementCount = 0;
	u64 m_Stride = 0;
	Format m_Format = Format::kUnknown;
};


enum class TextureType : u8
{
	kTexture1D,
	kTexture2D,
	kTexture3D,
};


enum class TextureUsage : u8
{
	kDefault = 0,
	kTransferSrc =			BvBit(0),
	kTransferDst =			BvBit(1),
	kShaderResource =		BvBit(2),
	kUnorderedAccess =		BvBit(3),
	kColorTarget =			BvBit(4),
	kDepthStencilTarget =	BvBit(5),
};
BV_USE_ENUM_CLASS_OPERATORS(TextureUsage);


enum class TextureCreateFlags : u8
{
	kNone = 0,
	kCreateCubemap = BvBit(0),
	kReserveMips = BvBit(1),
	kGenerateMips = kReserveMips | BvBit(2)
};
BV_USE_ENUM_CLASS_OPERATORS(TextureCreateFlags);


struct TextureDesc
{
	Extent3D m_Size{ 1,1,1 };
	u32 m_Alignment = 0;
	u32 m_MipLevels = 1;
	u32 m_ArraySize = 1;
	u8 m_SampleCount = 1;
	TextureType m_ImageType = TextureType::kTexture2D;
	Format m_Format = Format::kUnknown;
	TextureCreateFlags m_CreateFlags = TextureCreateFlags::kNone;
	TextureUsage m_UsageFlags = TextureUsage::kDefault;
	MemoryType m_MemoryType = MemoryType::kDevice;
	ResourceState m_ResourceState = ResourceState::kShaderResource;
};


enum class TextureViewType : u8
{
	kTexture1D,
	kTexture1DArray,
	kTexture2D,
	kTexture2DArray,
	kTextureCube,
	kTextureCubeArray,
	kTexture3D,
};


struct SubresourceDesc
{
	u32 firstMip = 0;
	u32 mipCount = kU32Max;
	u32 firstLayer = 0;
	u32 layerCount = kU32Max;
};


struct TextureSubresourceInfo
{
	u32 m_Width;
	u32 m_Height;
	u32 m_Detph;
	u64 m_NumRows;
	u64 m_RowPitch;
	u64 m_SlicePitch;
	u64 m_MipSize;
};


struct SubresourceFootprint
{
	u64 m_Offset;
	TextureSubresourceInfo m_Subresource;
};


struct SubresourceData
{
	const u8* m_pData;
	u64 m_RowPitch;
	u64 m_SlicePitch;
};


struct BufferInitData
{
	BvCommandContext* m_pContext = nullptr;
	const void* m_pData;
	u64 m_Size;
};


struct TextureInitData
{
	BvCommandContext* m_pContext = nullptr;
	u32 m_SubresourceCount = 0;
	SubresourceData* m_pSubresources = nullptr;
};


struct TextureViewDesc
{
	BvTexture* m_pTexture = nullptr;
	SubresourceDesc m_SubresourceDesc;
	TextureViewType m_ViewType = TextureViewType::kTexture2D;
	Format m_Format = Format::kUnknown;
};


struct ClearColorValue
{
	constexpr ClearColorValue()
		: r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	constexpr ClearColorValue(float r, float g, float b, float a = 1.0f)
		: r(r), g(g), b(b), a(a) {}
	constexpr ClearColorValue(const float* pColors)
	{
		std::copy(pColors, pColors + 4, colors);
	}
	constexpr ClearColorValue(float depth, u8 stencil = 0)
		: depth(depth), stencil(stencil) {}
	union
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};
		struct
		{
			float colors[4];
		};
		struct
		{
			float depth;
			u8 stencil;
		};
	};
};


enum class ClearFlags : u8
{
	kNone = 0,
	kClearDepth = BvBit(0),
	kClearStencil = BvBit(1),
	kClearDepthAndStencil = kClearDepth | kClearStencil
};
BV_USE_ENUM_CLASS_OPERATORS(ClearFlags);


struct Viewport
{
	float x;
	float y;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};


struct Rect
{
	i32 x;
	i32 y;
	u32 width;
	u32 height;
};


struct BufferCopyDesc
{
	u64 m_SrcSize = kU64Max;
	u64 m_SrcOffset = 0;
	u64 m_DstOffset = 0;
};


struct TextureCopyDesc
{
	Offset3D m_SrcTextureOffset{ 0,0,0 };
	u32 m_SrcMip = 0;
	u32 m_SrcLayer = 0;
	Offset3D m_DstTextureOffset{ 0,0,0 };
	u32 m_DstMip = 0;
	u32 m_DstLayer = 0;
	Extent3D m_Size{ kU32Max, kU32Max, kU32Max };
};


struct BufferTextureCopyDesc
{
	u64 m_BufferSize = kU64Max;
	u64 m_BufferOffset = 0;
	Offset3D m_TextureOffset{ 0,0,0 };
	Extent3D m_TextureSize{ kU32Max, kU32Max, kU32Max };
	u32 m_Mip = 0;
	u32 m_Layer = 0;
};


struct ResourceBarrierDesc
{
	enum class Type : u8
	{
		kStateTransition,
		kMemory,
		kStateTransitionAcquire,
		kStateTransitionRelease,
	};

	BvTexture* m_pTexture = nullptr;
	BvBuffer* m_pBuffer = nullptr;

	ResourceState m_SrcLayout = ResourceState::kCommon;
	ResourceState m_DstLayout = ResourceState::kCommon;

	Type m_Type = Type::kStateTransition;

	// These fields can be used for more detailed barriers (in Vulkan)
	CommandType m_SrcQueue = CommandType::kGraphics;
	CommandType m_DstQueue = CommandType::kGraphics;

	ResourceAccess m_SrcAccess = ResourceAccess::kAuto;
	ResourceAccess m_DstAccess = ResourceAccess::kAuto;

	PipelineStage m_SrcPipelineStage = PipelineStage::kAuto;
	PipelineStage m_DstPipelineStage = PipelineStage::kAuto;

	SubresourceDesc m_Subresource;
};


enum class ShaderResourceType : u8
{
	kUnknown,
	kConstantBuffer,
	kStructuredBuffer,
	kRWStructuredBuffer,
	kFormattedBuffer,
	kRWFormattedBuffer,
	kTexture,
	kRWTexture,
	kSampler,
};


enum class Filter : u8
{
	kPoint,
	kLinear,
};


enum class MipMapFilter : u8
{
	kPoint,
	kLinear,
};


enum class AddressMode : u8
{
	kWrap,
	kMirror,
	kClamp,
	kBorder,
	kMirrorOnce,
};


enum class ResolveMode : u8
{
	kNone,
	kAverage,
	kMin,
	kMax
};


enum class ShadingRateDimensions : u8
{
	k1x1 = 0,
	k1x2 = 0x1,
	k2x1 = 0x4,
	k2x2 = 0x5,
	k2x4 = 0x6,
	k4x2 = 0x9,
	k4x4 = 0xa
};


enum class ShadingRateCombinerOp : u8
{
	kKeep,
	kReplace,
	kMin,
	kMax,
	kSum,
	kMul,
};


struct RenderTargetDesc
{
	constexpr RenderTargetDesc(BvTextureView* pView, const ClearColorValue& clearValues, LoadOp loadOp, StoreOp storeOp,
		ResourceState stateBefore, ResourceState state, ResourceState stateAfter)
		: m_pView(pView), m_ClearValues(clearValues), m_LoadOp(loadOp), m_StoreOp(storeOp),
		m_StateBefore(stateBefore), m_State(state), m_StateAfter(stateAfter), m_ShadingRateTexelSizes{}
	{
	}

	static constexpr RenderTargetDesc AsRenderTarget(BvTextureView* pView, const ClearColorValue& clearValues = ClearColorValue(0.0f, 0.0f, 0.0f), LoadOp loadOp = LoadOp::kClear,
		StoreOp storeOp = StoreOp::kStore, ResourceState stateBefore = ResourceState::kCommon, ResourceState stateAfter = ResourceState::kShaderResource)
	{
		return RenderTargetDesc(pView, clearValues, loadOp, storeOp, stateBefore, ResourceState::kRenderTarget, stateAfter);
	}

	static constexpr RenderTargetDesc AsSwapChain(BvTextureView* pView, const ClearColorValue& clearValues = ClearColorValue(0.0f, 0.0f, 0.0f), LoadOp loadOp = LoadOp::kClear,
		StoreOp storeOp = StoreOp::kStore, ResourceState stateBefore = ResourceState::kCommon, ResourceState stateAfter = ResourceState::kPresent)
	{
		return RenderTargetDesc(pView, clearValues, loadOp, storeOp, stateBefore, ResourceState::kRenderTarget, stateAfter);
	}

	static constexpr RenderTargetDesc AsDepthStencil(BvTextureView* pView, const ClearColorValue& clearValues = ClearColorValue(1.0f, 0), LoadOp loadOp = LoadOp::kClear,
		StoreOp storeOp = StoreOp::kStore, ResourceState stateBefore = ResourceState::kCommon, ResourceState stateAfter = ResourceState::kShaderResource)
	{
		return RenderTargetDesc(pView, clearValues, loadOp, storeOp, stateBefore, ResourceState::kDepthStencilWrite, stateAfter);
	}

	static constexpr RenderTargetDesc AsResolve(BvTextureView* pView, ResourceState stateBefore = ResourceState::kShaderResource, ResourceState stateAfter = ResourceState::kShaderResource)
	{
		return RenderTargetDesc(pView, ClearColorValue(), LoadOp::kClear, StoreOp::kStore, stateBefore, ResourceState::kTransferDst, stateAfter);
	}

	BvTextureView* m_pView = nullptr;
	ClearColorValue m_ClearValues = ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	LoadOp m_LoadOp = LoadOp::kClear;
	StoreOp m_StoreOp = StoreOp::kStore;
	ResourceState m_StateBefore = ResourceState::kCommon;
	ResourceState m_State = ResourceState::kRenderTarget;
	ResourceState m_StateAfter = ResourceState::kShaderResource;
	ResolveMode m_ResolveMode = ResolveMode::kNone;
	u32 m_ShadingRateTexelSizes[2]{};
};


struct RenderPassTargetDesc
{
	constexpr RenderPassTargetDesc(BvTextureView* pView)
		: m_pView(pView) {}

	constexpr RenderPassTargetDesc(BvTextureView* pView, float r, float g, float b, float a = 1.0f)
		: m_pView(pView), m_ClearValues(r, g, b, a) {}

	constexpr RenderPassTargetDesc(BvTextureView* pView, const float* pColors)
		: m_pView(pView), m_ClearValues(pColors) {}

	constexpr RenderPassTargetDesc(BvTextureView* pView, float depth, u8 stencil)
		: m_pView(pView), m_ClearValues(depth, stencil) {}

	BvTextureView* m_pView = nullptr;
	ClearColorValue m_ClearValues = ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
};


enum class QueryType : u8
{
	kTimestamp,
	kOcclusion,
	kOcclusionBinary,
};
constexpr u32 kQueryTypeCount = 3;


enum class PredicationOp : u8
{
	kEqualZero,
	kNotEqualZero
};


enum class InputRate : u8
{
	kPerVertex,
	kPerInstance,
};


enum class Topology : u8
{
	kUndefined,
	kPointList,
	kLineList,
	kLineStrip,
	kTriangleList,
	kTriangleStrip,
	kLineListAdj,
	kLineStripAdj,
	kTriangleListAdj,
	kTriangleStripAdj,
};


enum class FillMode : u8
{
	kSolid,
	kWireframe,
};


enum class CullMode : u8
{
	kNone,
	kFront,
	kBack,
	kFrontAndBack,
};


enum class FrontFace : u8
{
	kClockwise,
	kCounterClockwise,
};


enum class StencilOp : u8
{
	kKeep,
	kZero,
	kReplace,
	kIncrSat,
	kDecrSat,
	kInvert,
	kIncrWrap,
	kDecrWrap,
};


enum class BlendFactor : u8
{
	kZero,
	kOne,
	kSrcColor,
	kInvSrcColor,
	kDstColor,
	kInvDstColor,
	kSrcAlpha,
	kInvkSrcAlpha,
	kDstAlpha,
	kInvDstAlpha,
	kBlendFactor,
	kInvBlendFactor,
	kAlphaFactor,
	kInvAlphaFactor,
	kSrcAlphaSat,
	kSrc1Color,
	kInvSrc1Color,
	kSrc1Alpha,
	kInvkSrc1Alpha,
};


enum class BlendOp : u8
{
	kAdd,
	kSubtract,
	kRevSubtract,
	kMin,
	kMax
};


enum class LogicOp : u8
{
	kClear,
	kSet,
	kCopy,
	kCopyInverted,
	kNoOp,
	kInvert,
	kAnd,
	kNand,
	kOr,
	kNor,
	kXor,
	kEquiv,
	kAndReverse,
	kAndInverted,
	kOrReverse,
	kOrInverted
};


enum class ColorMask : u8
{
	kNone = 0,
	kRed = BvBit(0),
	kGreen = BvBit(1),
	kBlue = BvBit(2),
	kAlpha = BvBit(3),
	kAll = kRed | kGreen | kBlue | kAlpha
};


enum class ShaderLanguage : u8
{
	kUnknown,
	kGLSL,
	kHLSL,
};


//Vulkan 1.0 supports SPIR - V 1.0
//Vulkan 1.1 supports SPIR - V 1.3 and below
//Vulkan 1.2 supports SPIR - V 1.5 and below
//Vulkan 1.3 supports SPIR - V 1.6 and below
enum class ShaderTarget : u8
{
	kUnknown,

	// OpenGL 4.5
	kOpenGL_4_5,

	// SPIR-V
	kSPIRV_1_0,
	kSPIRV_1_1,
	kSPIRV_1_2,
	kSPIRV_1_3,
	kSPIRV_1_4,
	kSPIRV_1_5,
	kSPIRV_1_6,

	// HLSL Shader Model
	kHLSL_5_0,
	kHLSL_5_1,
	kHLSL_6_0,
	kHLSL_6_1,
	kHLSL_6_2,
	kHLSL_6_3,
	kHLSL_6_4,
	kHLSL_6_5,
	kHLSL_6_6,
	kHLSL_6_7,
	kHLSL_6_8,
};


struct ShaderCreateDesc
{
	const u8* m_pByteCode = nullptr;
	size_t m_ByteCodeSize = 0;
	const char* m_pSourceCode = nullptr;
	size_t m_SourceCodeSize = 0;
	class IBvShaderCompiler* pShaderCompiler = nullptr;
	const char* m_pEntryPoint = "main";
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
	ShaderLanguage m_ShaderLanguage = ShaderLanguage::kUnknown;
	ShaderTarget m_ShaderTarget = ShaderTarget::kUnknown;
};


struct ShaderByteCodeDesc
{
	const u8* m_pByteCode = nullptr;
	size_t m_ByteCodeSize = 0;
	const char* m_pEntryPoint = "main";
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
};


struct VertexInputDesc
{
	static constexpr u32 kAutoOffset = kU32Max;

	// Element name
	const char* m_pName = nullptr;
	// Index of the input attribute (In Vulkan and OpenGL, this maps to the location index that an input variable is.
	// In D3D this is only used if there are multiple elements with the same semantic.
	u32			m_Binding = 0;
	// The offset of this variable in the struct
	u32			m_Offset = 0;
	// Variable format
	Format		m_Format = Format::kUnknown;
	// The input rate, can be per vertex or per instance
	InputRate	m_InputRate = InputRate::kPerVertex;
	// The instance rate / step, which determines how many instances will be drawn with the same per-instance
	// data before moving to the next one - this should be 0 if the input rate is InputRate::kPerVertex
	u32			m_InstanceRate = 0;

	bool operator==(const VertexInputDesc& rhs) const
	{
		return m_pName == m_pName && m_Binding == rhs.m_Binding && m_Offset == rhs.m_Offset && m_Format == rhs.m_Format
			&& m_InstanceRate == rhs.m_InstanceRate && m_InputRate == rhs.m_InputRate;
	}
};


struct InputAssemblyStateDesc
{
	Topology	m_Topology = Topology::kTriangleList;
	bool		m_PrimitiveRestart = false;
	IndexFormat m_IndexFormatForPrimitiveRestart = IndexFormat::kU16;
};


struct TessellationStateDesc
{
	u8 m_PatchControlPoints = 0;
};


struct ViewportStateDesc
{
	u8 m_MaxViewportCount = 1;
	u8 m_MaxScissorCount = 1;
};


struct RasterizerStateDesc
{
	FillMode m_FillMode = FillMode::kSolid;
	CullMode m_CullMode = CullMode::kNone;
	FrontFace m_FrontFace = FrontFace::kClockwise;
	bool m_EnableDepthBias = false;
	float m_DepthBias = 0.0f;
	float m_DepthBiasClamp = 0.0f;
	float m_DepthBiasSlope = 0.0f;
	bool m_EnableDepthClip = false;
	bool m_EnableConservativeRasterization = false;
};


struct StencilDesc
{
	StencilOp m_StencilFailOp = StencilOp::kKeep;
	StencilOp m_StencilDepthFailOp = StencilOp::kKeep;
	StencilOp m_StencilPassOp = StencilOp::kKeep;
	CompareOp m_StencilFunc = CompareOp::kNever;
};


struct DepthStencilDesc
{
	bool m_DepthTestEnable = false;
	bool m_DepthWriteEnable = false;
	CompareOp m_DepthOp = CompareOp::kNever;
	bool m_StencilTestEnable = false;
	u8 m_StencilReadMask = 0;
	u8 m_StencilWriteMask = 0;
	bool m_DepthBoundsTestEnable = false;
	StencilDesc m_StencilFront{};
	StencilDesc m_StencilBack{};
};


struct BlendAttachmentStateDesc
{
	bool m_BlendEnable = false;
	BlendFactor m_SrcBlend = BlendFactor::kZero;
	BlendFactor m_DstBlend = BlendFactor::kZero;
	BlendOp m_BlendOp = BlendOp::kAdd;
	BlendFactor m_SrcBlendAlpha = BlendFactor::kZero;
	BlendFactor m_DstBlendAlpha = BlendFactor::kZero;
	BlendOp m_AlphaBlendOp = BlendOp::kAdd;
	u8 m_RenderTargetWriteMask = 0xF;
};


struct BlendStateDesc
{
	BlendAttachmentStateDesc m_BlendAttachments[kMaxRenderTargets];
	LogicOp m_LogicOp = LogicOp::kClear;
	bool m_LogicEnable = false;
	bool m_AlphaToCoverageEnable = false;
};