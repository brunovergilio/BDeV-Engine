#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvStringId.h"
#include "BDeV/Core/Math/BvMath.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Utils/BvObject.h"
#include "BDeV/Core/Utils/BvHash.h"
#include <algorithm>


class IBvBuffer;
class IBvBufferView;
class IBvTexture;
class IBvTextureView;
class IBvSampler;
class IBvCommandContext;
class IBvAccelerationStructure;
class IBvRayTracingPipelineState;
class IBvShader;
class IBvShaderResourceLayout;
class IBvShaderBindingTable;
class IBvShaderCompiler;
class IBvRenderPass;


constexpr u32 kMaxDevices = 4;
constexpr u32 kMaxRenderTargets = 8;
constexpr u32 kMaxRenderTargetsWithDepth = kMaxRenderTargets + 1;
constexpr u32 kMaxShaderStages = 5;
constexpr u32 kMaxVertexBindings = 16;
constexpr u32 kMaxVertexBuffers = 16;
constexpr u32 kMaxContextGroupCount = 8;


enum class GPUType : u8
{
	kUnknown,
	kDiscrete,
	kIntegrated
};


enum class GPUVendor : u8
{
	kUnknown,
	kAMD,
	kImgTec,
	kNvidia,
	kARM,
	kQualcomm,
	kIntel,
};


enum RenderDeviceCapabilities : u32
{
	kNone							= 0,
	kWireframe						= BvBit(0),
	kGeometryShader					= BvBit(1),
	kTesselationShader				= BvBit(2),
	kDepthBoundsTest				= BvBit(3),
	kTimestampQueries				= BvBit(4),
	kIndirectDrawCount				= BvBit(5),
	kSamplerMinMaxReduction			= BvBit(6),
	kCustomBorderColor				= BvBit(7),
	kPredication					= BvBit(8),
	kConservativeRasterization		= BvBit(9),
	kShadingRate					= BvBit(10),
	kMeshShader						= BvBit(11),
	kMeshQuery						= BvBit(12),
	kRayTracing						= BvBit(13),
	kRayQuery						= BvBit(14),
	kMultiView						= BvBit(15),
	kTrueFullScreen					= BvBit(16),
};
BV_USE_ENUM_CLASS_OPERATORS(RenderDeviceCapabilities);


enum class CommandType : u8
{
	kNone,
	kGraphics,
	kCompute,
	kTransfer,
	kVideoDecode,
	kVideoEncode
};


struct ContextGroup
{
	u32 m_GroupIndex;
	u32 m_MaxContextCount;
	CommandType m_DedicatedCommandType;
	BvFixedVector<CommandType, 5> m_SupportedCommandTypes;

	bool SupportsCommandType(CommandType commandType) const { return m_SupportedCommandTypes.Contains(commandType); }
};


struct BvGPUInfo
{
	static constexpr u32 kMaxDeviceNameSize = 256;

	char m_DeviceName[kMaxDeviceNameSize];
	u32 m_DeviceId;
	u32 m_VendorId;
	u64 m_DeviceMemory;
	u32 m_GraphicsContextCount;
	u32 m_ComputeContextCount;
	u32 m_TransferContextCount;
	BvFixedVector<ContextGroup, kMaxContextGroupCount> m_ContextGroups;
	GPUType m_Type;
	GPUVendor m_Vendor;
	RenderDeviceCapabilities m_DeviceCaps;
};


enum class IndexFormat : u8
{
	kUnknown,
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
	u8 m_BitsPerPixel;
	u8 m_ElementCount : 3;
	u8 m_ElementSize : 5;
	u8 m_BlockWidth : 3;
	u8 m_BlockHeight : 3;
	u8 m_BlocksPerElement : 5;
	Format m_SRGBOrLinearVariant;
	bool m_IsSRGBFormat : 1;
	bool m_IsDepthStencil : 1;
	bool m_IsCompressed : 1;
	bool m_IsPacked : 1;
	bool m_IsPlanar : 1;
	u8 m_PlaneCount : 2;
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
		{ 16, 3, 1, 2, 2, 1, Format::kUnknown, false, false, false, false, true, 0 },// kV208
		{ 24, 3, 1, 1, 1, 1, Format::kUnknown, false, false, false, false, true, 0 },// kV408
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
	kNone,
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
	kRayGen					= BvBit(8),
	kAnyHit					= BvBit(9),
	kClosestHit				= BvBit(10),
	kMiss					= BvBit(11),
	kIntersection			= BvBit(12),
	kCallable				= BvBit(13),
	kAllStages				= BvBit(14) - 1,
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
	kConstantBuffer,

	// States used by buffers and textures
	kShaderResource,
	kPixelShaderResource,
	kRWResource,
	kTransferSrc,
	kTransferDst,

	// States only used by textures
	kRenderTarget,
	kDepthStencilRead,
	kDepthStencilWrite,
	kPresent,
	kResolveSrc,
	kResolveDst,

	// Extensions
	kPredication,
	kShadingRate,
	kASBuildRead,
	kASBuildWrite,
	kASPostBuildBuffer,
};


enum class ResourceAccess : u32
{
	kNone						= 0,
	kIndirectRead				= BvBit(0),
	kIndexRead					= BvBit(1),
	kVertexInputRead			= BvBit(2),
	kUniformRead				= BvBit(3),
	kShaderRead					= BvBit(4),
	kShaderWrite				= BvBit(5),
	kRenderTargetRead			= BvBit(6),
	kRenderTargetWrite			= BvBit(7),
	kDepthStencilRead			= BvBit(8),
	kDepthStencilWrite			= BvBit(9),
	kTransferRead				= BvBit(10),
	kTransferWrite				= BvBit(11),
	kHostRead					= BvBit(12),
	kHostWrite					= BvBit(13),
	kMemoryRead					= BvBit(14),
	kMemoryWrite				= BvBit(15),
	kInputAttachmentRead		= BvBit(16),
	kShadingRateRead			= BvBit(17),
	kPredicationRead			= BvBit(18),
	kAccelerationStructureRead	= BvBit(19),
	kAccelerationStructureWrite	= BvBit(20),
	kShaderBindingTableRead		= BvBit(21),
	kAccelerationStructurePostBuildWrite = BvBit(22),
	kAuto						= kU32Max
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
	kShadingRate				= BvBit(14),
	kPredication				= BvBit(15),
	kMesh						= BvBit(16),
	kAmplificationOrTask		= BvBit(17),
	kAccelerationStructureBuild	= BvBit(18),
	kAccelerationStructureCopy	= BvBit(19),
	kRayTracing					= BvBit(20),
	kAllShaderStages			= kVertexShader | kTessHullOrControlShader | kTessDomainOrEvalShader
		| kGeometryShader | kPixelOrFragmentShader | kComputeShader | kMesh | kAmplificationOrTask | kRayTracing,
	kAuto						= kU32Max
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


enum class SwapChainMode : u8
{
	kWindowed,
	kBorderlessFullscreen,
	kFullscreen
};


struct SwapChainDesc
{
	u32 m_SwapChainImageCount = 3;
	Format m_Format = Format::kUnknown;
	bool m_VSync = false;
	bool m_PreferHDR = false;
	SwapChainMode m_WindowMode = SwapChainMode::kWindowed;
};


enum class BufferUsage : u16
{
	kNone =						0,
	kConstantBuffer =			BvBit(0),
	kStructuredBuffer =			BvBit(1),
	kRWStructuredBuffer =		BvBit(2),
	kVertexBuffer =				BvBit(3),
	kIndexBuffer =				BvBit(4),
	kIndirectBuffer =			BvBit(5),
	kRayTracing =				BvBit(6),
};
BV_USE_ENUM_CLASS_OPERATORS(BufferUsage);


enum class BufferCreateFlags : u8
{
	kNone = 0,
	kCreateMapped = BvBit(0),
};
BV_USE_ENUM_CLASS_OPERATORS(BufferCreateFlags);


struct BufferDesc
{
	u64 m_Size = 0;
	BufferCreateFlags m_CreateFlags = BufferCreateFlags::kNone;
	BufferUsage m_UsageFlags = BufferUsage::kNone;
	MemoryType m_MemoryType = MemoryType::kDevice;
	ResourceState m_ResourceState = ResourceState::kCommon;
	bool m_Formatted = false;
};


struct BufferViewDesc
{
	IBvBuffer* m_pBuffer = nullptr;
	u64 m_Offset = 0;
	u32 m_ElementCount = 0;
	u32 m_Stride = 0;
	Format m_Format = Format::kUnknown;
};


struct VertexBufferView
{
	IBvBuffer* m_pBuffer = nullptr;
	u64 m_Offset = 0;
	u32 m_Stride = 0;
};


struct IndexBufferView
{
	IBvBuffer* m_pBuffer = nullptr;
	u64 m_Offset = 0;
	IndexFormat m_IndexFormat = IndexFormat::kUnknown;
};


enum class TextureType : u8
{
	kTexture1D,
	kTexture2D,
	kTexture3D,
};


enum class TextureUsage : u8
{
	kNone =					0,
	kRenderTarget =			BvBit(0),
	kDepthStencilTarget =	BvBit(1),
	kShaderResource =		BvBit(2),
	kUnorderedAccess =		BvBit(3),
	kInputAttachment =		BvBit(4),
	kShadingRate =			BvBit(5)
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
	TextureUsage m_UsageFlags = TextureUsage::kNone;
	MemoryType m_MemoryType = MemoryType::kDevice;
	ResourceState m_ResourceState = ResourceState::kCommon;
};


enum class TextureViewType : u8
{
	kUnknown,
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
	u32 planeSlice = 0;
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
	IBvCommandContext* m_pContext = nullptr;
	const void* m_pData = nullptr;
	u64 m_Size = 0;
};


struct TextureInitData
{
	IBvCommandContext* m_pContext = nullptr;
	u32 m_SubresourceCount = 0;
	const SubresourceData* m_pSubresources = nullptr;
};


struct TextureViewDesc
{
	IBvTexture* m_pTexture = nullptr;
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

	IBvTexture* m_pTexture = nullptr;
	IBvBuffer* m_pBuffer = nullptr;

	ResourceState m_SrcState = ResourceState::kCommon;
	ResourceState m_DstState = ResourceState::kCommon;

	Type m_Type = Type::kStateTransition;

	IBvCommandContext* m_pSrcContext = nullptr;
	IBvCommandContext* m_pDstContext = nullptr;

	// These fields can be used for more detailed barriers (in Vulkan)
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
	kDynamicConstantBuffer,
	kDynamicStructuredBuffer,
	kDynamicRWStructuredBuffer,
	kFormattedBuffer,
	kRWFormattedBuffer,
	kTexture,
	kRWTexture,
	kSampler,
	kInputAttachment,
	kAccelerationStructure,
};


// HLSL Type					DirectX Descriptor Type		Vulkan Descriptor Type		GLSL Type
// SamplerState					Sampler						Sampler						uniform sampler*
// SamplerComparisonState		Sampler						Sampler						uniform sampler*Shadow
// Buffer						SRV							Uniform Texel Buffer		uniform samplerBuffer
// RWBuffer						UAV							Storage Texel Buffer		uniform imageBuffer
// Texture*						SRV							Sampled Image				uniform texture*
// RWTexture*					UAV							Storage Image				uniform image*
// cbuffer						CBV							Uniform Buffer				uniform{ ... }
// ConstantBuffer				CBV							Uniform Buffer				uniform{ ... }
// tbuffer						SRV							Uniform Texel Buffer		uniform samplerBuffer
// TextureBuffer				SRV							Uniform Texel Buffer		uniform samplerBuffer
// StructuredBuffer				SRV							Storage Buffer				buffer{ ... }
// RWStructuredBuffer			UAV							Storage Buffer				buffer{ ... }
// ByteAddressBuffer			SRV							Storage Buffer				-
// RWByteAddressBuffer			UAV							Storage Buffer				-
// AppendStructuredBuffer		UAV							Storage Buffer				-
// ConsumeStructuredBuffer		UAV							Storage Buffer				-


struct ShaderResourceDesc
{
	BvStringId m_Name;
	u32 m_Binding;
	u32 m_Count;
	ShaderResourceType m_ShaderResourceType;
	ShaderStage m_ShaderStages;
	const IBvSampler* const* m_ppStaticSamplers;
	bool m_Bindless;

	template<ShaderResourceType Type>
	static ShaderResourceDesc As(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ name, binding, count, Type, shaderStages, nullptr }; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ BvStringId::Empty(), binding, count, Type, shaderStages, nullptr }; }

	static ShaderResourceDesc AsConstantBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kConstantBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kRWStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsDynamicConstantBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kDynamicConstantBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsDynamicStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kDynamicStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsDynamicRWStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kDynamicRWStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsFormattedBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWFormattedBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kRWFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsTexture(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWTexture(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kRWTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsSampler(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kSampler, shaderStages, nullptr }; }
	static ShaderResourceDesc AsInputAttachment(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kInputAttachment, shaderStages, nullptr }; }
	static ShaderResourceDesc AsAccelerationStructure(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kAccelerationStructure, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStaticSampler(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kSampler, shaderStages, ppSamplers }; }

	static ShaderResourceDesc AsConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsDynamicConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsDynamicConstantBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsDynamicStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsDynamicStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsDynamicRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsDynamicRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsInputAttachment(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsInputAttachment(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsAccelerationStructure(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsAccelerationStructure(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(BvStringId::Empty(), binding, shaderStages, count, ppSamplers); }

	friend bool operator<(const ShaderResourceDesc& lhs, const ShaderResourceDesc& rhs)
	{
		return lhs.m_Binding < rhs.m_Binding;
	}

	friend bool operator==(const ShaderResourceDesc& lhs, const ShaderResourceDesc& rhs)
	{
		return lhs.m_Name == rhs.m_Name && lhs.m_Binding == rhs.m_Binding && lhs.m_Count == rhs.m_Count && lhs.m_ShaderResourceType == rhs.m_ShaderResourceType
			&& lhs.m_ShaderStages == rhs.m_ShaderStages && lhs.m_ppStaticSamplers == rhs.m_ppStaticSamplers && lhs.m_Bindless == rhs.m_Bindless;
	}
};


struct ShaderResourceConstantDesc
{
	BvStringId m_Name;
	u32 m_Binding;
	u32 m_Size;
	ShaderStage m_ShaderStages;

	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, binding, sizeof(T), shaderStages }; }
	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId name, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, 0, sizeof(T), shaderStages }; }
	template<typename T>
	static ShaderResourceConstantDesc As(ShaderStage shaderStages = ShaderStage::kAllStages) { return { nullptr, 0, sizeof(T), shaderStages }; }

	friend bool operator<(const ShaderResourceConstantDesc& lhs, const ShaderResourceConstantDesc& rhs)
	{
		return lhs.m_Binding < rhs.m_Binding;
	}

	friend bool operator==(const ShaderResourceConstantDesc& lhs, const ShaderResourceConstantDesc& rhs)
	{
		return lhs.m_Name == rhs.m_Name && lhs.m_Binding == rhs.m_Binding && lhs.m_Size == rhs.m_Size && lhs.m_ShaderStages == rhs.m_ShaderStages;
	}
};


struct ShaderResourceSetDesc
{
	u32 m_Index;
	u32 m_ResourceCount;
	const ShaderResourceDesc* m_pResources;
	bool m_Bindless;
	u32 m_ConstantCount;
	const ShaderResourceConstantDesc* m_pConstants;

	friend bool operator<(const ShaderResourceSetDesc& lhs, const ShaderResourceSetDesc& rhs)
	{
		return lhs.m_Index < rhs.m_Index;
	}
};


struct ShaderResourceLayoutDesc
{
	u32 m_ShaderResourceSetCount = 0;
	const ShaderResourceSetDesc* m_pShaderResourceSets = nullptr;
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


enum class ReductionMode : u8
{
	kStandard,
	kMin,
	kMax
};


struct SamplerDesc
{
	Filter			m_MagFilter = Filter::kLinear;
	Filter			m_MinFilter = Filter::kLinear;
	MipMapFilter	m_MipmapMode = MipMapFilter::kLinear;
	AddressMode		m_AddressModeU = AddressMode::kWrap;
	AddressMode		m_AddressModeV = AddressMode::kWrap;
	AddressMode		m_AddressModeW = AddressMode::kWrap;
	CompareOp		m_CompareOp = CompareOp::kNone;
	ReductionMode	m_ReductionMode = ReductionMode::kStandard;
	bool			m_AnisotropyEnable = false;
	float			m_MaxAnisotropy = 1.0f;
	float			m_MipLodBias = 0.0f;
	float			m_MinLod = 0.0f;
	float			m_MaxLod = kF32Max;
	float			m_BorderColor[4]{};
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
	constexpr RenderTargetDesc() = default;

	constexpr RenderTargetDesc(IBvTextureView* pView, const ClearColorValue& clearValues, LoadOp loadOp, StoreOp storeOp,
		ResourceState stateBefore, ResourceState state, ResourceState stateAfter)
		: m_pView(pView), m_ClearValues(clearValues), m_LoadOp(loadOp), m_StoreOp(storeOp),
		m_StateBefore(stateBefore), m_State(state), m_StateAfter(stateAfter)
	{
	}

	constexpr RenderTargetDesc(IBvTextureView* pView, const ClearColorValue& clearValues, LoadOp loadOp, StoreOp storeOp,
		ResourceState stateBefore, ResourceState state, ResourceState stateAfter, ResolveMode resolveMode)
		: m_pView(pView), m_ClearValues(clearValues), m_LoadOp(loadOp), m_StoreOp(storeOp),
		m_StateBefore(stateBefore), m_State(state), m_StateAfter(stateAfter), m_ResolveMode(resolveMode)
	{
	}

	static constexpr RenderTargetDesc AsRenderTarget(IBvTextureView* pView, const ClearColorValue& clearValues = ClearColorValue(0.0f, 0.0f, 0.0f), LoadOp loadOp = LoadOp::kClear,
		StoreOp storeOp = StoreOp::kStore, ResourceState stateBefore = ResourceState::kCommon, ResourceState stateAfter = ResourceState::kShaderResource)
	{
		return RenderTargetDesc(pView, clearValues, loadOp, storeOp, stateBefore, ResourceState::kRenderTarget, stateAfter);
	}

	static constexpr RenderTargetDesc AsSwapChain(IBvTextureView* pView, const ClearColorValue& clearValues = ClearColorValue(0.0f, 0.0f, 0.0f), LoadOp loadOp = LoadOp::kClear,
		StoreOp storeOp = StoreOp::kStore, ResourceState stateBefore = ResourceState::kCommon, ResourceState stateAfter = ResourceState::kPresent)
	{
		return RenderTargetDesc(pView, clearValues, loadOp, storeOp, stateBefore, ResourceState::kRenderTarget, stateAfter);
	}

	static constexpr RenderTargetDesc AsDepthStencil(IBvTextureView* pView, const ClearColorValue& clearValues = ClearColorValue(1.0f, 0), LoadOp loadOp = LoadOp::kClear,
		StoreOp storeOp = StoreOp::kStore, ResourceState stateBefore = ResourceState::kCommon, ResourceState stateAfter = ResourceState::kShaderResource)
	{
		return RenderTargetDesc(pView, clearValues, loadOp, storeOp, stateBefore, ResourceState::kDepthStencilWrite, stateAfter);
	}

	static constexpr RenderTargetDesc AsColorResolve(IBvTextureView* pView, ResolveMode resolveMode = ResolveMode::kAverage,
		ResourceState stateBefore = ResourceState::kCommon, ResourceState stateAfter = ResourceState::kRenderTarget)
	{
		return RenderTargetDesc(pView, ClearColorValue(), LoadOp::kClear, StoreOp::kStore, stateBefore, ResourceState::kRenderTarget, stateAfter, resolveMode);
	}

	static constexpr RenderTargetDesc AsDepthResolve(IBvTextureView* pView, ResolveMode resolveMode, ResourceState stateBefore = ResourceState::kCommon)
	{
		return RenderTargetDesc(pView, ClearColorValue(), LoadOp::kClear, StoreOp::kStore, stateBefore, ResourceState::kDepthStencilWrite, ResourceState::kDepthStencilWrite, resolveMode);
	}

	IBvTextureView* m_pView = nullptr;
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
	constexpr RenderPassTargetDesc() = default;

	constexpr RenderPassTargetDesc(IBvTextureView* pView)
		: m_pView(pView) {}

	constexpr RenderPassTargetDesc(IBvTextureView* pView, float r, float g, float b, float a = 1.0f)
		: m_pView(pView), m_ClearValues(r, g, b, a) {}

	constexpr RenderPassTargetDesc(IBvTextureView* pView, const float* pColors)
		: m_pView(pView), m_ClearValues(pColors) {}

	constexpr RenderPassTargetDesc(IBvTextureView* pView, float depth, u8 stencil)
		: m_pView(pView), m_ClearValues(depth, stencil) {}

	IBvTextureView* m_pView = nullptr;
	ClearColorValue m_ClearValues = ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
};


struct AttachmentRef
{
	u32 m_Index = 0;
	ResourceState m_ResourceState = ResourceState::kCommon;
	ResolveMode m_ResolveMode = ResolveMode::kNone;

	constexpr bool operator==(const AttachmentRef& rhs) const
	{
		return m_Index == rhs.m_Index && m_ResourceState == rhs.m_ResourceState && m_ResolveMode == rhs.m_ResolveMode;
	}

	constexpr bool operator!=(const AttachmentRef& rhs) const
	{
		return !(*this == rhs);
	}
};


struct ShadingRateAttachmentRef
{
	u32 m_Index = 0;
	ResourceState m_ResourceState = ResourceState::kCommon;
	u32 m_TexelSizes[2]{ 0,0 };

	constexpr bool operator==(const ShadingRateAttachmentRef& rhs) const
	{
		return m_Index == rhs.m_Index && m_ResourceState == rhs.m_ResourceState
			&& m_TexelSizes[0] == rhs.m_TexelSizes[0] && m_TexelSizes[1] == rhs.m_TexelSizes[1];
	}

	constexpr bool operator!=(const ShadingRateAttachmentRef& rhs) const
	{
		return !(*this == rhs);
	}
};


struct SubpassDesc
{
	u32 m_InputAttachmentCount = 0;
	u32 m_ColorAttachmentCount = 0;
	const AttachmentRef* m_pColorAttachments = nullptr;
	const AttachmentRef* m_pDepthStencilAttachment = nullptr;
	const AttachmentRef* m_pInputAttachments = nullptr;
	const AttachmentRef* m_pResolveAttachments = nullptr;
	const AttachmentRef* m_pDepthStencilResolveAttachment = nullptr;
	const ShadingRateAttachmentRef* m_pShadingRateAttachment = nullptr;
	u32 m_MultiviewCount = 0;
	u32 m_MultiviewCorrelationMask = 0;

	constexpr bool operator==(const SubpassDesc& rhs) const
	{
		if (m_ColorAttachmentCount != rhs.m_ColorAttachmentCount || m_InputAttachmentCount != rhs.m_InputAttachmentCount)
		{
			return false;
		}

		if ((m_pDepthStencilAttachment == nullptr && rhs.m_pDepthStencilAttachment != nullptr)
			|| (m_pDepthStencilAttachment != nullptr && rhs.m_pDepthStencilAttachment == nullptr))
		{
			return false;
		}

		if (m_pDepthStencilAttachment != nullptr && rhs.m_pDepthStencilAttachment != nullptr
			&& *m_pDepthStencilAttachment != *rhs.m_pDepthStencilAttachment)
		{
			return false;
		}

		if ((m_pResolveAttachments == nullptr && rhs.m_pResolveAttachments != nullptr)
			|| (m_pResolveAttachments != nullptr && rhs.m_pResolveAttachments == nullptr))
		{
			return false;
		}

		if (m_pResolveAttachments != nullptr && rhs.m_pResolveAttachments != nullptr
			&& *m_pResolveAttachments != *rhs.m_pResolveAttachments)
		{
			return false;
		}

		if (m_pDepthStencilResolveAttachment != nullptr && rhs.m_pDepthStencilResolveAttachment != nullptr
			&& *m_pDepthStencilResolveAttachment != *rhs.m_pDepthStencilResolveAttachment)
		{
			return false;
		}

		if ((m_pShadingRateAttachment == nullptr && rhs.m_pShadingRateAttachment != nullptr)
			|| (m_pShadingRateAttachment != nullptr && rhs.m_pShadingRateAttachment == nullptr))
		{
			return false;
		}

		if (m_pShadingRateAttachment != nullptr && rhs.m_pShadingRateAttachment != nullptr
			&& *m_pShadingRateAttachment != *rhs.m_pShadingRateAttachment)
		{
			return false;
		}

		for (auto i = 0u; i < m_ColorAttachmentCount; ++i)
		{
			if (m_pColorAttachments[i] != rhs.m_pColorAttachments[i])
			{
				return false;
			}
		}

		for (auto i = 0u; i < m_InputAttachmentCount; ++i)
		{
			if (m_pInputAttachments[i] != rhs.m_pInputAttachments[i])
			{
				return false;
			}
		}

		if (m_MultiviewCount != rhs.m_MultiviewCount || m_MultiviewCorrelationMask != rhs.m_MultiviewCorrelationMask)
		{
			return false;
		}

		return true;
	}

	constexpr bool operator!=(const SubpassDesc& rhs) const
	{
		return !(*this == rhs);
	}
};


struct SubpassDependency
{
	static constexpr auto kExternalSubpassIndex = kU32Max;

	u32 m_SrcSubpass = 0;
	u32 m_DstSubpass = 0;
	ResourceAccess m_SrcAccess = ResourceAccess::kAuto;
	ResourceAccess m_DstAccess = ResourceAccess::kAuto;
	PipelineStage m_SrcStage = PipelineStage::kAuto;
	PipelineStage m_DstStage = PipelineStage::kAuto;

	constexpr bool operator==(const SubpassDependency& rhs) const
	{
		return m_SrcSubpass == rhs.m_SrcSubpass && (u32)m_SrcAccess == (u32)rhs.m_SrcAccess && (u32)m_SrcStage == (u32)rhs.m_SrcStage
			&& m_DstSubpass == rhs.m_DstSubpass && (u32)m_DstAccess == (u32)rhs.m_DstAccess && (u32)m_DstStage == (u32)rhs.m_DstStage;
	}

	constexpr bool operator!=(const SubpassDependency& rhs) const
	{
		return !(*this == rhs);
	}
};



struct RenderPassAttachment
{
	RenderPassAttachment()
		: m_Format(Format::kUnknown), m_StateBefore(ResourceState::kCommon), m_StateAfter(ResourceState::kCommon), m_SampleCount(1),
		m_LoadOp(LoadOp::kClear), m_StoreOp(StoreOp::kStore)
	{}

	Format m_Format;
	ResourceState m_StateBefore;
	ResourceState m_StateAfter;
	u8 m_SampleCount : 5;
	LoadOp m_LoadOp : 2;
	StoreOp m_StoreOp : 1;

	constexpr bool operator==(const RenderPassAttachment& rhs) const
	{
		return m_Format == rhs.m_Format &&
			m_StateBefore == rhs.m_StateBefore &&
			m_StateAfter == rhs.m_StateAfter &&
			m_SampleCount == rhs.m_SampleCount &&
			m_LoadOp == rhs.m_LoadOp &&
			m_StoreOp == rhs.m_StoreOp;
	}

	constexpr bool operator!=(const RenderPassAttachment& rhs) const
	{
		return !(*this == rhs);
	}
};


struct RenderPassDesc
{
	const RenderPassAttachment* m_pAttachments = nullptr;
	const SubpassDesc* m_pSubpasses = nullptr;
	const SubpassDependency* m_pSubpassDependencies = nullptr;
	u32 m_AttachmentCount = 0;
	u32 m_SubpassCount = 0;
	u32 m_SubpassDependencyCount = 0;

	bool operator==(const RenderPassDesc& rhs) const noexcept
	{
		return true;
	}

	bool operator!=(const RenderPassDesc& rhs) const noexcept
	{
		return !(*this == rhs);
	}
};


template<>
struct std::hash<RenderPassDesc>
{
	size_t operator()(const RenderPassDesc& renderPassDesc) const
	{
		u64 hash = 0;
		HashCombine(hash, renderPassDesc.m_AttachmentCount);
		for (auto i = 0u; i < renderPassDesc.m_AttachmentCount; ++i)
		{
			auto& attachment = renderPassDesc.m_pAttachments[i];
			HashCombine(hash, attachment.m_Format, attachment.m_StateBefore, attachment.m_StateAfter, attachment.m_SampleCount,
				attachment.m_LoadOp, attachment.m_StoreOp);
		}

		HashCombine(hash, renderPassDesc.m_SubpassCount);
		for (auto i = 0u; i < renderPassDesc.m_SubpassCount; ++i)
		{
			auto& subpass = renderPassDesc.m_pSubpasses[i];
			HashCombine(hash, subpass.m_ColorAttachmentCount);
			for (auto j = 0u; j < subpass.m_ColorAttachmentCount; ++j)
			{
				auto& attachment = subpass.m_pColorAttachments[j];
				HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
			}

			if (subpass.m_pDepthStencilAttachment)
			{
				HashCombine(hash, subpass.m_pDepthStencilAttachment->m_Index, subpass.m_pDepthStencilAttachment->m_ResourceState);
			}

			HashCombine(hash, subpass.m_InputAttachmentCount);
			for (auto j = 0u; j < subpass.m_InputAttachmentCount; ++j)
			{
				auto& attachment = subpass.m_pInputAttachments[j];
				HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
			}

			if (subpass.m_pResolveAttachments)
			{
				for (auto j = 0u; j < subpass.m_ColorAttachmentCount; ++j)
				{
					auto& attachment = subpass.m_pResolveAttachments[j];
					HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
				}
			}

			if (subpass.m_pShadingRateAttachment)
			{
				HashCombine(hash, subpass.m_pShadingRateAttachment->m_Index, subpass.m_pShadingRateAttachment->m_ResourceState,
					subpass.m_pShadingRateAttachment->m_TexelSizes[0], subpass.m_pShadingRateAttachment->m_TexelSizes[1]);
			}
		}

		HashCombine(hash, renderPassDesc.m_SubpassDependencyCount);
		for (auto i = 0u; i < renderPassDesc.m_SubpassDependencyCount; ++i)
		{
			auto& subpassDep = renderPassDesc.m_pSubpassDependencies[i];
			HashCombine(hash, subpassDep.m_SrcSubpass, subpassDep.m_DstSubpass,
				subpassDep.m_SrcAccess, subpassDep.m_DstAccess, subpassDep.m_SrcStage, subpassDep.m_DstStage);
		}

		return hash;
	}
};


enum class QueryType : u8
{
	kTimestamp,
	kOcclusion,
	kOcclusionBinary,
	kPipelineStatistics,
	kMeshPipelineStatistics,
};
constexpr u32 kQueryTypeCount = 5;


struct PipelineStatistics
{
	u64 m_InputAssemblyVertices = 0;
	u64 m_InputAssemblyPrimitives = 0;
	u64 m_VertexShaderInvocations = 0;
	u64 m_GeometryShaderInvocations = 0;
	u64 m_GeometryShaderPrimitives = 0;
	u64 m_ClippingInvocations = 0;
	u64 m_ClippingPrimitives = 0;
	u64 m_PixelOrFragmentShaderInvocations = 0;
	u64 m_HullOrControlShaderInvocations = 0;
	u64 m_DomainOrEvaluationShaderInvocations = 0;
	u64 m_ComputeShaderInvocations = 0;
	u64 m_TaskOrAmplificationShaderInvocations = 0;
	u64 m_MeshShaderInvocations = 0;
	u64 m_MeshShaderPrimitives = 0;
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
	kTriangleFan,
	kLineListAdj,
	kLineStripAdj,
	kTriangleListAdj,
	kTriangleStripAdj,
	kPatchList,
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
	const char* m_pSourceCode = nullptr;
	union
	{
		size_t m_ByteCodeSize = 0;
		size_t m_SourceCodeSize;
	};
	IBvShaderCompiler* pShaderCompiler = nullptr;
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
	// The buffer slot this attribute belongs to
	u32			m_Binding = 0;
	// Index of the input attribute when using the same semantic. Note: In Vulkan and OpenGL, this maps to the location index
	// that an input variable is. In D3D this is only used if there are multiple elements with the same semantic.
	u32			m_Index = 0;
	// The offset of this variable in the struct; specify kAutoOffset to let the pipeline figure out the offset.
	u32			m_Offset = kAutoOffset;
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
	IndexFormat m_IndexFormatForPrimitiveRestart = IndexFormat::kUnknown;
};


struct RasterizerStateDesc
{
	FillMode m_FillMode = FillMode::kSolid;
	CullMode m_CullMode = CullMode::kNone;
	FrontFace m_FrontFace = FrontFace::kClockwise;
	i32 m_DepthBias = 0;
	f32 m_DepthBiasClamp = 0.0f;
	f32 m_DepthBiasSlope = 0.0f;
	bool m_EnableDepthClip = false;
	bool m_EnableConservativeRasterization = false;
};


struct StencilDesc
{
	StencilOp m_StencilFailOp = StencilOp::kKeep;
	StencilOp m_StencilDepthFailOp = StencilOp::kKeep;
	StencilOp m_StencilPassOp = StencilOp::kKeep;
	CompareOp m_StencilFunc = CompareOp::kNone;
};


struct DepthStencilDesc
{
	bool m_DepthTestEnable = false;
	bool m_DepthWriteEnable = false;
	CompareOp m_DepthOp = CompareOp::kNone;
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


struct GraphicsPipelineStateDesc
{
	u32							m_VertexInputDescCount = 0;
	VertexInputDesc*			m_pVertexInputDescs = nullptr;
	IBvShader*					m_Shaders[kMaxShaderStages]{};
	InputAssemblyStateDesc		m_InputAssemblyStateDesc;
	RasterizerStateDesc			m_RasterizerStateDesc;
	DepthStencilDesc			m_DepthStencilDesc;
	BlendStateDesc				m_BlendStateDesc;
	IBvShaderResourceLayout*	m_pShaderResourceLayout = nullptr;
	IBvRenderPass*				m_pRenderPass = nullptr;
	Format						m_RenderTargetFormats[kMaxRenderTargets]{};
	Format						m_DepthStencilFormat = Format::kUnknown;
	u8							m_PatchControlPoints = 0;
	bool						m_ShadingRateEnabled = false;
	u32							m_SampleCount = 1;
	u32							m_SampleMask = kMax<u32>;
	u32							m_SubpassIndex = 0;
	u32							m_MultiviewCount = 1;
};


struct ComputePipelineStateDesc
{
	IBvShader* m_pShader = nullptr;
	IBvShaderResourceLayout* m_pShaderResourceLayout = nullptr;
};


enum class RayTracingAccelerationStructureType : u8
{
	kUnknown,
	kBottomLevel,
	kTopLevel
};


struct DrawCommandArgs
{
	u32 m_VertexCount = 0;
	u32 m_InstanceCount = 0;
	u32 m_FirstVertex = 0;
	u32 m_FirstInstance = 0;
};


struct DrawIndexedCommandArgs
{
	u32 m_IndexCount = 0;
	u32 m_InstanceCount = 0;
	u32 m_FirstIndex = 0;
	i32 m_VertexOffset = 0;
	u32 m_FirstInstance = 0;
};


struct DispatchCommandArgs
{
	u32 m_ThreadGroupCountX = 0;
	u32 m_ThreadGroupCountY = 0;
	u32 m_ThreadGroupCountZ = 0;
};


struct DispatchMeshCommandArgs
{
	u32 m_ThreadGroupCountX = 0;
	u32 m_ThreadGroupCountY = 0;
	u32 m_ThreadGroupCountZ = 0;
};


enum class RayTracingGeometryType : u8
{
	kUnknown,
	kTriangles,
	kAABB
};


enum class RayTracingGeometryFlags : u8
{
	kNone = 0,
	kOpaque = BvBit(0),
	kNoDuplicateAnyHitInvocation = BvBit(1)
};
BV_USE_ENUM_CLASS_OPERATORS(RayTracingGeometryFlags);


enum class RayTracingInstanceFlags : u8
{
	kNone = 0,
	kTriangleCullDisable = BvBit(0),
	kTriangleFrontCounterclockwise = BvBit(1),
	kForceOpaque = BvBit(2),
	kForceNonOpaque = BvBit(3)
};
BV_USE_ENUM_CLASS_OPERATORS(RayTracingInstanceFlags);


enum class RayTracingAccelerationStructureFlags : u8
{
	kNone = 0,
	kAllowUpdate = BvBit(0),
	kAllowCompaction = BvBit(1),
	kPreferFastTrace = BvBit(2),
	kPreferFastBuild = BvBit(3),
	kLowMemory = BvBit(4)
};
BV_USE_ENUM_CLASS_OPERATORS(RayTracingAccelerationStructureFlags);


enum class ASPostBuildAction : u8
{
	kWriteCompactedSize,
};


struct ASPostBuildDesc
{
	ASPostBuildAction m_Action = ASPostBuildAction::kWriteCompactedSize;
	IBvBuffer* m_pDstBuffer = nullptr;
	u64 m_DstBufferOffset = 0;
};


struct BLASGeometryDesc
{
	struct TriangleDesc
	{
		u32 m_VertexCount;
		u32 m_VertexStride;
		u32 m_IndexCount;
		Format m_VertexFormat;
		IndexFormat m_IndexFormat;
	};

	struct AABBDesc
	{
		u32 m_Stride;
		u32 m_Count;
	};

	BvStringId m_Id;
	union
	{
		TriangleDesc m_Triangle{};
		AABBDesc m_AABB;
	};

	RayTracingGeometryFlags m_Flags = RayTracingGeometryFlags::kNone;
	RayTracingGeometryType m_Type = RayTracingGeometryType::kUnknown;
};


struct BLASDesc
{
	u32 m_GeometryCount = 0;
	const BLASGeometryDesc* m_pGeometries = 0;
};


struct BLASBuildGeometryDesc
{
	struct TriangleDesc
	{
		IBvBuffer* m_pVertexBuffer;
		u64 m_VertexOffset;
		IBvBuffer* m_pIndexBuffer;
		u64 m_IndexOffset;
	};

	struct AABBDesc
	{
		IBvBuffer* m_pBuffer;
		u64 m_Offset;
	};

	BvStringId m_Id;
	union
	{
		TriangleDesc m_Triangle{};
		AABBDesc m_AABB;
	};

	RayTracingGeometryFlags m_Flags = RayTracingGeometryFlags::kNone;
	RayTracingGeometryType m_Type = RayTracingGeometryType::kUnknown;
};


struct BLASBuildDesc
{
	bool m_Update = false;
	u32 m_GeometryCount = 0;
	const BLASBuildGeometryDesc* m_pGeometries = nullptr;
	IBvAccelerationStructure* m_pBLAS = nullptr;
	IBvBuffer* m_pScratchBuffer = nullptr;
	u64 m_ScratchBufferOffset = 0;
};


struct TLASDesc
{
	u32 m_InstanceCount = 0;
	RayTracingGeometryFlags m_Flags = RayTracingGeometryFlags::kNone;
};


struct TLASInstanceDesc
{
	Float34 m_Transform{ Float4(1.0f, 0.0f, 0.0f, 0.0f), Float4(0.0f, 1.0f, 0.0f, 0.0f), Float4(0.0f, 0.0f, 1.0f, 0.0f) };
	u32 m_InstanceId = 0;
	u32 m_InstanceMask = 0;
	u32 m_ShaderBindingTableIndex = 0;
	RayTracingInstanceFlags m_Flags = RayTracingInstanceFlags::kNone;
	IBvAccelerationStructure* m_pBLAS = nullptr;
};


struct TLASBuildDesc
{
	bool m_Update = false;
	u32 m_InstanceCount = 0;
	IBvAccelerationStructure* m_pTLAS = nullptr;
	IBvBuffer* m_pInstanceBuffer = nullptr;
	u64 m_InstanceBufferOffset = 0;
	IBvBuffer* m_pScratchBuffer = nullptr;
	u64 m_ScratchBufferOffset = 0;
};


enum class AccelerationStructureCopyMode : u8
{
	kClone,
	kCompact
};


struct AccelerationStructureCopyDesc
{
	IBvAccelerationStructure* m_pSrc = nullptr;
	IBvAccelerationStructure* m_pDst = nullptr;
	AccelerationStructureCopyMode m_CopyMode = AccelerationStructureCopyMode::kClone;
};


struct RayTracingAccelerationStructureScratchSize
{
	u64 m_Build = 0;
	u64 m_Update = 0;
};


struct RayTracingAccelerationStructureDesc
{
	RayTracingAccelerationStructureType m_Type = RayTracingAccelerationStructureType::kUnknown;
	RayTracingAccelerationStructureFlags m_Flags = RayTracingAccelerationStructureFlags::kNone;
	u64 m_CompactedSize = 0;
	union
	{
		BLASDesc m_BLAS{};
		TLASDesc m_TLAS;
	};
};


enum class ShaderGroupType : u8
{
	kNone,
	kGeneral,
	kTriangles,
	kProcedural
};


struct ShaderGroupDesc
{
	static constexpr u32 kUnusedShader = kU32Max;

	const char* m_pName = nullptr;
	ShaderGroupType m_Type = ShaderGroupType::kNone;
	u32 m_General = kUnusedShader;
	u32 m_ClosestHit = kUnusedShader;
	u32 m_AnyHit = kUnusedShader;
	u32 m_Intersection = kUnusedShader;
};


struct RayTracingPipelineStateDesc
{
	u32 m_ShaderCount = 0;
	u32 m_ShaderGroupCount = 0;
	const IBvShader* const* m_ppShaders = nullptr;
	const ShaderGroupDesc* m_pShaderGroupDescs = nullptr;
	IBvShaderResourceLayout* m_pShaderResourceLayout = nullptr;
	u32 m_MaxPipelineRayRecursionDepth = 0;
	u32 m_MaxPayloadSize = 0;
	u32 m_MaxAttributeSize = 0;
	bool m_ForcePayloadAndAttributeSizes = false; // Vulkan only
};


enum ShaderBindingTableGroupType : u8
{
	kRayGen,
	kMiss,
	kHit,
	kCallable,
	kCount
};


struct ShaderBindingTableDesc
{
	IBvRayTracingPipelineState* m_pPSO = nullptr;
};


struct DeviceAddressRange
{
	u64 m_Address = 0;
	u64 m_Size = 0;
};


struct DeviceAddressRangeAndStride
{
	u64 m_Address = 0;
	u64 m_Size = 0;
	u64 m_Stride = 0;
};


struct DispatchRaysCommandArgs
{
	DeviceAddressRange m_RayGenShader;
	DeviceAddressRangeAndStride m_MissShader;
	DeviceAddressRangeAndStride m_HitShader;
	DeviceAddressRangeAndStride m_CallableShader;
	u32 m_Width = 0;
	u32 m_Height = 0;
	u32 m_Depth = 0;
};


struct CommandContextDesc
{
	constexpr CommandContextDesc() = default;
	constexpr CommandContextDesc(CommandType commandType, bool requireDedicated = true)
		: m_CommandType(commandType), m_RequireDedicated(requireDedicated)	{}
	constexpr CommandContextDesc(u32 contextGroupIndex)
		: m_ContextGroupIndex(contextGroupIndex) {}

	u32 m_ContextGroupIndex = kU32Max;
	CommandType m_CommandType = CommandType::kNone;
	bool m_RequireDedicated = false;
};


struct BvRenderDeviceCreateDesc
{
	struct ContextGroupDesc
	{
		u32 m_GroupIndex;
		u32 m_ContextCount;
	};

	u32 m_GPUIndex = kU32Max;
	BvFixedVector<ContextGroupDesc, kMaxContextGroupCount> m_ContextGroups;
	bool m_UseDebug = true;
};


using GPUList = BvFixedVector<BvGPUInfo*, kMaxDevices>;