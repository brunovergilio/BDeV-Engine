#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvObject.h"
#include <cmath>


class BvBuffer;
class BvTexture;
class BvTextureView;
class BvCommandContext;
class BvRenderDevice;


class IBvRenderDeviceChild
{
public:
	virtual ~IBvRenderDeviceChild() {}
	virtual BvRenderDevice* GetDevice() = 0;
};


constexpr u32 kMaxRenderTargets = 8;
constexpr u32 kMaxRenderTargetsWithDepth = kMaxRenderTargets + 1;
constexpr u32 kMaxShaderStages = 5;
constexpr u32 kMaxVertexAttributes = 16;
constexpr u32 kMaxVertexBindings = 16;
constexpr u32 kMaxVertexBuffers = 16;


enum class QueueFamilyType : u8
{
	kGraphics,
	kCompute,
	kTransfer,
};


enum class CommandType : u8
{
	kPrimary,
	kSecondary
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


struct FormatInfo
{
	u8 m_ElementCount : 3;
	u8 m_ElementSize : 5;
	u8 m_BlockWidth : 3;
	u8 m_BlockHeight : 3;
	bool m_IsCompressed : 1;
	bool m_IsSRGBFormat : 1;
	Format m_SRGBVariant;
	Format m_NonSRGBVariant;
};


constexpr FormatInfo GetFormatInfo(Format format)
{
	constexpr FormatInfo kFormatInfos[] =
	{
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kUnknown
		{ 4, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA32_Typeless
		{ 4, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA32_Float
		{ 4, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA32_UInt
		{ 4, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA32_SInt
		{ 3, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB32_Typeless
		{ 3, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB32_Float
		{ 3, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB32_UInt
		{ 3, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB32_SInt
		{ 4, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA16_Typeless
		{ 4, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA16_Float
		{ 4, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA16_UNorm
		{ 4, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA16_UInt
		{ 4, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA16_SNorm
		{ 4, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA16_SInt
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG32_Typeless
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG32_Float
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG32_UInt
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG32_SInt
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR32G8X24_Typeless
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kD32_Float_S8X24_UInt
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR32_Float_X8X24_Typeless
		{ 2, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kX32_Typeless_G8X24_UInt
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB10A2_Typeless
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB10A2_UNorm
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB10A2_UInt
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG11B10_Float
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA8_Typeless
		{ 4, 1, 1, 1, false, false, Format::kRGBA8_UNorm_SRGB, Format::kRGBA8_UNorm },// kRGBA8_UNorm
		{ 4, 1, 1, 1, false, true, Format::kRGBA8_UNorm_SRGB, Format::kRGBA8_UNorm },// kRGBA8_UNorm_SRGB
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA8_UInt
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA8_SNorm
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGBA8_SInt
		{ 2, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG16_Typeless
		{ 2, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG16_Float
		{ 2, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG16_UNorm
		{ 2, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG16_UInt
		{ 2, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG16_SNorm
		{ 2, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG16_SInt
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR32_Typeless
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kD32_Float
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR32_Float
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR32_UInt
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR32_SInt
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR24G8_Typeless
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kD24_UNorm_S8_UInt
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR24_UNorm_X8_Typeless
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kX24_Typeless_G8_UInt
		{ 2, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG8_Typeless
		{ 2, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG8_UNorm
		{ 2, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG8_UInt
		{ 2, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG8_SNorm
		{ 2, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG8_SInt
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR16_Typeless
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR16_Float
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kD16_UNorm
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR16_UNorm
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR16_UInt
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR16_SNorm
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR16_SInt
		{ 1, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR8_Typeless
		{ 1, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR8_UNorm
		{ 1, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR8_UInt
		{ 1, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR8_SNorm
		{ 1, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR8_SInt
		{ 1, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kA8_UNorm
		{ 1, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kR1_UNorm
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB9E5_SHAREDEXP
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRG8_BG8_UNorm
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kGR8_GB8_UNorm
		{ 3, 8, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC1_Typeless
		{ 3, 8, 4, 4, true, false, Format::kBC1_UNorm_SRGB, Format::kBC1_UNorm },// kBC1_UNorm
		{ 3, 8, 4, 4, true, true, Format::kBC1_UNorm_SRGB, Format::kBC1_UNorm },// kBC1_UNorm_SRGB
		{ 4, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC2_Typeless
		{ 4, 16, 4, 4, true, false, Format::kBC2_UNorm_SRGB, Format::kBC2_UNorm },// kBC2_UNorm
		{ 4, 16, 4, 4, true, true, Format::kBC2_UNorm_SRGB, Format::kBC2_UNorm },// kBC2_UNorm_SRGB
		{ 4, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC3_Typeless
		{ 4, 16, 4, 4, true, false, Format::kBC3_UNorm_SRGB, Format::kBC3_UNorm },// kBC3_UNorm
		{ 4, 16, 4, 4, true, true, Format::kBC3_UNorm_SRGB, Format::kBC3_UNorm },// kBC3_UNorm_SRGB
		{ 1, 8, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC4_Typeless
		{ 1, 8, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC4_UNorm
		{ 1, 8, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC4_SNorm
		{ 2, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC5_Typeless
		{ 2, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC5_UNorm
		{ 2, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC5_SNorm
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kB5G6R5_UNorm
		{ 1, 2, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kBGR5A1_UNorm
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kBGRA8_UNorm
		{ 4, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kBGRX8_UNorm
		{ 1, 4, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kRGB10_XR_BIAS_A2_UNorm
		{ 4, 1, 1, 1, false, false, Format::kBGRA8_UNorm_SRGB, Format::kBGRA8_Typeless },// kBGRA8_Typeless
		{ 4, 1, 1, 1, false, true, Format::kBGRA8_UNorm_SRGB, Format::kBGRA8_Typeless },// kBGRA8_UNorm_SRGB
		{ 4, 1, 1, 1, false, false, Format::kBGRX8_UNorm_SRGB, Format::kBGRX8_Typeless },// kBGRX8_Typeless
		{ 4, 1, 1, 1, false, true, Format::kBGRX8_UNorm_SRGB, Format::kBGRX8_Typeless },// kBGRX8_UNorm_SRGB
		{ 3, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC6H_Typeless
		{ 3, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC6H_UF16
		{ 3, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC6H_SF16
		{ 4, 16, 4, 4, true, false, Format::kUnknown, Format::kUnknown },// kBC7_Typeless
		{ 4, 16, 4, 4, true, false, Format::kBC7_UNorm_SRGB, Format::kBC7_UNorm },// kBC7_UNorm
		{ 4, 16, 4, 4, true, true, Format::kBC7_UNorm_SRGB, Format::kBC7_UNorm },// kBC7_UNorm_SRGB
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kAYUV
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kY410
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kY416
		{ 3, 1, 1, 1, false, false, Format::kUnknown, Format::kUnknown },// kNV12
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kP010
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kP016
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// k420_OPAQUE
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kYUY2
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kY210
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kY216
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kNV11
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kAI44
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kIA44
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kP8
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kA8P8
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kBGRA4_UNorm
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// Undefined
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kP208
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kV208
		{ 0, 0, 0, 0, false, false, Format::kUnknown, Format::kUnknown },// kV408
	};

	return kFormatInfos[static_cast<u32>(format)];
}


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
	kUniformBuffer = BvBit(1),
	kStorageBuffer = BvBit(2),
	kUniformTexelBuffer = BvBit(3),
	kStorageTexelBuffer = BvBit(4),
	kIndexBuffer = BvBit(5),
	kVertexBuffer = BvBit(6),
	kIndirectBuffer = BvBit(7),
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
	kDefault = 0x0,
	kTransferSrc = 0x1,
	kTransferDst = 0x2,
	kShaderResource = 0x4,
	kUnorderedAccess = 0x8,
	kColorTarget = 0x10,
	kDepthStencilTarget = 0x20,
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
	u8 m_MipLevels = 1;
	u8 m_LayerCount = 1;
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


struct TextureSubresource
{
	u32 m_Width;
	u32 m_Height;
	u32 m_Detph;
	u32 m_NumRows;
	u64 m_RowPitch;
	u64 m_SlicePitch;
	u64 m_MipSize;
};


struct SubresourceFootprint
{
	u64 m_Offset;
	TextureSubresource m_Subresource;
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
	QueueFamilyType m_SrcQueue = QueueFamilyType::kGraphics;
	QueueFamilyType m_DstQueue = QueueFamilyType::kGraphics;

	ResourceAccess m_SrcAccess = ResourceAccess::kAuto;
	ResourceAccess m_DstAccess = ResourceAccess::kAuto;

	PipelineStage m_SrcPipelineStage = PipelineStage::kAuto;
	PipelineStage m_DstPipelineStage = PipelineStage::kAuto;

	SubresourceDesc m_Subresource;
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
	// Index of the input attribute (In Vulkan and OpenGL, this maps to the location index that an input variable is.
	// In D3D this is only used if there are multiple elements with the same semantic.
	u32			m_Location = 0;
	// Buffer slot this attribute refers to
	u32			m_Binding = 0;
	// The stride of the variable or struct used to represent this element
	u32			m_Stride = 0;
	// The offset of this variable in the struct
	u32			m_Offset = 0;
	// The instance rate / step, which determines how many instances will be drawn with the same per-instance
	// data before moving to the next one - this should be 0 if the input rate is InputRate::kPerVertex
	u32			m_InstanceRate = 0;
	// The input rate, can be per vertex or per instance
	InputRate	m_InputRate = InputRate::kPerVertex;
	// Variable format
	Format		m_Format = Format::kUnknown;
	// Determines whether the variable data is normalized ([-1, 1] for signed types, [0, 1] for unsigned types)
	bool		m_Normalized = false;

	bool operator==(const VertexInputDesc& rhs) const
	{
		return m_Location == rhs.m_Location && m_Binding == rhs.m_Binding && m_Stride == rhs.m_Stride
			&& m_Offset == rhs.m_Offset && m_InputRate == rhs.m_InputRate && m_InstanceRate == rhs.m_InstanceRate
			&& m_Format == rhs.m_Format && m_Normalized == rhs.m_Normalized;
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