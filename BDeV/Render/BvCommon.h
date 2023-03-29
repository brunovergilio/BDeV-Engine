#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvEnum.h"
#include "BDeV/Utils/BvUtils.h"


constexpr u32 kMaxRenderTargets = 8;
constexpr u32 kMaxRenderTargetsWithDepth = kMaxRenderTargets + 1;


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

enum class MemoryFlags : u8
{
	kDeviceLocal	= 0x1,
	kHostVisible	= 0x2,
	kHostCoherent	= 0x4,
	kUpload			= kHostVisible | kHostCoherent,
	kHostCached		= 0x8,
	kReadBack		= kHostVisible | kHostCached | kHostCoherent,
};
BV_USE_ENUM_CLASS_OPERATORS(MemoryFlags);


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
	kNV12 = 103,
	kP010 = 104,
	kP016 = 105,
	k420_OPAQUE = 106,
	kYUY2 = 107,
	kY210 = 108,
	kY216 = 109,
	kNV11 = 110,
	kAI44 = 111,
	kIA44 = 112,
	kP8 = 113,
	kA8P8 = 114,
	kBGRA4_UNorm = 115,
	kP208 = 130,
	kV208 = 131,
	kV408 = 132,
};


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
	kAll					= BvBit(8) - 1,
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

	// For automatic features
	kUnknown,
};


enum class ResourceAccess : u32
{
	kNone				= 0,
	kIndirectRead		= BvBit(0),
	kIndexRead			= BvBit(1),
	kVertexInputRead	= BvBit(2),
	kUniformRead		= BvBit(3),
	kShaderRead			= BvBit(4),
	kShaderWrite		= BvBit(5),
	kRenderTargetRead	= BvBit(6),
	kRenderTargetWrite	= BvBit(7),
	kDepthStencilRead	= BvBit(8),
	kDepthStencilWrite	= BvBit(9),
	kTransferRead		= BvBit(10),
	kTransferWrite		= BvBit(11),
	kHostRead			= BvBit(12),
	kHostWrite			= BvBit(13),
	kMemoryRead			= BvBit(14),
	kMemoryWrite		= BvBit(15),
	kAuto				= BvBit(16)
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
	kAuto						= BvBit(14)
};
BV_USE_ENUM_CLASS_OPERATORS(PipelineStage);


struct Extent
{
	union
	{
		struct 
		{
			u32 width;
			u32 height;
			u32 depthOrLayerCount;
		};
		struct 
		{
			u32 x;
			u32 y;
			u32 z;
		};
	};
};


struct SubresourceDesc
{
	u32 firstMip = 0;
	u32 mipCount = 1;
	u32 firstLayer = 0;
	u32 layerCount = 1;
};