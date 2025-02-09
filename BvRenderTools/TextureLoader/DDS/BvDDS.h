#pragma once


#include "BvRenderTools/TextureLoader/BvDDSTextureLoader.h"


#ifdef __clang__
#pragma clang diagnostic ignored "-Wtautological-type-limit-compare"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4062)

#define D3DX12_NO_STATE_OBJECT_HELPERS

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)													\
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |		    \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

#ifndef DDS_ALPHA_MODE_DEFINED
#define DDS_ALPHA_MODE_DEFINED
enum DDS_ALPHA_MODE : uint32_t
{
	DDS_ALPHA_MODE_UNKNOWN = 0,
	DDS_ALPHA_MODE_STRAIGHT = 1,
	DDS_ALPHA_MODE_PREMULTIPLIED = 2,
	DDS_ALPHA_MODE_OPAQUE = 3,
	DDS_ALPHA_MODE_CUSTOM = 4,
};
#endif

enum DDS_LOADER_FLAGS : uint8_t
{
	DDS_LOADER_DEFAULT = 0,
	DDS_LOADER_FORCE_SRGB = 0x1,
	DDS_LOADER_MIP_RESERVE = 0x8,
};


// Copy of the DXGIFormat enumeration, so there's no need to rely on Microsoft's header file
enum DXGIFormat
{
	DXGI_FORMAT_UNKNOWN = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	DXGI_FORMAT_R32G32B32A32_UINT = 3,
	DXGI_FORMAT_R32G32B32A32_SINT = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS = 5,
	DXGI_FORMAT_R32G32B32_FLOAT = 6,
	DXGI_FORMAT_R32G32B32_UINT = 7,
	DXGI_FORMAT_R32G32B32_SINT = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	DXGI_FORMAT_R16G16B16A16_UINT = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	DXGI_FORMAT_R16G16B16A16_SINT = 14,
	DXGI_FORMAT_R32G32_TYPELESS = 15,
	DXGI_FORMAT_R32G32_FLOAT = 16,
	DXGI_FORMAT_R32G32_UINT = 17,
	DXGI_FORMAT_R32G32_SINT = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	DXGI_FORMAT_R10G10B10A2_UINT = 25,
	DXGI_FORMAT_R11G11B10_FLOAT = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	DXGI_FORMAT_R8G8B8A8_UINT = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	DXGI_FORMAT_R8G8B8A8_SINT = 32,
	DXGI_FORMAT_R16G16_TYPELESS = 33,
	DXGI_FORMAT_R16G16_FLOAT = 34,
	DXGI_FORMAT_R16G16_UNORM = 35,
	DXGI_FORMAT_R16G16_UINT = 36,
	DXGI_FORMAT_R16G16_SNORM = 37,
	DXGI_FORMAT_R16G16_SINT = 38,
	DXGI_FORMAT_R32_TYPELESS = 39,
	DXGI_FORMAT_D32_FLOAT = 40,
	DXGI_FORMAT_R32_FLOAT = 41,
	DXGI_FORMAT_R32_UINT = 42,
	DXGI_FORMAT_R32_SINT = 43,
	DXGI_FORMAT_R24G8_TYPELESS = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
	DXGI_FORMAT_R8G8_TYPELESS = 48,
	DXGI_FORMAT_R8G8_UNORM = 49,
	DXGI_FORMAT_R8G8_UINT = 50,
	DXGI_FORMAT_R8G8_SNORM = 51,
	DXGI_FORMAT_R8G8_SINT = 52,
	DXGI_FORMAT_R16_TYPELESS = 53,
	DXGI_FORMAT_R16_FLOAT = 54,
	DXGI_FORMAT_D16_UNORM = 55,
	DXGI_FORMAT_R16_UNORM = 56,
	DXGI_FORMAT_R16_UINT = 57,
	DXGI_FORMAT_R16_SNORM = 58,
	DXGI_FORMAT_R16_SINT = 59,
	DXGI_FORMAT_R8_TYPELESS = 60,
	DXGI_FORMAT_R8_UNORM = 61,
	DXGI_FORMAT_R8_UINT = 62,
	DXGI_FORMAT_R8_SNORM = 63,
	DXGI_FORMAT_R8_SINT = 64,
	DXGI_FORMAT_A8_UNORM = 65,
	DXGI_FORMAT_R1_UNORM = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
	DXGI_FORMAT_BC1_TYPELESS = 70,
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB = 72,
	DXGI_FORMAT_BC2_TYPELESS = 73,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB = 75,
	DXGI_FORMAT_BC3_TYPELESS = 76,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	DXGI_FORMAT_BC4_TYPELESS = 79,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC4_SNORM = 81,
	DXGI_FORMAT_BC5_TYPELESS = 82,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC5_SNORM = 84,
	DXGI_FORMAT_B5G6R5_UNORM = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	DXGI_FORMAT_BC6H_TYPELESS = 94,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC6H_SF16 = 96,
	DXGI_FORMAT_BC7_TYPELESS = 97,
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	DXGI_FORMAT_AYUV = 100,
	DXGI_FORMAT_Y410 = 101,
	DXGI_FORMAT_Y416 = 102,
	DXGI_FORMAT_NV12 = 103,
	DXGI_FORMAT_P010 = 104,
	DXGI_FORMAT_P016 = 105,
	DXGI_FORMAT_420_OPAQUE = 106,
	DXGI_FORMAT_YUY2 = 107,
	DXGI_FORMAT_Y210 = 108,
	DXGI_FORMAT_Y216 = 109,
	DXGI_FORMAT_NV11 = 110,
	DXGI_FORMAT_AI44 = 111,
	DXGI_FORMAT_IA44 = 112,
	DXGI_FORMAT_P8 = 113,
	DXGI_FORMAT_A8P8 = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM = 115,
	DXGI_FORMAT_P208 = 130,
	DXGI_FORMAT_V208 = 131,
	DXGI_FORMAT_V408 = 132,
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
	DXGI_FORMAT_FORCE_UINT = 0xffffffff
};


enum DDSResourceDimension
{
	kUnknown = 0,
	//kBuffer = 1,
	kTexture1D = 2,
	kTexture2D = 3,
	kTexture3D = 4
};


constexpr uint32_t kReqMipLevels = 15;
constexpr uint32_t kReqSubresources = 30720;
constexpr uint32_t kReqTexture1DArrayAxisDimension = 2048;
constexpr uint32_t kReqTexture1DUDimension = 16384;
constexpr uint32_t kReqTexture2DArrayAxisDimension = 2048;
constexpr uint32_t kReqTexture2DUOrVDimension = 16384;
constexpr uint32_t kReqTexture3DArrayAxisDimension = 2048;
constexpr uint32_t kReqTexture3DUVOrWDimension = 16384;
constexpr uint32_t kReqTextureCubeDimension = 16384;

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push,1)

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

enum DDS_MISC_FLAGS2
{
	DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};


constexpr u32 MakeFourCC(u32 ch0, u32 ch1, u32 ch2, u32 ch3)
{
	return ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24));
}

constexpr u32 kFourCC_DDS = MakeFourCC('D', 'D', 'S', ' ');

const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

// dwFlags (DDS_HEADER)
constexpr u32 kDDSD_CAPS = 0x1; // Required in every.dds file.
constexpr u32 kDDSD_HEIGHT = 0x2; // Required in every.dds file.
constexpr u32 kDDSD_WIDTH = 0x4; // Required in every.dds file.
constexpr u32 kDDSD_PITCH = 0x8; // Required when pitch is provided for an uncompressed texture.
constexpr u32 kDDSD_PIXELFORMAT = 0x1000; // Required in every.dds file.
constexpr u32 kDDSD_MIPMAPCOUNT = 0x20000; // Required in a mipmapped texture.
constexpr u32 kDDSD_LINEARSIZE = 0x80000; // Required when pitch is provided for a compressed texture.
constexpr u32 kDDSD_DEPTH = 0x800000; // Required in a depth texture.
constexpr u32 kDDSD_REQUIRED_FLAGS = kDDSD_CAPS | kDDSD_WIDTH | kDDSD_HEIGHT | kDDSD_PIXELFORMAT;

// dwCaps (DDS_HEADER)
constexpr u32 kDDSCAPS_COMPLEX = 0x8; // Optional; must be used on any file that contains more than one surface(a mipmap, a cubic environment map, or mipmapped volume texture).
constexpr u32 kDDSCAPS_MIPMAP = 0x400000; // Optional; should be used for a mipmap.
constexpr u32 kDDSCAPS_TEXTURE = 0x1000; // Required

// dwCaps2 (DDS_HEADER)
constexpr u32 kDDSCAPS2_CUBEMAP = 0x200; // Required for a cube map.
constexpr u32 kDDSCAPS2_CUBEMAP_POSITIVEX = 0x400; // Required when these surfaces are stored in a cube map.
constexpr u32 kDDSCAPS2_CUBEMAP_NEGATIVEX = 0x800; // Required when these surfaces are stored in a cube map.
constexpr u32 kDDSCAPS2_CUBEMAP_POSITIVEY = 0x1000; // Required when these surfaces are stored in a cube map.
constexpr u32 kDDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000; // Required when these surfaces are stored in a cube map.
constexpr u32 kDDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000; // Required when these surfaces are stored in a cube map.
constexpr u32 kDDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000; // Required when these surfaces are stored in a cube map.
constexpr u32 kDDSCAPS2_CUBEMAP_ALLFACES = kDDSCAPS2_CUBEMAP | kDDSCAPS2_CUBEMAP_POSITIVEX | kDDSCAPS2_CUBEMAP_NEGATIVEX | kDDSCAPS2_CUBEMAP_POSITIVEY
	| kDDSCAPS2_CUBEMAP_NEGATIVEY | kDDSCAPS2_CUBEMAP_POSITIVEZ | kDDSCAPS2_CUBEMAP_NEGATIVEZ;

constexpr u32 kDDSCAPS2_VOLUME = 0x200000; // Required for a volume texture.

// dwFlags (DDS_PIXELFORMAT)
constexpr u32 kDDPF_ALPHAPIXELS = 0x1; // Texture contains alpha data; dwRGBAlphaBitMask contains valid data.
constexpr u32 kDDPF_ALPHA = 0x2; // Used in some older DDS files for alpha channel only uncompressed data(dwRGBBitCount contains the alpha channel bitcount; dwABitMask contains valid data)
constexpr u32 kDDPF_FOURCC = 0x4; // Texture contains compressed RGB data; dwFourCC contains valid data.
constexpr u32 kDDPF_RGB = 0x40; // Texture contains uncompressed RGB data; dwRGBBitCount and the RGB masks(dwRBitMask, dwGBitMask, dwBBitMask) contain valid data.
constexpr u32 kDDPF_YUV = 0x200; // Used in some older DDS files for YUV uncompressed data(dwRGBBitCount contains the YUV bit count; dwRBitMask contains the Y mask, dwGBitMask contains the U mask, dwBBitMask contains the V mask)
constexpr u32 kDDPF_LUMINANCE = 0x20000; // Used in some older DDS files for single channel color uncompressed data(dwRGBBitCount contains the luminance channel bit count; dwRBitMask contains the channel mask).Can be combined with DDPF_ALPHAPIXELS for a two channel DDS file.

// dwFourCC (DDS_PIXELFORMAT)
constexpr u32 kFourCC_DX10 = MakeFourCC('D', 'X', '1', '0');
constexpr u32 kFourCC_DXT1 = MakeFourCC('D', 'X', 'T', '1');
constexpr u32 kFourCC_DXT2 = MakeFourCC('D', 'X', 'T', '2');
constexpr u32 kFourCC_DXT3 = MakeFourCC('D', 'X', 'T', '3');
constexpr u32 kFourCC_DXT4 = MakeFourCC('D', 'X', 'T', '4');
constexpr u32 kFourCC_DXT5 = MakeFourCC('D', 'X', 'T', '5');

constexpr u32 kFourCC_ATI1 = MakeFourCC('A', 'T', 'I', '1');
constexpr u32 kFourCC_BC4U = MakeFourCC('B', 'C', '4', 'U');
constexpr u32 kFourCC_BC4S = MakeFourCC('B', 'C', '4', 'S');

constexpr u32 kFourCC_ATI2 = MakeFourCC('A', 'T', 'I', '2');
constexpr u32 kFourCC_BC5U = MakeFourCC('B', 'C', '5', 'U');
constexpr u32 kFourCC_BC5S = MakeFourCC('B', 'C', '5', 'S');

constexpr u32 kFourCC_RGBG = MakeFourCC('R', 'G', 'B', 'G');
constexpr u32 kFourCC_GRGB = MakeFourCC('G', 'R', 'G', 'B');
constexpr u32 kFourCC_YUY2 = MakeFourCC('Y', 'U', 'Y', '2');

// miscFlag (DDS_HEADER_DXT10)
constexpr u32 kDDS_RESOURCE_MISC_TEXTURECUBE = 0x4; // Indicates a 2D texture is a cube-map texture.

// miscFlags2 (DDS_HEADER_DXT10)
constexpr u32 kDDS_ALPHA_MODE_UNKNOWN = 0x0; // Alpha channel content is unknown.This is the value for legacy files, which typically is assumed to be 'straight' alpha.
constexpr u32 kDDS_ALPHA_MODE_STRAIGHT = 0x1; // Any alpha channel content is presumed to use straight alpha.
constexpr u32 kDDS_ALPHA_MODE_PREMULTIPLIED = 0x2; // Any alpha channel content is using premultiplied alpha.The only legacy file formats that indicate this information are 'DX2' and 'DX4'.
constexpr u32 kDDS_ALPHA_MODE_OPAQUE = 0x3; // Any alpha channel content is all set to fully opaque.
constexpr u32 kDDS_ALPHA_MODE_CUSTOM = 0x4; // Any alpha channel content is being used as a 4th channel and is not intended to represent transparency(straight or premultiplied).

struct DDS_PIXELFORMAT
{
	uint32_t    size;
	uint32_t    flags;
	uint32_t    fourCC;
	uint32_t    RGBBitCount;
	uint32_t    RBitMask;
	uint32_t    GBitMask;
	uint32_t    BBitMask;
	uint32_t    ABitMask;
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

struct DDS_HEADER_DXT10
{
	DXGIFormat	    dxgiFormat;
	uint32_t        resourceDimension;
	uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
	uint32_t        arraySize;
	uint32_t        miscFlags2;
};

constexpr u32 kMinDDSHeaderSize = sizeof(u32) + sizeof(DDS_HEADER);
constexpr u32 kMinDDSDX10HeaderSize = kMinDDSHeaderSize + sizeof(DDS_HEADER_DXT10);

#pragma pack(pop)


IBvTextureLoader::Result LoadDDSTexture(const uint8_t* bitData, size_t bitSize, IBvTextureBlob::Info& textureInfo, BvVector<SubresourceData>& subresources) noexcept(false);


class BvDDS
{
	BV_NOCOPY(BvDDS);

public:
	BvDDS();
	BvDDS(const void* pData, size_t size);
	BvDDS(BvDDS&& rhs) noexcept;
	BvDDS& operator=(BvDDS&& rhs) noexcept;

	DXGIFormat GetDXGIFormat() const;
	Format GetFormat() const;
	TextureType GetType() const;
	u32 GetWidth() const;
	u32 GetHeight() const;
	u32 GetDepth() const;
	u32 GetMipCount() const;
	u32 GetArraySize() const;
	u32 GetPlaneCount() const;
	bool IsCubeMap() const;
	DDS_ALPHA_MODE GetAlphaMode() const;
	bool IsValid() const;
	bool HasDX10Header() const;

	u32 GetSubresourceCount() const;
	void GetSubresourceData(SubresourceData& subresource, u32 mipLevel = 0, u32 arraySlice = 0, u32 planeSlice = 0) const;
	u32 GetAllSubresourceData(SubresourceData* pSubresources, u32 subresourceCount) const;

private:
	void ProcessHeaders();
	bool ValidateData();
	DXGIFormat GetDXGIFormat(const DDS_PIXELFORMAT& ddpf) const;

private:
	const u8* m_pData = nullptr;
	size_t m_Size = 0;
	const DDS_HEADER* m_pDDSHeader = nullptr;
	const DDS_HEADER_DXT10* m_pDDSHeaderDX10 = nullptr;
	const u8* m_pTextureData = nullptr;
	size_t m_TextureDataSize = 0;
};