#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/Utils/BvEnum.h"
#include "BvCore/Utils/BvUtils.h"


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


enum class Format : u8
{
	kUndefined,

	kRGBA32_Float,
	kRGBA32_UInt,
	kRGBA32_SInt,

	kRGB32_Float,
	kRGB32_UInt,
	kRGB32_SInt,

	kRG32_Float,
	kRG32_UInt,
	kRG32_SInt,

	kR32_Float,
	kR32_UInt,
	kR32_SInt,

	kRGBA16_Float,
	kRGBA16_UNorm,
	kRGBA16_UInt,
	kRGBA16_SNorm,
	kRGBA16_SInt,

	//kRGB16_Float,
	//kRGB16_UNorm,
	//kRGB16_UInt,
	//kRGB16_SNorm,
	//kRGB16_SInt,

	kRG16_Float,
	kRG16_UNorm,
	kRG16_UInt,
	kRG16_SNorm,
	kRG16_SInt,

	kR16_Float,
	kR16_UNorm,
	kR16_UInt,
	kR16_SNorm,
	kR16_SInt,

	kRGBA8_UNormSRGB,
	kRGBA8_UNorm,
	kRGBA8_UInt,
	kRGBA8_SNorm,
	kRGBA8_SInt,

	kBGRA8_UNorm,
	kBGRA8_UNormSRGB,

	//kRGB8_UNormSRGB,
	//kRGB8_UNorm,
	//kRGB8_UInt,
	//kRGB8_SNorm,
	//kRGB8_SInt,

	//kRG8_UNormSRGB,
	kRG8_UNorm,
	kRG8_UInt,
	kRG8_SNorm,
	kRG8_SInt,

	kR8_UNorm,
	kR8_UInt,
	kR8_SNorm,
	kR8_SInt,

	kD32_Float_S8_UInt,
	kD32_Float,
	kD24_UNorm_S8_UInt,
	kD16_UNorm,

	kRGB10A2_UNorm,
	kRGB10A2_UInt,
	kRG11B10_Float,

	kBC1_UNorm,
	kBC1_UNormSRGB,
	kBC2_UNorm,
	kBC2_UNormSRGB,
	kBC3_UNorm,
	kBC3_UNormSRGB,
	kBC4_UNorm,
	kBC4_SNorm,
	kBC5_UNorm,
	kBC5_SNorm,
	kBC6H_UFloat,
	kBC6H_SFloat,
	kBC7_UNorm,
	kBC7_UNormSRGB,
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
	kUndefined,

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
	kBeginning					= BvBit(1),
	kIndirectDraw				= BvBit(2),
	kVertexInput				= BvBit(3),
	kVertexShader				= BvBit(4),
	kTessHullOrControlShader	= BvBit(5),
	kTessDomainOrEvalShader		= BvBit(6),
	kGeometryShader				= BvBit(7),
	kPixelOrFragmentShader		= BvBit(8),
	kDepth						= BvBit(9),
	kRenderTarget				= BvBit(10),
	kComputeShader				= BvBit(11),
	kTransfer					= BvBit(12),
	kHost						= BvBit(13),
	kEnd						= BvBit(14),
	kAllShaderStages			= kVertexShader | kTessHullOrControlShader | kTessDomainOrEvalShader
		| kGeometryShader | kPixelOrFragmentShader | kComputeShader,
	kAuto						= BvBit(15)
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
