#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvStringId.h"
#include "BDeV/Core/Math/BvMath.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Utils/BvObject.h"
#include "BDeV/Core/Utils/BvHash.h"
#include <algorithm>

#define BV_RENDER_VAR(var) \
BV_INLINE std::conditional_t<(std::is_fundamental_v<decltype(m_##var)> || std::is_enum_v<decltype(m_##var)>), decltype(m_##var), const decltype(m_##var)&> Get##var() const { return m_##var; } \
BV_INLINE auto& Set##var(std::conditional_t<(std::is_fundamental_v<decltype(m_##var)> || std::is_enum_v<decltype(m_##var)>), decltype(m_##var), const decltype(m_##var)&> value) { m_##var = value; return *this; }

#define BV_RENDER_VAR_PTR(prefix, var) \
BV_INLINE std::conditional_t<(std::is_fundamental_v<decltype(m_##prefix##var)> || std::is_enum_v<decltype(m_##prefix##var)>), decltype(m_##prefix##var), const decltype(m_##prefix##var)&> Get##var##Ptr() const { return m_##prefix##var; } \
BV_INLINE auto& Set##var##Ptr(std::conditional_t<(std::is_fundamental_v<decltype(m_##prefix##var)> || std::is_enum_v<decltype(m_##prefix##var)>), decltype(m_##prefix##var), const decltype(m_##prefix##var)&> value) { m_##prefix##var = value; return *this; }


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
class IBvPipelineCache;
class BvWindow;


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
	kShadingRatePerDraw				= BvBit(10),
	kShadingRateImage				= BvBit(11),
	kMeshShader						= BvBit(12),
	kMeshQuery						= BvBit(13),
	kRayTracing						= BvBit(14),
	kRayQuery						= BvBit(15),
	kMultiView						= BvBit(16),
	kTrueFullScreen					= BvBit(17),
	kDepthStencilResolve			= BvBit(18)
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

	// Extensions
	kPredication,
	kShadingRate,
	kASBuildRead,
	kASBuildWrite,
	kRayTracing
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
			u32 m_Width;
			u32 m_Height;
			u32 m_Depth;
		};
		struct
		{
			u32 m_X;
			u32 m_Y;
			u32 m_Z;
		};
		u32 m_Sizes[3];
	};

	friend constexpr bool operator==(const Extent3D& lhs, const Extent3D& rhs)
	{
		return lhs.m_X == rhs.m_X && lhs.m_Y == rhs.m_Y && lhs.m_Z == rhs.m_Z;
	}

	friend constexpr bool operator!=(const Extent3D& lhs, const Extent3D& rhs)
	{
		return !(lhs == rhs);
	}
};


struct Offset3D
{
	i32 m_X = 0;
	i32 m_Y = 0;
	i32 m_Z = 0;
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
	SwapChainMode m_WindowMode = SwapChainMode::kWindowed;
	bool m_PreferHDR = false;

	BV_RENDER_VAR(SwapChainImageCount);
	BV_RENDER_VAR(Format);
	BV_RENDER_VAR(VSync);
	BV_RENDER_VAR(WindowMode);
	BV_RENDER_VAR(PreferHDR);
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
	BufferUsage m_UsageFlags = BufferUsage::kNone;
	BufferCreateFlags m_CreateFlags = BufferCreateFlags::kNone;
	MemoryType m_MemoryType = MemoryType::kDevice;
	bool m_Formatted = false;

	BV_RENDER_VAR(Size);
	BV_RENDER_VAR(UsageFlags);
	BV_RENDER_VAR(CreateFlags);
	BV_RENDER_VAR(MemoryType);
	BV_RENDER_VAR(Formatted);
};


struct BufferInitData
{
	BufferInitData() = default;
	BufferInitData(IBvCommandContext* pContext, const void* pData = nullptr, u64 size = 0)
		: m_pContext(pContext), m_pData(pData), m_Size(size) {}

	IBvCommandContext* m_pContext = nullptr;
	const void* m_pData = nullptr;
	u64 m_Size = 0;
	ResourceState m_ResourceState = ResourceState::kCommon;
};


struct BufferViewDesc
{
	IBvBuffer* m_pBuffer = nullptr;
	u64 m_Offset = 0;
	u32 m_ElementCount = 0;
	u32 m_Stride = 0;
	Format m_Format = Format::kUnknown;

	BV_RENDER_VAR_PTR(p, Buffer);
	BV_RENDER_VAR(Offset);
	BV_RENDER_VAR(ElementCount);
	BV_RENDER_VAR(Stride);
	BV_RENDER_VAR(Format);
};


struct VertexBufferView
{
	VertexBufferView(IBvBuffer* pBuffer, u32 stride, u64 offset = 0)
		: m_pBuffer(pBuffer), m_Stride(stride), m_Offset(offset) {}

	IBvBuffer* m_pBuffer = nullptr;
	u64 m_Offset = 0;
	u32 m_Stride = 0;
};


struct IndexBufferView
{
	IndexBufferView(IBvBuffer* pBuffer = nullptr, IndexFormat indexFormat = IndexFormat::kU32, u64 offset = 0)
		: m_pBuffer(pBuffer), m_IndexFormat(indexFormat), m_Offset(offset) {}

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

	BV_RENDER_VAR(Size);
	BV_RENDER_VAR(Alignment);
	BV_RENDER_VAR(MipLevels);
	BV_RENDER_VAR(ArraySize);
	BV_RENDER_VAR(SampleCount);
	BV_RENDER_VAR(ImageType);
	BV_RENDER_VAR(Format);
	BV_RENDER_VAR(CreateFlags);
	BV_RENDER_VAR(UsageFlags);
	BV_RENDER_VAR(MemoryType);
};


struct SubresourceData
{
	const u8* m_pData;
	u64 m_RowPitch;
	u64 m_SlicePitch;
};


struct TextureInitData
{
	TextureInitData() = default;

	TextureInitData(IBvCommandContext* pContext, u32 subresourceCount = 0, const SubresourceData* pSubresources = nullptr)
		: m_pContext(pContext), m_SubresourceCount(subresourceCount), m_pSubresources(pSubresources) {}

	template<typename Container>
	TextureInitData(IBvCommandContext* pContext, const Container& subresources)
		: m_pContext(pContext), m_SubresourceCount(subresources.Size()), m_pSubresources(subresources.Data()) {}

	IBvCommandContext* m_pContext = nullptr;
	const SubresourceData* m_pSubresources = nullptr;
	u32 m_SubresourceCount = 0;
	u32 m_FirstSubresource = 0;
	ResourceState m_ResourceState = ResourceState::kCommon;
	u32 m_MutableFormatCount = 0;
	const Format* m_pMutableFormats = nullptr;
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
	Format m_Format;
	u32 m_Width;
	u32 m_Height;
	u32 m_Detph;
	u32 m_NumRows;
	u32 m_RowPitch;
	u32 m_RowSize;
	u32 m_SlicePitch;
};


struct SubresourceFootprint
{
	u64 m_Offset;
	TextureSubresourceInfo m_Subresource;
};


struct TextureViewDesc
{
	IBvTexture* m_pTexture = nullptr;
	SubresourceDesc m_SubresourceDesc;
	TextureViewType m_ViewType = TextureViewType::kTexture2D;
	Format m_Format = Format::kUnknown;

	BV_RENDER_VAR_PTR(p, Texture);
	BV_RENDER_VAR(SubresourceDesc);
	BV_RENDER_VAR(ViewType);
	BV_RENDER_VAR(Format);
};


struct ClearColorValue
{
	constexpr ClearColorValue()
		: r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	constexpr ClearColorValue(f32 r, f32 g, f32 b, f32 a = 1.0f)
		: r(r), g(g), b(b), a(a) {}
	constexpr ClearColorValue(const f32* pColors)
	{
		std::copy(pColors, pColors + 4, colors);
	}
	constexpr ClearColorValue(f32 depth, u8 stencil = 0)
		: depth(depth), stencil(stencil) {}
	union
	{
		struct
		{
			f32 r;
			f32 g;
			f32 b;
			f32 a;
		};
		struct
		{
			f32 colors[4];
		};
		struct
		{
			f32 depth;
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
	Viewport(f32 width, f32 height)
		: m_Width(width), m_Height(height) {}

	Viewport(f32 width, f32 height, f32 minDepth, f32 maxDepth)
		: m_Width(width), m_Height(height), m_MinDepth(minDepth), m_MaxDepth(maxDepth) {}

	Viewport(f32 x, f32 y, f32 width, f32 height, f32 minDepth, f32 maxDepth)
		: m_X(x), m_Y(y), m_Width(width), m_Height(height), m_MinDepth(minDepth), m_MaxDepth(maxDepth) {}

	f32 m_X = 0.0f;
	f32 m_Y = 0.0f;
	f32 m_Width = 0.0f;
	f32 m_Height = 0.0f;
	f32 m_MinDepth = 0.0f;
	f32 m_MaxDepth = 1.0f;
};


struct Rect
{
	Rect() = default;
	Rect(u32 width, u32 height) : m_Right(width), m_Bottom(height) {}
	Rect(i32 left, i32 top, i32 right, i32 bottom) : m_Left(left), m_Top(top), m_Right(right), m_Bottom(bottom) {}

	i32 m_Left = 0;
	i32 m_Top = 0;
	i32 m_Right = 0;
	i32 m_Bottom = 0;
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
	u32 m_SrcPlane = 0;
	Offset3D m_DstTextureOffset{ 0,0,0 };
	u32 m_DstMip = 0;
	u32 m_DstLayer = 0;
	u32 m_DstPlane = 0;
	Extent3D m_Size{ kU32Max, kU32Max, kU32Max };
};


struct BufferTextureCopyDesc
{
	SubresourceFootprint m_SubresourceFootprint{};
	Offset3D m_TextureOffset{ 0,0,0 };
	Extent3D m_TextureSize{ kU32Max, kU32Max, kU32Max };
	u32 m_Mip = 0;
	u32 m_Layer = 0;
	u32 m_Plane = 0;
};


struct ResourceBarrierDesc
{
	enum class Type : u8
	{
		kStateTransition,
		kMemory,
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

	BV_RENDER_VAR_PTR(p, Texture);
	BV_RENDER_VAR_PTR(p, Buffer);
	BV_RENDER_VAR(SrcState);
	BV_RENDER_VAR(DstState);
	BV_RENDER_VAR(Type);
	BV_RENDER_VAR_PTR(p, SrcContext);
	BV_RENDER_VAR_PTR(p, DstContext);
	BV_RENDER_VAR(SrcAccess);
	BV_RENDER_VAR(DstAccess);
	BV_RENDER_VAR(SrcPipelineStage);
	BV_RENDER_VAR(DstPipelineStage);
	BV_RENDER_VAR(Subresource);
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
	ShaderResourceDesc() = default;

	ShaderResourceDesc(ShaderResourceType shaderResourceType, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages)
		: m_ShaderResourceType(shaderResourceType), m_Binding(binding), m_ShaderStages(shaderStages)
	{
	}

	ShaderResourceDesc(ShaderResourceType shaderResourceType, u32 binding,
		u32 count = 1, ShaderStage shaderStages = ShaderStage::kAllStages)
		: m_ShaderResourceType(shaderResourceType), m_Binding(binding), m_Count(count), m_ShaderStages(shaderStages)
	{
	}

	ShaderResourceDesc(ShaderResourceType shaderResourceType, u32 binding,
		u32 count, IBvSampler* const* ppSamplers, ShaderStage shaderStages = ShaderStage::kAllStages)
		: m_ShaderResourceType(shaderResourceType), m_Binding(binding), m_Count(count), m_ShaderStages(shaderStages), m_StaticSamplers(count, ppSamplers)
	{
	}

	ShaderResourceDesc(const BvStringId& name, ShaderResourceType shaderResourceType, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages)
		: m_Name(name), m_ShaderResourceType(shaderResourceType), m_Binding(binding), m_ShaderStages(shaderStages)
	{
	}

	ShaderResourceDesc(const BvStringId& name, ShaderResourceType shaderResourceType, u32 binding,
		u32 count = 1, ShaderStage shaderStages = ShaderStage::kAllStages)
		: m_Name(name), m_ShaderResourceType(shaderResourceType), m_Binding(binding), m_Count(count), m_ShaderStages(shaderStages)
	{
	}

	ShaderResourceDesc(const BvStringId& name, ShaderResourceType shaderResourceType, u32 binding,
		u32 count, IBvSampler* const* ppSamplers, ShaderStage shaderStages = ShaderStage::kAllStages)
		: m_Name(name), m_ShaderResourceType(shaderResourceType), m_Binding(binding), m_Count(count), m_ShaderStages(shaderStages),
		m_StaticSamplers(count, ppSamplers)
	{
	}

	BvStringId m_Name{};
	u32 m_Binding = 0;
	u32 m_Count = 0;
	ShaderResourceType m_ShaderResourceType = ShaderResourceType::kUnknown;
	ShaderStage m_ShaderStages = ShaderStage::kAllStages;
	BvVector<IBvSampler*> m_StaticSamplers;

	BV_RENDER_VAR(Name);
	BV_RENDER_VAR(Binding);
	BV_RENDER_VAR(Count);
	BV_RENDER_VAR(ShaderResourceType);
	BV_RENDER_VAR(ShaderStages);
	
	auto& AddShaderStage(ShaderStage shaderStage)
	{
		m_ShaderStages |= shaderStage;

		return *this;
	}

	auto& AddStaticSampler(IBvSampler* pSampler)
	{
		BV_ASSERT(pSampler, "Invalid Sampler data");
		m_StaticSamplers.PushBack(pSampler);

		return *this;
	}

	auto& AddStaticSamplers(u32 count, IBvSampler* const* ppSamplers)
	{
		BV_ASSERT(ppSamplers, "Invalid Sampler data");
		for (auto i = 0; i < count; ++i)
		{
			m_StaticSamplers.PushBack(ppSamplers[i]);
		}

		return *this;
	}

	auto& SetStaticSamplers(u32 count, IBvSampler* const* ppSamplers)
	{
		BV_ASSERT(ppSamplers, "Invalid Sampler data");
		m_StaticSamplers.Assign(count, ppSamplers);
		
		return *this;
	}

	auto& Set(ShaderResourceType shaderResourceType, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages)
	{
		return SetShaderResourceType(shaderResourceType).SetBinding(binding).SetShaderStages(shaderStages);
	}

	auto& Set(ShaderResourceType shaderResourceType, u32 binding,
		u32 count = 1, ShaderStage shaderStages = ShaderStage::kAllStages)
	{
		return SetShaderResourceType(shaderResourceType).SetBinding(binding).SetShaderStages(shaderStages)
			.SetCount(count);
	}

	auto& Set(ShaderResourceType shaderResourceType, u32 binding,
		u32 count, IBvSampler* const* ppSamplers, ShaderStage shaderStages = ShaderStage::kAllStages)
	{
		return SetShaderResourceType(shaderResourceType).SetBinding(binding).SetShaderStages(shaderStages)
			.SetCount(count).SetStaticSamplers(count, ppSamplers);
	}

	auto& Set(const BvStringId& name, ShaderResourceType shaderResourceType, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages)
	{
		return SetName(name).SetShaderResourceType(shaderResourceType).SetBinding(binding).SetShaderStages(shaderStages);
	}

	auto& Set(const BvStringId& name, ShaderResourceType shaderResourceType, u32 binding,
		u32 count = 1, ShaderStage shaderStages = ShaderStage::kAllStages)
	{
		return SetName(name).SetShaderResourceType(shaderResourceType).SetBinding(binding).SetShaderStages(shaderStages)
			.SetCount(count);
	}

	auto& Set(const BvStringId& name, ShaderResourceType shaderResourceType, u32 binding,
		u32 count, IBvSampler* const* ppSamplers, ShaderStage shaderStages = ShaderStage::kAllStages)
	{
		return SetName(name).SetShaderResourceType(shaderResourceType).SetBinding(binding).SetShaderStages(shaderStages)
			.SetCount(count).SetStaticSamplers(count, ppSamplers);
	}

	bool IsDynamic() const
	{
		return m_ShaderResourceType == ShaderResourceType::kDynamicConstantBuffer
			|| m_ShaderResourceType == ShaderResourceType::kDynamicStructuredBuffer
			|| m_ShaderResourceType == ShaderResourceType::kDynamicRWStructuredBuffer;
	}

	static ShaderResourceDesc AsConstantBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kConstantBuffer, binding, count, shaderStages); }
	static ShaderResourceDesc AsStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kStructuredBuffer, binding, count, shaderStages); }
	static ShaderResourceDesc AsRWStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kRWStructuredBuffer, binding, count, shaderStages); }
	static ShaderResourceDesc AsDynamicConstantBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return ShaderResourceDesc(name, ShaderResourceType::kDynamicConstantBuffer, binding, 1, shaderStages); }
	static ShaderResourceDesc AsDynamicStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return ShaderResourceDesc(name, ShaderResourceType::kDynamicStructuredBuffer, binding, 1, shaderStages); }
	static ShaderResourceDesc AsDynamicRWStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return ShaderResourceDesc(name, ShaderResourceType::kDynamicRWStructuredBuffer, binding, 1, shaderStages); }
	static ShaderResourceDesc AsFormattedBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kFormattedBuffer, binding, count, shaderStages); }
	static ShaderResourceDesc AsRWFormattedBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kRWFormattedBuffer, binding, count, shaderStages); }
	static ShaderResourceDesc AsTexture(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kTexture, binding, count, shaderStages); }
	static ShaderResourceDesc AsRWTexture(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kRWTexture, binding, count, shaderStages); }
	static ShaderResourceDesc AsSampler(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kSampler, binding, count, shaderStages); }
	static ShaderResourceDesc AsInputAttachment(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kInputAttachment, binding, count, shaderStages); }
	static ShaderResourceDesc AsAccelerationStructure(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc(name, ShaderResourceType::kAccelerationStructure, binding, count, shaderStages); }
	static ShaderResourceDesc AsStaticSampler(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, IBvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc(name, ShaderResourceType::kSampler, binding, count, ppSamplers, shaderStages); }

	static ShaderResourceDesc AsConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsDynamicConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return AsDynamicConstantBuffer(BvStringId::Empty(), binding, shaderStages); }
	static ShaderResourceDesc AsDynamicStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return AsDynamicStructuredBuffer(BvStringId::Empty(), binding, shaderStages); }
	static ShaderResourceDesc AsDynamicRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return AsDynamicRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages); }
	static ShaderResourceDesc AsFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsInputAttachment(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsInputAttachment(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsAccelerationStructure(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsAccelerationStructure(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, IBvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(BvStringId::Empty(), binding, shaderStages, count, ppSamplers); }

	friend bool operator<(const ShaderResourceDesc& lhs, const ShaderResourceDesc& rhs)
	{
		return lhs.m_Binding < rhs.m_Binding;
	}

	friend bool operator==(const ShaderResourceDesc& lhs, const ShaderResourceDesc& rhs)
	{
		return lhs.m_Name == rhs.m_Name && lhs.m_Binding == rhs.m_Binding && lhs.m_Count == rhs.m_Count && lhs.m_ShaderResourceType == rhs.m_ShaderResourceType
			&& lhs.m_ShaderStages == rhs.m_ShaderStages && lhs.m_StaticSamplers == rhs.m_StaticSamplers;
	}
};


struct ShaderResourceConstantDesc
{
	ShaderResourceConstantDesc() = default;
	ShaderResourceConstantDesc(const BvStringId& name, u32 binding, u32 size, ShaderStage shaderStages)
		: m_Name(name), m_Binding(binding), m_Size(size), m_ShaderStages(shaderStages) {}
	ShaderResourceConstantDesc(const BvStringId& name, u32 size, ShaderStage shaderStages)
		: m_Name(name), m_Size(size), m_ShaderStages(shaderStages) {}
	ShaderResourceConstantDesc(u32 size, ShaderStage shaderStages)
		: m_Size(size), m_ShaderStages(shaderStages) {}

	BvStringId m_Name{};
	u32 m_Binding = 0;
	u32 m_Size = 0;
	ShaderStage m_ShaderStages = ShaderStage::kAllStages;

	BV_RENDER_VAR(Name);
	BV_RENDER_VAR(Binding);
	BV_RENDER_VAR(Size);
	BV_RENDER_VAR(ShaderStages);

	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, binding, sizeof(T), shaderStages }; }
	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId& name, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, 0, sizeof(T), shaderStages }; }
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
	enum class Type : u8
	{
		kDefault,
		kBindless,
		kDynamic,
	};

	u32 m_Index;
	Type m_Type;
	BvVector<ShaderResourceDesc> m_Resources;
	BvVector<ShaderResourceConstantDesc> m_Constants;

	auto& AddConstantBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kConstantBuffer, binding, count, shaderStages); return *this; }
	auto& AddStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kStructuredBuffer, binding, count, shaderStages); return *this; }
	auto& AddRWStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kRWStructuredBuffer, binding, count, shaderStages); return *this; }
	auto& AddFormattedBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kFormattedBuffer, binding, count, shaderStages); return *this; }
	auto& AddRWFormattedBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kRWFormattedBuffer, binding, count, shaderStages); return *this; }
	auto& AddTexture(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kTexture, binding, count, shaderStages); return *this; }
	auto& AddRWTexture(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kRWTexture, binding, count, shaderStages); return *this; }
	auto& AddSampler(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kSampler, binding, count, shaderStages); return *this; }
	auto& AddInputAttachment(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kInputAttachment, binding, count, shaderStages); return *this; }
	auto& AddAccelerationStructure(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { m_Resources.EmplaceBack(name, ShaderResourceType::kAccelerationStructure, binding, count, shaderStages); return *this; }
	auto& AddStaticSampler(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, IBvSampler* const* ppSamplers = nullptr) { m_Resources.EmplaceBack(name, ShaderResourceType::kSampler, binding, count, ppSamplers, shaderStages); return *this; }

	auto& AddConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddConstantBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddRWFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddRWFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddTexture(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddRWTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddRWTexture(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddSampler(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddInputAttachment(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddInputAttachment(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddAccelerationStructure(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AddAccelerationStructure(BvStringId::Empty(), binding, shaderStages, count); }
	auto& AddStaticSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, IBvSampler* const* ppSamplers = nullptr) { return AddStaticSampler(BvStringId::Empty(), binding, shaderStages, count, ppSamplers); }

	auto& AddDynamicConstantBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Resources.EmplaceBack(name, ShaderResourceType::kDynamicConstantBuffer, binding, 1, shaderStages); return *this; }
	auto& AddDynamicStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Resources.EmplaceBack(name, ShaderResourceType::kDynamicStructuredBuffer, binding, 1, shaderStages); return *this; }
	auto& AddDynamicRWStructuredBuffer(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Resources.EmplaceBack(name, ShaderResourceType::kDynamicRWStructuredBuffer, binding, 1, shaderStages); return *this; }

	auto& AddDynamicConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return AddDynamicConstantBuffer(BvStringId::Empty(), binding, shaderStages); }
	auto& AddDynamicStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return AddDynamicStructuredBuffer(BvStringId::Empty(), binding, shaderStages); }
	auto& AddDynamicRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return AddDynamicRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages); }

	auto& AddConstant(const BvStringId& name, u32 binding, u32 size, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Constants.EmplaceBack(name, binding, size, shaderStages); return *this; }
	auto& AddConstant(const BvStringId& name, u32 size, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Constants.EmplaceBack(name, 0, size, shaderStages); return *this; }
	auto& AddConstant(u32 size, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Constants.EmplaceBack(nullptr, 0, size, shaderStages); return *this; }

	template<typename T> auto& AddConstant(const BvStringId& name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Constants.EmplaceBack(name, binding, sizeof(T), shaderStages); return *this; }
	template<typename T> auto& AddConstant(const BvStringId& name, ShaderStage shaderStages = ShaderStage::kAllStages) { m_Constants.EmplaceBack(name, 0, sizeof(T), shaderStages); return *this; }
	template<typename T> auto& AddConstant(ShaderStage shaderStages = ShaderStage::kAllStages) { m_Constants.EmplaceBack(nullptr, 0, sizeof(T), shaderStages); return *this; }

	bool IsBindless() const { return m_Type == Type::kBindless; }
	bool IsDynamic() const { return m_Type == Type::kDynamic; }

	BV_RENDER_VAR(Index);
	BV_RENDER_VAR(Type);
	BV_RENDER_VAR(Resources);
	BV_RENDER_VAR(Constants);

	friend bool operator<(const ShaderResourceSetDesc& lhs, const ShaderResourceSetDesc& rhs)
	{
		return lhs.m_Index < rhs.m_Index;
	}
};


struct ShaderResourceLayoutCreateDesc
{
	BvVector<ShaderResourceSetDesc> m_ShaderResourceSets;

	BV_INLINE auto& AddResourceSet(u32 index = kU32Max)
	{
		return m_ShaderResourceSets.PushBack(ShaderResourceSetDesc{ index == kU32Max ? (u32)m_ShaderResourceSets.Size() : index, ShaderResourceSetDesc::Type::kDefault });
	}

	BV_INLINE auto& AddBindlessResourceSet(u32 index = kU32Max)
	{
		return m_ShaderResourceSets.PushBack(ShaderResourceSetDesc{ index == kU32Max ? (u32)m_ShaderResourceSets.Size() : index, ShaderResourceSetDesc::Type::kBindless });
	}

	BV_INLINE auto& AddDynamicResourceSet(u32 index = kU32Max)
	{
		return m_ShaderResourceSets.PushBack(ShaderResourceSetDesc{ index == kU32Max ? (u32)m_ShaderResourceSets.Size() : index, ShaderResourceSetDesc::Type::kDynamic });
	}
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
	SamplerDesc() = default;

	SamplerDesc(Filter magFilter, Filter minFilter, MipMapFilter mipmapMode,
		AddressMode addressModeU = AddressMode::kWrap, AddressMode addressModeV = AddressMode::kWrap, AddressMode addressModeW = AddressMode::kWrap)
		: m_MagFilter(magFilter), m_MinFilter(minFilter), m_MipmapMode(mipmapMode),
		m_AddressModeU(addressModeU), m_AddressModeV(addressModeV), m_AddressModeW(addressModeW) {}

	Filter m_MagFilter = Filter::kLinear;
	Filter m_MinFilter = Filter::kLinear;
	MipMapFilter m_MipmapMode = MipMapFilter::kLinear;
	AddressMode m_AddressModeU = AddressMode::kWrap;
	AddressMode m_AddressModeV = AddressMode::kWrap;
	AddressMode m_AddressModeW = AddressMode::kWrap;
	CompareOp m_CompareOp = CompareOp::kNone;
	ReductionMode m_ReductionMode = ReductionMode::kStandard;
	bool m_AnisotropyEnable = false;
	f32 m_MaxAnisotropy = 1.0f;
	f32 m_MipLodBias = 0.0f;
	f32 m_MinLod = 0.0f;
	f32 m_MaxLod = kF32Max;
	f32 m_BorderColor[4]{};

	BV_RENDER_VAR(MagFilter);
	BV_RENDER_VAR(MinFilter);
	BV_RENDER_VAR(MipmapMode);
	BV_RENDER_VAR(AddressModeU);
	BV_RENDER_VAR(AddressModeV);
	BV_RENDER_VAR(AddressModeW);
	BV_RENDER_VAR(CompareOp);
	BV_RENDER_VAR(ReductionMode);
	BV_RENDER_VAR(AnisotropyEnable);
	BV_RENDER_VAR(MaxAnisotropy);
	BV_RENDER_VAR(MipLodBias);
	BV_RENDER_VAR(MinLod);
	BV_RENDER_VAR(MaxLod);

	auto& SetBorderColor(f32 r, f32 g, f32 b, f32 a = 1.0f)
	{
		m_BorderColor[0] = r;
		m_BorderColor[1] = g;
		m_BorderColor[2] = b;
		m_BorderColor[3] = a;

		return *this;
	}

	auto& SetBorderColor(const f32(&color)[4])
	{
		memcpy(m_BorderColor, color, sizeof(m_BorderColor));

		return *this;
	}

	auto& SetFilter(Filter magFilter, Filter minFilter, MipMapFilter mipmapMode)
	{
		return SetMagFilter(magFilter).SetMinFilter(minFilter).SetMipmapMode(mipmapMode);
	}

	auto& SetAddressMode(AddressMode addressModeU, AddressMode addressModeV, AddressMode addressModeW)
	{
		return SetAddressModeU(addressModeU).SetAddressModeV(addressModeV).SetAddressModeW(addressModeW);
	}

	auto& SetAnisotropy(bool enable, f32 maxAnisotropy)
	{
		return SetAnisotropyEnable(enable).SetMaxAnisotropy(maxAnisotropy);
	}

	auto& SetLOD(f32 min, f32 max, f32 bias = 0.0f)
	{
		return SetMinLod(min).SetMaxLod(max).SetMipLodBias(bias);
	}
};


struct GPUFenceDesc
{
	u64 m_Value;

	BV_RENDER_VAR(Value);
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
	enum class Type : u8
	{
		kNone,
		kColor,
		kDepthStencil,
		kReadOnlyDepthStencil,
		kShadingRate
	};

	IBvTextureView* m_pView = nullptr;
	IBvTextureView* m_pResolveView = nullptr;
	ClearColorValue m_ClearValues = ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	Type m_Type = Type::kNone;
	LoadOp m_LoadOp = LoadOp::kClear;
	StoreOp m_StoreOp = StoreOp::kStore;
	ResourceState m_StateBefore = ResourceState::kCommon;
	ResourceState m_StateAfter = ResourceState::kCommon;
	ResourceState m_ResolveStateBefore = ResourceState::kCommon;
	ResourceState m_ResolveStateAfter = ResourceState::kCommon;
	ResolveMode m_ResolveMode = ResolveMode::kNone;

	auto& SetView(IBvTextureView* pView, Type type, ResourceState stateBefore = ResourceState::kCommon,
		ResourceState stateAfter = ResourceState::kCommon)
	{
		m_pView = pView;
		m_Type = type;
		m_StateBefore = stateBefore;
		m_StateAfter = stateAfter;

		return *this;
	}

	auto& SetColorView(IBvTextureView* pView, ResourceState stateBefore = ResourceState::kCommon,
		ResourceState stateAfter = ResourceState::kPixelShaderResource)
	{
		m_pView = pView;
		m_Type = Type::kColor;
		m_StateBefore = stateBefore;
		m_StateAfter = stateAfter;

		return *this;
	}

	auto& SetDepthStencilView(IBvTextureView* pView, ResourceState stateBefore = ResourceState::kCommon,
		ResourceState stateAfter = ResourceState::kPixelShaderResource)
	{
		m_pView = pView;
		m_Type = Type::kDepthStencil;
		m_StateBefore = stateBefore;
		m_StateAfter = stateAfter;

		return *this;
	}

	auto& SetReadOnlyDepthStencilView(IBvTextureView* pView, ResourceState stateBefore = ResourceState::kCommon,
		ResourceState stateAfter = ResourceState::kDepthStencilRead)
	{
		m_pView = pView;
		m_Type = Type::kReadOnlyDepthStencil;
		m_StateBefore = stateBefore;
		m_StateAfter = stateAfter;

		return *this;
	}

	auto& SetShadingRateView(IBvTextureView* pView, ResourceState stateBefore = ResourceState::kCommon,
		ResourceState stateAfter = ResourceState::kShadingRate)
	{
		m_pView = pView;
		m_Type = Type::kShadingRate;
		m_StateBefore = stateBefore;
		m_StateAfter = stateAfter;

		return *this;
	}

	auto& SetResolveView(IBvTextureView* pView, ResourceState stateBefore = ResourceState::kCommon,
		ResourceState stateAfter = ResourceState::kCommon, ResolveMode resolveMode = ResolveMode::kNone)
	{
		m_pResolveView = pView;
		m_ResolveStateBefore = stateBefore;
		m_ResolveStateAfter = stateAfter;
		m_ResolveMode = resolveMode;

		return *this;
	}

	auto& SetLoadStoreOps(LoadOp loadOp = LoadOp::kClear, StoreOp storeOp = StoreOp::kStore)
	{
		m_LoadOp = loadOp;
		m_StoreOp = storeOp;

		return *this;
	}

	bool IsValid() const
	{
		return m_pView != nullptr;
	}

	bool IsColor() const { return m_Type == Type::kColor; }
	bool IsDepthStencil() const { return m_Type == Type::kDepthStencil; }
	bool IsReadOnlyDepthStencil() const { return m_Type == Type::kReadOnlyDepthStencil; }
	bool IsShadingRate() const { return m_Type == Type::kShadingRate; }

	BV_RENDER_VAR(ClearValues);
	BV_RENDER_VAR(Type);
	BV_RENDER_VAR(LoadOp);
	BV_RENDER_VAR(StoreOp);
	BV_RENDER_VAR(StateBefore);
	BV_RENDER_VAR(StateAfter);
	BV_RENDER_VAR(ResolveStateBefore);
	BV_RENDER_VAR(ResolveStateAfter);
	BV_RENDER_VAR(ResolveMode);
};


struct RenderPassTargetDesc
{
	RenderPassTargetDesc() = default;

	RenderPassTargetDesc(IBvTextureView* pView)
		: m_pView(pView) {}

	RenderPassTargetDesc(IBvTextureView* pView, f32 r, f32 g, f32 b, f32 a = 1.0f)
		: m_pView(pView), m_ClearValues(r, g, b, a) {}

	RenderPassTargetDesc(IBvTextureView* pView, const f32* pColors)
		: m_pView(pView), m_ClearValues(pColors) {}

	RenderPassTargetDesc(IBvTextureView* pView, f32 depth, u8 stencil)
		: m_pView(pView), m_ClearValues(depth, stencil) {}

	IBvTextureView* m_pView = nullptr;
	ClearColorValue m_ClearValues = ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
};


struct AttachmentRef
{
	AttachmentRef() = default;
	AttachmentRef(u32 index, ResourceState resourceState, ResolveMode resolveMode = ResolveMode::kNone)
		: m_Index(index), m_ResourceState(resourceState), m_ResolveMode(resolveMode)
	{}

	u32 m_Index = kU32Max;
	ResourceState m_ResourceState = ResourceState::kCommon;
	ResolveMode m_ResolveMode = ResolveMode::kNone;

	bool IsValid() const
	{
		return m_Index != kU32Max;
	}

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
	ShadingRateAttachmentRef() = default;
	ShadingRateAttachmentRef(u32 index, u32 xSize, u32 ySize, ResourceState resourceState = ResourceState::kShadingRate)
		: m_Index(index), m_ResourceState(resourceState), m_TexelSizes{ xSize, ySize } {}

	u32 m_Index = kU32Max;
	ResourceState m_ResourceState = ResourceState::kShadingRate;
	u32 m_TexelSizes[2]{ 0,0 };

	bool IsValid() const
	{
		return m_Index != kU32Max;
	}

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
	BvVector<AttachmentRef> m_ColorAttachments;
	BvVector<AttachmentRef> m_InputAttachments;
	BvVector<AttachmentRef> m_ResolveAttachments;
	AttachmentRef m_DepthStencilAttachment;
	AttachmentRef m_DepthStencilResolveAttachment;
	ShadingRateAttachmentRef m_ShadingRateAttachment;
	u32 m_MultiviewCount = 0;
	u32 m_MultiviewCorrelationMask = 0;

	BV_RENDER_VAR(MultiviewCount);
	BV_RENDER_VAR(MultiviewCorrelationMask);

	auto& AddColorAttachment(u32 index, ResourceState resourceState)
	{
		m_ColorAttachments.EmplaceBack(index, resourceState);
		
		return *this;
	}

	auto& AddInputAttachment(u32 index, ResourceState resourceState)
	{
		m_InputAttachments.EmplaceBack(index, resourceState);

		return *this;
	}

	auto& AddResolveAttachment(u32 index, ResourceState resourceState, ResolveMode resolveMode)
	{
		m_ResolveAttachments.EmplaceBack(index, resourceState, resolveMode);

		return *this;
	}

	auto& SetDepthStencilAttachment(u32 index, ResourceState resourceState)
	{
		m_DepthStencilAttachment = AttachmentRef(index, resourceState);

		return *this;
	}

	auto& SetDepthStencilResolveAttachment(u32 index, ResourceState resourceState, ResolveMode resolveMode)
	{
		m_DepthStencilResolveAttachment = AttachmentRef(index, resourceState, resolveMode);

		return *this;
	}

	auto& ShadingRateAttachment(u32 index, u32 xSize, u32 ySize, ResourceState resourceState = ResourceState::kShadingRate)
	{
		m_ShadingRateAttachment = ShadingRateAttachmentRef(index, xSize, ySize, resourceState);

		return *this;
	}

	bool operator==(const SubpassDesc& rhs) const
	{
		if (m_ColorAttachments != rhs.m_ColorAttachments || m_InputAttachments != rhs.m_InputAttachments
			|| m_ResolveAttachments != rhs.m_ResolveAttachments || m_DepthStencilAttachment != rhs.m_DepthStencilAttachment
			|| m_DepthStencilResolveAttachment != rhs.m_DepthStencilResolveAttachment || m_ShadingRateAttachment != rhs.m_ShadingRateAttachment
			|| m_MultiviewCount != rhs.m_MultiviewCount || m_MultiviewCorrelationMask != rhs.m_MultiviewCorrelationMask)
		{
			return false;
		}

		return true;
	}

	bool operator!=(const SubpassDesc& rhs) const
	{
		return !(*this == rhs);
	}
};


struct SubpassDependency
{
	static constexpr auto kExternalSubpassIndex = kU32Max;

	ResourceAccess m_SrcAccess = ResourceAccess::kAuto;
	ResourceAccess m_DstAccess = ResourceAccess::kAuto;
	PipelineStage m_SrcStage = PipelineStage::kAuto;
	PipelineStage m_DstStage = PipelineStage::kAuto;
	u16 m_SrcSubpass = 0;
	u16 m_DstSubpass = 0;

	BV_RENDER_VAR(SrcAccess);
	BV_RENDER_VAR(DstAccess);
	BV_RENDER_VAR(SrcStage);
	BV_RENDER_VAR(DstStage);
	BV_RENDER_VAR(SrcSubpass);
	BV_RENDER_VAR(DstSubpass);

	auto& SetSrc(u16 subpass, ResourceAccess access, PipelineStage stage)
	{
		return SetSrcSubpass(subpass).SetSrcAccess(access).SetSrcStage(stage);
	}

	auto& SetDst(u16 subpass, ResourceAccess access, PipelineStage stage)
	{
		return SetDstSubpass(subpass).SetDstAccess(access).SetDstStage(stage);
	}

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

	RenderPassAttachment(Format format, ResourceState stateBefore, ResourceState stateAfter, u8 sampleCount, LoadOp loadOp, StoreOp storeOp)
		: m_Format(format), m_StateBefore(stateBefore), m_StateAfter(stateAfter), m_SampleCount(sampleCount), m_LoadOp(loadOp), m_StoreOp(storeOp)
	{}

	RenderPassAttachment(Format format, ResourceState stateAfter)
		: m_Format(format), m_StateBefore(ResourceState::kCommon), m_StateAfter(stateAfter), m_SampleCount(1),
		m_LoadOp(LoadOp::kClear), m_StoreOp(StoreOp::kStore)
	{}

	Format m_Format;
	ResourceState m_StateBefore;
	ResourceState m_StateAfter;
	u8 m_SampleCount : 5;
	LoadOp m_LoadOp : 2;
	StoreOp m_StoreOp : 1;

	BV_RENDER_VAR(Format);
	BV_RENDER_VAR(StateBefore);
	BV_RENDER_VAR(StateAfter);
	BV_RENDER_VAR(SampleCount);
	BV_RENDER_VAR(LoadOp);
	BV_RENDER_VAR(StoreOp);

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
	BvVector<RenderPassAttachment> m_Attachments;
	BvVector<SubpassDesc> m_Subpasses;
	BvVector<SubpassDependency> m_SubpassDependencies;

	auto& AddAttachment()
	{
		return m_Attachments.EmplaceBack();
	}

	auto& AddAttachment(Format format, ResourceState stateBefore, ResourceState stateAfter, u8 sampleCount, LoadOp loadOp, StoreOp storeOp)
	{
		return m_Attachments.EmplaceBack(format, stateBefore, stateAfter, sampleCount, loadOp, storeOp);
	}

	auto& AddAttachment(Format format, ResourceState stateAfter)
	{
		return m_Attachments.EmplaceBack(format, stateAfter);
	}

	auto& AddSubpass()
	{
		return m_Subpasses.EmplaceBack();
	}

	auto& AddSubpassDependency()
	{
		return m_SubpassDependencies.EmplaceBack();
	}

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
		HashCombine(hash, renderPassDesc.m_Attachments.Size());
		for (auto i = 0u; i < renderPassDesc.m_Attachments.Size(); ++i)
		{
			auto& attachment = renderPassDesc.m_Attachments[i];
			HashCombine(hash, attachment.m_Format, attachment.m_StateBefore, attachment.m_StateAfter, attachment.m_SampleCount,
				attachment.m_LoadOp, attachment.m_StoreOp);
		}

		HashCombine(hash, renderPassDesc.m_Subpasses.Size());
		for (auto i = 0u; i < renderPassDesc.m_Subpasses.Size(); ++i)
		{
			auto& subpass = renderPassDesc.m_Subpasses[i];
			HashCombine(hash, subpass.m_ColorAttachments.Size());
			for (auto j = 0u; j < subpass.m_ColorAttachments.Size(); ++j)
			{
				auto& attachment = subpass.m_ColorAttachments[j];
				HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
			}

			if (subpass.m_DepthStencilAttachment.IsValid())
			{
				HashCombine(hash, subpass.m_DepthStencilAttachment.m_Index, subpass.m_DepthStencilAttachment.m_ResourceState);
			}

			HashCombine(hash, subpass.m_InputAttachments.Size());
			for (auto j = 0u; j < subpass.m_InputAttachments.Size(); ++j)
			{
				auto& attachment = subpass.m_InputAttachments[j];
				HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
			}

			if (subpass.m_ResolveAttachments.Size())
			{
				for (auto j = 0u; j < subpass.m_ResolveAttachments.Size(); ++j)
				{
					auto& attachment = subpass.m_ResolveAttachments[j];
					HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
				}
			}

			if (subpass.m_ShadingRateAttachment.IsValid())
			{
				HashCombine(hash, subpass.m_ShadingRateAttachment.m_Index, subpass.m_ShadingRateAttachment.m_ResourceState,
					subpass.m_ShadingRateAttachment.m_TexelSizes[0], subpass.m_ShadingRateAttachment.m_TexelSizes[1]);
			}
		}

		HashCombine(hash, renderPassDesc.m_SubpassDependencies.Size());
		for (auto i = 0u; i < renderPassDesc.m_SubpassDependencies.Size(); ++i)
		{
			auto& subpassDep = renderPassDesc.m_SubpassDependencies[i];
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
	QueryTypeCount,
};
constexpr u32 kQueryTypeCount = u32(QueryType::QueryTypeCount);


struct QueryHeapDesc
{
	QueryHeapDesc(QueryType type = QueryType::kTimestamp, u32 count = 1)
		: m_Type(type), m_Count(count) {}

	QueryType m_Type;
	u32 m_Count;

	BV_RENDER_VAR(Type);
	BV_RENDER_VAR(Count);
};


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
};


struct MeshPipelineStatistics : PipelineStatistics
{
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
	kHLSL_6_9,
};


struct ShaderSourceDesc
{
	const char* m_pSourceCode = nullptr;
	const char* m_pEntryPoint = "main";
	u32 m_SourceCodeSize = 0;
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
	ShaderLanguage m_ShaderLanguage = ShaderLanguage::kUnknown;
	ShaderTarget m_ShaderTarget = ShaderTarget::kUnknown;

	bool IsTargetSPIRV() const
	{
		return m_ShaderTarget >= ShaderTarget::kSPIRV_1_0 && m_ShaderTarget <= ShaderTarget::kSPIRV_1_6;
	}

	bool IsTargetHLSL() const
	{
		return m_ShaderTarget >= ShaderTarget::kHLSL_5_0 && m_ShaderTarget <= ShaderTarget::kHLSL_6_9;
	}
};


struct ShaderDesc
{
	ShaderDesc() = default;
	ShaderDesc(const u8* pByteCode,	size_t byteCodeSize, ShaderStage shaderStage, const char* pEntryPoint = "main")
		: m_pByteCode(pByteCode), m_ByteCodeSize(byteCodeSize), m_ShaderStage(shaderStage), m_pEntryPoint(pEntryPoint) {}

	const u8* m_pByteCode = nullptr;
	size_t m_ByteCodeSize = 0;
	const char* m_pEntryPoint = "main";
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
};


struct VertexInputDesc
{
	static constexpr u32 kAutoOffset = kU32Max;
	
	constexpr VertexInputDesc() = default;
	constexpr VertexInputDesc(const char* pName, u32 binding, Format format, u32 index = 0, InputRate inputRate = InputRate::kPerVertex,
		u32 offset = kAutoOffset, u32 instanceRate = 0)
		: m_pName(pName), m_Binding(binding), m_Index(index), m_Format(format), m_InputRate(inputRate),
		m_Offset(offset), m_InstanceRate(instanceRate) {}

	// Element name
	const char* m_pName = nullptr;
	// The buffer slot this attribute belongs to
	u32 m_Binding = 0;
	// Index of the input attribute when using the same semantic. Note: In Vulkan and OpenGL, this maps to the location index
	// that an input variable is. In D3D this is only used if there are multiple elements with the same semantic.
	u32 m_Index = 0;
	// The offset of this variable in the struct; specify kAutoOffset to let the pipeline figure out the offset.
	u32 m_Offset = kAutoOffset;
	// Variable format
	Format m_Format = Format::kUnknown;
	// The input rate, can be per vertex or per instance
	InputRate m_InputRate = InputRate::kPerVertex;
	// The instance rate / step, which determines how many instances will be drawn with the same per-instance
	// data before moving to the next one - this should be 0 if the input rate is InputRate::kPerVertex
	u32 m_InstanceRate = 0;

	BV_RENDER_VAR_PTR(p, Name);
	BV_RENDER_VAR(Binding);
	BV_RENDER_VAR(Index);
	BV_RENDER_VAR(Offset);
	BV_RENDER_VAR(Format);
	BV_RENDER_VAR(InputRate);
	BV_RENDER_VAR(InstanceRate);

	bool operator==(const VertexInputDesc& rhs) const
	{
		return m_pName == m_pName && m_Binding == rhs.m_Binding && m_Offset == rhs.m_Offset && m_Format == rhs.m_Format
			&& m_InstanceRate == rhs.m_InstanceRate && m_InputRate == rhs.m_InputRate;
	}
};

struct InputAssemblyStateDesc
{
	constexpr InputAssemblyStateDesc(Topology topology = Topology::kTriangleList, bool primitiveRestart = false,
		IndexFormat indexFormatForPrimitiveRestart = IndexFormat::kUnknown)
		: m_Topology(topology), m_PrimitiveRestart(primitiveRestart), m_IndexFormatForPrimitiveRestart(indexFormatForPrimitiveRestart)
	{}

	Topology m_Topology = Topology::kTriangleList;
	bool m_PrimitiveRestart = false;
	IndexFormat m_IndexFormatForPrimitiveRestart = IndexFormat::kUnknown;
	
	BV_RENDER_VAR(Topology);
	BV_RENDER_VAR(PrimitiveRestart);
	BV_RENDER_VAR(IndexFormatForPrimitiveRestart);
};


struct RasterizerStateDesc
{
	constexpr RasterizerStateDesc() = default;
	constexpr RasterizerStateDesc(FillMode fillMode, CullMode cullMode, FrontFace frontFace, bool enableDepthClip = false,
		i32 depthBias = 0, f32 depthBiasClamp = 0.0f, f32 depthBiasSlope = 0.0f, bool enableConservativeRasterization = false)
		: m_FillMode(fillMode), m_CullMode(cullMode), m_FrontFace(frontFace), m_DepthBias(depthBias), m_DepthBiasClamp(depthBiasClamp),
		m_DepthBiasSlope(depthBiasSlope), m_EnableDepthClip(enableDepthClip), m_EnableConservativeRasterization(enableConservativeRasterization) {}

	FillMode m_FillMode = FillMode::kSolid;
	CullMode m_CullMode = CullMode::kNone;
	FrontFace m_FrontFace = FrontFace::kClockwise;
	bool m_EnableDepthClip = false;
	i32 m_DepthBias = 0;
	f32 m_DepthBiasClamp = 0.0f;
	f32 m_DepthBiasSlope = 0.0f;
	bool m_EnableConservativeRasterization = false;

	BV_RENDER_VAR(FillMode);
	BV_RENDER_VAR(CullMode);
	BV_RENDER_VAR(FrontFace);
	BV_RENDER_VAR(EnableDepthClip);
	BV_RENDER_VAR(DepthBias);
	BV_RENDER_VAR(DepthBiasClamp);
	BV_RENDER_VAR(DepthBiasSlope);
	BV_RENDER_VAR(EnableConservativeRasterization);
};


struct StencilDesc
{
	constexpr StencilDesc() = default;
	constexpr StencilDesc(StencilOp stencilFailOp, StencilOp stencilDepthFailOp, StencilOp stencilPassOp, CompareOp stencilFunc)
		: m_StencilFailOp(stencilFailOp), m_StencilDepthFailOp(stencilDepthFailOp), m_StencilPassOp(stencilPassOp), m_StencilFunc(stencilFunc)
	{}

	StencilOp m_StencilFailOp = StencilOp::kKeep;
	StencilOp m_StencilDepthFailOp = StencilOp::kKeep;
	StencilOp m_StencilPassOp = StencilOp::kKeep;
	CompareOp m_StencilFunc = CompareOp::kNone;

	BV_RENDER_VAR(StencilFailOp);
	BV_RENDER_VAR(StencilDepthFailOp);
	BV_RENDER_VAR(StencilPassOp);
	BV_RENDER_VAR(StencilFunc);
};


struct DepthStencilDesc
{
	constexpr DepthStencilDesc() = default;
	constexpr DepthStencilDesc(bool depthTestEnable, bool depthWriteEnable, CompareOp depthOp, bool stencilTestEnable = false,
		u8 stencilReadMask = 0, u8 stencilWriteMask = 0, bool depthBoundsTestEnable = false, const StencilDesc& stencilFront = {}, const StencilDesc& stencilBack = {}) {}
	
	bool m_DepthTestEnable = false;
	bool m_DepthWriteEnable = false;
	CompareOp m_DepthOp = CompareOp::kNone;
	bool m_StencilTestEnable = false;
	u8 m_StencilReadMask = 0;
	u8 m_StencilWriteMask = 0;
	bool m_DepthBoundsTestEnable = false;
	StencilDesc m_StencilFront{};
	StencilDesc m_StencilBack{};

	BV_RENDER_VAR(DepthTestEnable);
	BV_RENDER_VAR(DepthWriteEnable);
	BV_RENDER_VAR(DepthOp);
	BV_RENDER_VAR(StencilTestEnable);
	BV_RENDER_VAR(StencilReadMask);
	BV_RENDER_VAR(StencilWriteMask);
	BV_RENDER_VAR(DepthBoundsTestEnable);
	BV_RENDER_VAR(StencilFront);
	BV_RENDER_VAR(StencilBack);
};


enum class RenderTargetWriteMask : u8
{
	kZero = 0,
	kRed = BvBit(0),
	kGreen = BvBit(1),
	kBlue = BvBit(2),
	kAlpha = BvBit(3),
	kAll = kRed | kGreen | kBlue | kAlpha
};
BV_USE_ENUM_CLASS_OPERATORS(RenderTargetWriteMask);


struct BlendAttachmentStateDesc
{
	constexpr BlendAttachmentStateDesc() = default;
	constexpr BlendAttachmentStateDesc(bool blendEnable, BlendFactor srcBlend, BlendFactor dstBlend, BlendOp blendOp,
		BlendFactor srcBlendAlpha, BlendFactor dstBlendAlpha, BlendOp alphaBlendOp, RenderTargetWriteMask renderTargetWriteMask = RenderTargetWriteMask::kAll)
		: m_BlendEnable(blendEnable), m_SrcBlend(srcBlend), m_DstBlend(dstBlend), m_BlendOp(blendOp), m_SrcBlendAlpha(srcBlendAlpha),
		m_DstBlendAlpha(dstBlendAlpha), m_AlphaBlendOp(alphaBlendOp), m_RenderTargetWriteMask(renderTargetWriteMask)
	{}

	bool m_BlendEnable = false;
	BlendFactor m_SrcBlend = BlendFactor::kZero;
	BlendFactor m_DstBlend = BlendFactor::kZero;
	BlendOp m_BlendOp = BlendOp::kAdd;
	BlendFactor m_SrcBlendAlpha = BlendFactor::kZero;
	BlendFactor m_DstBlendAlpha = BlendFactor::kZero;
	BlendOp m_AlphaBlendOp = BlendOp::kAdd;
	RenderTargetWriteMask m_RenderTargetWriteMask = RenderTargetWriteMask::kAll;

	BV_RENDER_VAR(BlendEnable);
	BV_RENDER_VAR(SrcBlend);
	BV_RENDER_VAR(DstBlend);
	BV_RENDER_VAR(BlendOp);
	BV_RENDER_VAR(SrcBlendAlpha);
	BV_RENDER_VAR(DstBlendAlpha);
	BV_RENDER_VAR(AlphaBlendOp);
	BV_RENDER_VAR(RenderTargetWriteMask);
};


struct BlendStateDesc
{
	constexpr BlendStateDesc() = default;

	BlendAttachmentStateDesc m_BlendAttachments[kMaxRenderTargets];
	LogicOp m_LogicOp = LogicOp::kClear;
	bool m_LogicEnable = false;
	bool m_AlphaToCoverageEnable = false;

	BV_RENDER_VAR(LogicOp);
	BV_RENDER_VAR(LogicEnable);
	BV_RENDER_VAR(AlphaToCoverageEnable);

	auto& SetBlendAttachmentStateDesc(u32 index, bool blendEnable, BlendFactor srcBlend, BlendFactor dstBlend, BlendOp blendOp,
		BlendFactor srcBlendAlpha, BlendFactor dstBlendAlpha, BlendOp alphaBlendOp, RenderTargetWriteMask renderTargetWriteMask = RenderTargetWriteMask::kAll)
	{
		m_BlendAttachments[index] =
			BlendAttachmentStateDesc(blendEnable, srcBlend, dstBlend, blendOp, srcBlendAlpha, dstBlendAlpha, alphaBlendOp, renderTargetWriteMask);

		return *this;
	}
};


struct GraphicsPipelineStateDesc
{
	BvFixedVector<IBvShader*, kMaxShaderStages> m_Shaders;
	BvVector<VertexInputDesc> m_VertexInputDescs;
	InputAssemblyStateDesc m_InputAssemblyStateDesc;
	RasterizerStateDesc m_RasterizerStateDesc;
	DepthStencilDesc m_DepthStencilDesc;
	BlendStateDesc m_BlendStateDesc;
	IBvShaderResourceLayout* m_pShaderResourceLayout = nullptr;
	IBvRenderPass* m_pRenderPass = nullptr;
	BvFixedVector<Format, kMaxRenderTargets> m_RenderTargetFormats;
	Format m_DepthStencilFormat = Format::kUnknown;
	u8 m_PatchControlPoints = 0;
	bool m_ShadingRateEnabled = false;
	u32 m_SampleCount = 1;
	u32 m_SampleMask = kMax<u32>;
	u32 m_SubpassIndex = 0;
	u32 m_MultiviewCount = 1;

	BV_RENDER_VAR_PTR(p, ShaderResourceLayout);
	BV_RENDER_VAR_PTR(p, RenderPass);
	BV_RENDER_VAR(DepthStencilFormat);
	BV_RENDER_VAR(PatchControlPoints);
	BV_RENDER_VAR(ShadingRateEnabled);
	BV_RENDER_VAR(SampleCount);
	BV_RENDER_VAR(SampleMask);
	BV_RENDER_VAR(SubpassIndex);
	BV_RENDER_VAR(MultiviewCount);

	GraphicsPipelineStateDesc() = default;

	auto& AddShader(IBvShader* pShader)
	{
		m_Shaders.PushBack(pShader);
		return *this;
	}

	auto& AddVertexInput(const char* pName, u32 binding, Format format, u32 index = 0, InputRate inputRate = InputRate::kPerVertex,
		u32 offset = VertexInputDesc::kAutoOffset, u32 instanceRate = 0)
	{
		m_VertexInputDescs.EmplaceBack(pName, binding, format, index, inputRate, offset, instanceRate);
		return *this;
	}

	auto& SetInputAssemblyState(Topology topology = Topology::kTriangleList, bool primitiveRestart = false,
		IndexFormat indexFormatForPrimitiveRestart = IndexFormat::kUnknown)
	{
		m_InputAssemblyStateDesc = InputAssemblyStateDesc(topology, primitiveRestart, indexFormatForPrimitiveRestart);
		return *this;
	}

	auto& SetRasterizerState(FillMode fillMode, CullMode cullMode, FrontFace frontFace, bool enableDepthClip = false,
		i32 depthBias = 0, f32 depthBiasClamp = 0.0f, f32 depthBiasSlope = 0.0f, bool enableConservativeRasterization = false)
	{
		m_RasterizerStateDesc = RasterizerStateDesc(fillMode, cullMode, frontFace, enableDepthClip, depthBias, depthBiasClamp, depthBiasSlope, enableConservativeRasterization);
		return *this;
	}

	auto& SetRasterizerFillMode(FillMode fillMode)
	{
		m_RasterizerStateDesc.SetFillMode(fillMode);
		return *this;
	}

	auto& SetRasterizerCullMode(CullMode cullMode)
	{
		m_RasterizerStateDesc.SetCullMode(cullMode);
		return *this;
	}

	auto& SetRasterizerFrontFace(FrontFace frontFace)
	{
		m_RasterizerStateDesc.SetFrontFace(frontFace);
		return *this;
	}

	auto& SetRasterizerDepthBias(i32 depthBias = 0, f32 depthBiasClamp = 0.0f, f32 depthBiasSlope = 0.0f)
	{
		m_RasterizerStateDesc.SetDepthBias(depthBias).SetDepthBiasClamp(depthBiasClamp).SetDepthBiasSlope(depthBiasSlope);

		return *this;
	}

	auto& SetDepthStencilState(bool depthTestEnable, bool depthWriteEnable, CompareOp depthOp, bool stencilTestEnable = false,
		u8 stencilReadMask = 0, u8 stencilWriteMask = 0, bool depthBoundsTestEnable = false, const StencilDesc& stencilFront = {}, const StencilDesc& stencilBack = {})
	{
		m_DepthStencilDesc = DepthStencilDesc(depthTestEnable, depthWriteEnable, depthOp, stencilTestEnable, stencilReadMask, stencilWriteMask,
			depthBoundsTestEnable, stencilFront, stencilBack);
		return *this;
	}

	auto& SetDepthTests(bool depthTestEnable, bool depthWriteEnable, CompareOp depthOp)
	{
		m_DepthStencilDesc.SetDepthTestEnable(depthTestEnable).SetDepthWriteEnable(depthWriteEnable).SetDepthOp(depthOp);
		return *this;
	}

	auto& SetStencilTests(bool stencilTestEnable, u8 stencilReadMask, u8 stencilWriteMask)
	{
		m_DepthStencilDesc.SetStencilTestEnable(stencilTestEnable).SetStencilReadMask(stencilReadMask).SetStencilWriteMask(stencilWriteMask);
		return *this;
	}

	auto& SetStencilFrontOp(StencilOp stencilFailOp, StencilOp stencilDepthFailOp, StencilOp stencilPassOp, CompareOp stencilFunc)
	{
		m_DepthStencilDesc.m_StencilFront = StencilDesc(stencilFailOp, stencilDepthFailOp, stencilPassOp, stencilFunc);
		return *this;
	}

	auto& SetStencilBackOp(StencilOp stencilFailOp, StencilOp stencilDepthFailOp, StencilOp stencilPassOp, CompareOp stencilFunc)
	{
		m_DepthStencilDesc.m_StencilBack = StencilDesc(stencilFailOp, stencilDepthFailOp, stencilPassOp, stencilFunc);
		return *this;
	}

	auto& SetBlendState(LogicOp logicOp = LogicOp::kClear, bool logicEnable = false, bool alphaToCoverageEnable = false)
	{
		m_BlendStateDesc.SetLogicOp(logicOp).SetLogicEnable(logicEnable).SetAlphaToCoverageEnable(alphaToCoverageEnable);
		return *this;
	}

	auto& AddRenderTargetFormat(Format format)
	{
		m_RenderTargetFormats.EmplaceBack(format);
		return *this;
	}

	auto& AddRenderTarget(Format format, bool blendEnable, BlendFactor srcBlend, BlendFactor dstBlend, BlendOp blendOp,
		BlendFactor srcBlendAlpha, BlendFactor dstBlendAlpha, BlendOp alphaBlendOp, RenderTargetWriteMask renderTargetWriteMask = RenderTargetWriteMask::kAll)
	{
		m_RenderTargetFormats.EmplaceBack(format);
		m_BlendStateDesc.SetBlendAttachmentStateDesc(m_RenderTargetFormats.Size() - 1, blendEnable, srcBlend, dstBlend, blendOp, srcBlendAlpha,
			dstBlendAlpha, alphaBlendOp, renderTargetWriteMask);
		return *this;
	}
};


struct ComputePipelineStateDesc
{
	ComputePipelineStateDesc() = default;
	ComputePipelineStateDesc(IBvShader* pShader, IBvShaderResourceLayout* pShaderResourceLayout)
		: m_pShader(pShader), m_pShaderResourceLayout(pShaderResourceLayout) {}

	IBvShader* m_pShader = nullptr;
	IBvShaderResourceLayout* m_pShaderResourceLayout = nullptr;

	BV_RENDER_VAR_PTR(p, Shader);
	BV_RENDER_VAR_PTR(p, ShaderResourceLayout);
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


enum class RayTracingInstanceFlags : u32
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


enum class RayTracingAccelerationStructurePostBuildType : u8
{
	kSize,
	kCompactedSize,
};


struct RayTracingAccelerationStructurePostBuildDesc
{
	RayTracingAccelerationStructurePostBuildType m_Type = RayTracingAccelerationStructurePostBuildType::kSize;
	IBvBuffer* m_pDstBuffer = nullptr;
	u64 m_DstBufferOffset = 0;
};


struct RayTracingAccelerationStructureGeometryDesc
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
		u32 m_Count;
		u32 m_Stride;
	};

	struct InstanceDesc
	{
		u32 m_InstanceCount;
	};

	BvStringId m_Id;
	union
	{
		TriangleDesc m_Triangle{};
		AABBDesc m_AABB;
		InstanceDesc m_Instance;
	};

	RayTracingGeometryFlags m_Flags = RayTracingGeometryFlags::kNone;
	RayTracingGeometryType m_Type = RayTracingGeometryType::kUnknown;
};


struct RayTracingAccelerationStructureBuildGeometryDesc
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

	struct InstanceDesc
	{
		IBvBuffer* m_pBuffer;
		u64 m_Offset;
		u32 m_Count;
	};

	BvStringId m_Id;
	union
	{
		TriangleDesc m_Triangle{};
		AABBDesc m_AABB;
		InstanceDesc m_Instance;
	};

	RayTracingGeometryFlags m_Flags = RayTracingGeometryFlags::kNone;
	RayTracingGeometryType m_Type = RayTracingGeometryType::kUnknown;
};


struct RayTracingAccelerationStructureBuildDesc
{
	BvVector<RayTracingAccelerationStructureBuildGeometryDesc> m_Geometries;
	IBvAccelerationStructure* m_pAS = nullptr;
	IBvBuffer* m_pScratchBuffer = nullptr;
	u64 m_ScratchBufferOffset = 0;
	bool m_Update = false;
	RayTracingAccelerationStructureType m_Type = RayTracingAccelerationStructureType::kUnknown;
};


struct RayTracingAccelerationStructureInstanceDesc
{
	RayTracingAccelerationStructureInstanceDesc()
		: m_Transform{ { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } }, m_InstanceId(0), m_InstanceMask(0),
		m_ShaderBindingTableIndex(0), m_Flags(RayTracingInstanceFlags::kNone), m_AccelerationStructure(0) {}

	f32 m_Transform[3][4];
	u32 m_InstanceId : 24;
	u32 m_InstanceMask : 8;
	u32 m_ShaderBindingTableIndex : 24;
	RayTracingInstanceFlags m_Flags : 8;
	u64 m_AccelerationStructure;
};


enum class RayTracingAccelerationStructureCopyMode : u8
{
	kClone,
	kCompact
};


struct RayTracingAccelerationStructureCopyDesc
{
	IBvAccelerationStructure* m_pSrc = nullptr;
	IBvAccelerationStructure* m_pDst = nullptr;
	RayTracingAccelerationStructureCopyMode m_CopyMode = RayTracingAccelerationStructureCopyMode::kClone;
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
	BvVector<RayTracingAccelerationStructureGeometryDesc> m_Geometries;
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
	ShaderGroupDesc() = default;
	ShaderGroupDesc(const char* pName, u32 general)
		: m_pName(pName), m_Type(ShaderGroupType::kGeneral), m_General(general) {}
	ShaderGroupDesc(const char* pName, u32 closestHit, u32 anyHit)
		: m_pName(pName), m_Type(ShaderGroupType::kTriangles), m_ClosestHit(closestHit), m_AnyHit(anyHit) {}
	ShaderGroupDesc(const char* pName, u32 closestHit, u32 anyHit, u32 intersection)
		: m_pName(pName), m_Type(ShaderGroupType::kProcedural), m_ClosestHit(closestHit), m_AnyHit(anyHit), m_Intersection(intersection) {}

	static constexpr u32 kUnusedShader = kU32Max;

	const char* m_pName = nullptr;
	ShaderGroupType m_Type = ShaderGroupType::kNone;
	u32 m_General = kUnusedShader;
	u32 m_ClosestHit = kUnusedShader;
	u32 m_AnyHit = kUnusedShader;
	u32 m_Intersection = kUnusedShader;

	friend bool operator<(const ShaderGroupDesc& lhs, const ShaderGroupDesc& rhs)
	{
		return u32(lhs.m_Type) < u32(rhs.m_Type);
	}
};


struct RayTracingPipelineStateDesc
{
	BvVector<IBvShader*> m_Shaders;
	BvVector<ShaderGroupDesc> m_ShaderGroupDescs;
	IBvShaderResourceLayout* m_pShaderResourceLayout = nullptr;
	u32 m_MaxPipelineRayRecursionDepth = 0;
	u32 m_MaxPayloadSize = 0;
	u32 m_MaxAttributeSize = 0;
	bool m_ForcePayloadAndAttributeSizes = false; // Vulkan only

	BV_RENDER_VAR_PTR(p, ShaderResourceLayout);
	BV_RENDER_VAR(MaxPipelineRayRecursionDepth);
	BV_RENDER_VAR(MaxPayloadSize);
	BV_RENDER_VAR(MaxAttributeSize);
	BV_RENDER_VAR(ForcePayloadAndAttributeSizes);

	RayTracingPipelineStateDesc() = default;

	auto& AddShader(IBvShader* pShader)
	{
		m_Shaders.EmplaceBack(pShader);
		return *this;
	}

	auto& AddGeneralShaderGroup(const char* pName, u32 shaderIndex)
	{
		m_ShaderGroupDescs.EmplaceBack(pName, shaderIndex);
		return *this;
	}

	auto& AddRayGenShaderGroup(const char* pName, u32 shaderIndex)
	{
		return AddGeneralShaderGroup(pName, shaderIndex);
	}

	auto& AddMissShaderGroup(const char* pName, u32 shaderIndex)
	{
		return AddGeneralShaderGroup(pName, shaderIndex);
	}

	auto& AddCallableShaderGroup(const char* pName, u32 shaderIndex)
	{
		return AddGeneralShaderGroup(pName, shaderIndex);
	}

	auto& AddTriangleShaderGroup(const char* pName, u32 closestHitShaderIndex, u32 anyHitShaderIndex)
	{
		m_ShaderGroupDescs.EmplaceBack(pName, closestHitShaderIndex, anyHitShaderIndex);
		return *this;
	}

	auto& AddProceduralShaderGroup(const char* pName, u32 closestHitShaderIndex, u32 anyHitShaderIndex, u32 intersectionShaderIndex)
	{
		m_ShaderGroupDescs.EmplaceBack(pName, closestHitShaderIndex, anyHitShaderIndex, intersectionShaderIndex);
		return *this;
	}
};


struct PipelineCacheInitData
{
	u64 m_Size = 0;
	const void* m_pInitData = nullptr;

	BV_RENDER_VAR(Size);
	BV_RENDER_VAR_PTR(p, InitData);
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
		: m_CommandType(commandType), m_RequireDedicated(requireDedicated) {}
	constexpr CommandContextDesc(u32 contextGroupIndex)
		: m_ContextGroupIndex(contextGroupIndex) {}

	u32 m_ContextGroupIndex = kU32Max;
	CommandType m_CommandType = CommandType::kNone;
	bool m_RequireDedicated = false;
};


struct RenderEngineDesc
{
	bool m_EnableDebugLayer = true;

	// TODO: Implement these
	using DebugMessageCallbackFn = void(*)();
	IBvMemoryArena* m_pMemoryArena = nullptr;
	u64 m_MaxVirtualMemoryReserveSize = 0;
	bool m_UseVirtualMemory = false;
};


struct RenderDeviceDesc
{
	struct ContextGroupDesc
	{
		u32 m_GroupIndex;
		u32 m_ContextCount;
	};

	enum BackendOption : u8
	{
		kNone,
		kVulkan,
		kD3D12
	};

	BvFixedVector<ContextGroupDesc, kMaxContextGroupCount> m_ContextGroups;
	u32 m_GPUIndex = kU32Max;
	BackendOption m_ExtendedBackendOptions = BackendOption::kNone;

	union
	{
		struct Vulkan
		{
			u32 m_MaxSetsPerDescriptorPool;
			u32 m_QueryPoolSizes[kQueryTypeCount];
			u32 m_AccelerationStructureQueryPoolSize;
		} m_Vulkan;
		struct D3D12
		{
			u32 m_GPUDescriptorHeapSizes[2];
			u32 m_CPUDescriptorHeapSizes[4];
			u32 m_MaxDescriptorAllocationsPerPool;
			u32 m_QueryHeapSizes[kQueryTypeCount];
		} m_D3D12;
	};

};


using GPUList = BvFixedVector<BvGPUInfo*, kMaxDevices>;