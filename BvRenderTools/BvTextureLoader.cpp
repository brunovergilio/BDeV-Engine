#include "BvTextureLoader.h"
#include "BDeV/System/File/BvFile.h"


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

enum class LoaderResult : uint8_t
{
	kOk,
	kBadPointer,
	kArithmeticOverflow,
	kNotSupported,
	kInvalidData,
	kInvalidArg,
	kEOF,
	kOutOfMemory,
	kFail
};

// Copy of the DXGIFormat enumeration, so there's no need to rely on Microsoft's header file
enum class DXGIFormat : u8
{
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_R32G32B32A32_TYPELESS,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_UINT,
	DXGI_FORMAT_R32G32B32A32_SINT,
	DXGI_FORMAT_R32G32B32_TYPELESS,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32_UINT,
	DXGI_FORMAT_R32G32B32_SINT,
	DXGI_FORMAT_R16G16B16A16_TYPELESS,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_UNORM,
	DXGI_FORMAT_R16G16B16A16_UINT,
	DXGI_FORMAT_R16G16B16A16_SNORM,
	DXGI_FORMAT_R16G16B16A16_SINT,
	DXGI_FORMAT_R32G32_TYPELESS,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32_UINT,
	DXGI_FORMAT_R32G32_SINT,
	DXGI_FORMAT_R32G8X24_TYPELESS,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
	DXGI_FORMAT_R10G10B10A2_TYPELESS,
	DXGI_FORMAT_R10G10B10A2_UNORM,
	DXGI_FORMAT_R10G10B10A2_UINT,
	DXGI_FORMAT_R11G11B10_FLOAT,
	DXGI_FORMAT_R8G8B8A8_TYPELESS,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	DXGI_FORMAT_R8G8B8A8_UINT,
	DXGI_FORMAT_R8G8B8A8_SNORM,
	DXGI_FORMAT_R8G8B8A8_SINT,
	DXGI_FORMAT_R16G16_TYPELESS,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16G16_UNORM,
	DXGI_FORMAT_R16G16_UINT,
	DXGI_FORMAT_R16G16_SNORM,
	DXGI_FORMAT_R16G16_SINT,
	DXGI_FORMAT_R32_TYPELESS,
	DXGI_FORMAT_D32_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32_UINT,
	DXGI_FORMAT_R32_SINT,
	DXGI_FORMAT_R24G8_TYPELESS,
	DXGI_FORMAT_D24_UNORM_S8_UINT,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT,
	DXGI_FORMAT_R8G8_TYPELESS,
	DXGI_FORMAT_R8G8_UNORM,
	DXGI_FORMAT_R8G8_UINT,
	DXGI_FORMAT_R8G8_SNORM,
	DXGI_FORMAT_R8G8_SINT,
	DXGI_FORMAT_R16_TYPELESS,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_D16_UNORM,
	DXGI_FORMAT_R16_UNORM,
	DXGI_FORMAT_R16_UINT,
	DXGI_FORMAT_R16_SNORM,
	DXGI_FORMAT_R16_SINT,
	DXGI_FORMAT_R8_TYPELESS,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8_UINT,
	DXGI_FORMAT_R8_SNORM,
	DXGI_FORMAT_R8_SINT,
	DXGI_FORMAT_A8_UNORM,
	DXGI_FORMAT_R1_UNORM,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
	DXGI_FORMAT_R8G8_B8G8_UNORM,
	DXGI_FORMAT_G8R8_G8B8_UNORM,
	DXGI_FORMAT_BC1_TYPELESS,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC1_UNORM_SRGB,
	DXGI_FORMAT_BC2_TYPELESS,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC2_UNORM_SRGB,
	DXGI_FORMAT_BC3_TYPELESS,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC3_UNORM_SRGB,
	DXGI_FORMAT_BC4_TYPELESS,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC4_SNORM,
	DXGI_FORMAT_BC5_TYPELESS,
	DXGI_FORMAT_BC5_UNORM,
	DXGI_FORMAT_BC5_SNORM,
	DXGI_FORMAT_B5G6R5_UNORM,
	DXGI_FORMAT_B5G5R5A1_UNORM,
	DXGI_FORMAT_B8G8R8A8_UNORM,
	DXGI_FORMAT_B8G8R8X8_UNORM,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
	DXGI_FORMAT_B8G8R8A8_TYPELESS,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
	DXGI_FORMAT_B8G8R8X8_TYPELESS,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
	DXGI_FORMAT_BC6H_TYPELESS,
	DXGI_FORMAT_BC6H_UF16,
	DXGI_FORMAT_BC6H_SF16,
	DXGI_FORMAT_BC7_TYPELESS,
	DXGI_FORMAT_BC7_UNORM,
	DXGI_FORMAT_BC7_UNORM_SRGB,
	DXGI_FORMAT_AYUV,
	DXGI_FORMAT_Y410,
	DXGI_FORMAT_Y416,
	DXGI_FORMAT_NV12,
	DXGI_FORMAT_P010,
	DXGI_FORMAT_P016,
	DXGI_FORMAT_420_OPAQUE,
	DXGI_FORMAT_YUY2,
	DXGI_FORMAT_Y210,
	DXGI_FORMAT_Y216,
	DXGI_FORMAT_NV11,
	DXGI_FORMAT_AI44,
	DXGI_FORMAT_IA44,
	DXGI_FORMAT_P8,
	DXGI_FORMAT_A8P8,
	DXGI_FORMAT_B4G4R4A4_UNORM,
	DXGI_FORMAT_P208,
	DXGI_FORMAT_V208,
	DXGI_FORMAT_V408,
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
	DXGI_FORMAT_FORCE_UINT
};


enum DDSResourceDimension
{
	kUnknown = 0,
	kBuffer = 1,
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

const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

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

#pragma pack(pop)

//--------------------------------------------------------------------------------------
// Standard version
LoaderResult LoadDDSTextureFromMemory(
	const u8* ddsData,
	size_t ddsDataSize,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	size_t maxsize = 0,
	DDS_ALPHA_MODE* alphaMode = nullptr,
	bool* isCubeMap = nullptr);

LoaderResult LoadDDSTextureFromFile(
	const char* szFileName,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	size_t maxsize = 0,
	DDS_ALPHA_MODE* alphaMode = nullptr,
	bool* isCubeMap = nullptr);

LoaderResult LoadDDSTextureFromFile(
	const wchar_t* szFileName,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	size_t maxsize = 0,
	DDS_ALPHA_MODE* alphaMode = nullptr,
	bool* isCubeMap = nullptr);

// Extended version
LoaderResult LoadDDSTextureFromMemoryEx(
	const u8* ddsData,
	size_t ddsDataSize,
	size_t maxsize,
	unsigned int loadFlags,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	DDS_ALPHA_MODE* alphaMode = nullptr,
	bool* isCubeMap = nullptr);

LoaderResult LoadDDSTextureFromFileEx(
	const char* szFileName,
	size_t maxsize,
	unsigned int loadFlags,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	DDS_ALPHA_MODE* alphaMode = nullptr,
	bool* isCubeMap = nullptr);

LoaderResult LoadDDSTextureFromFileEx(
	const wchar_t* szFileName,
	size_t maxsize,
	unsigned int loadFlags,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	DDS_ALPHA_MODE* alphaMode = nullptr,
	bool* isCubeMap = nullptr);
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
namespace
{
	inline uint32_t CountMips(uint32_t width, uint32_t height) noexcept
	{
		if (width == 0 || height == 0)
			return 0;

		uint32_t count = 1;
		while (width > 1 || height > 1)
		{
			width >>= 1;
			height >>= 1;
			count++;
		}
		return count;
	}

	//--------------------------------------------------------------------------------------
	LoaderResult LoadTextureDataFromMemory(
		const uint8_t* ddsData,
		size_t ddsDataSize,
		const DDS_HEADER** header,
		const uint8_t** bitData,
		size_t* bitSize) noexcept
	{
		if (!header || !bitData || !bitSize)
		{
			return LoaderResult::kBadPointer;
		}

		*bitSize = 0;

		if (ddsDataSize > UINT32_MAX)
		{
			return LoaderResult::kFail;
		}

		if (ddsDataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
		{
			return LoaderResult::kFail;
		}

		// DDS files always start with the same magic number ("DDS ")
		auto dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData);
		if (dwMagicNumber != DDS_MAGIC)
		{
			return LoaderResult::kFail;
		}

		auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData + sizeof(uint32_t));

		// Verify header to validate DDS file
		if (hdr->size != sizeof(DDS_HEADER) ||
			hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
		{
			return LoaderResult::kFail;
		}

		// Check for DX10 extension
		bool bDXT10Header = false;
		if ((hdr->ddspf.flags & DDS_FOURCC) &&
			(MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
		{
			// Must be long enough for both headers and magic value
			if (ddsDataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)))
			{
				return LoaderResult::kFail;
			}

			bDXT10Header = true;
		}

		// setup the pointers in the process request
		*header = hdr;
		auto offset = sizeof(uint32_t)
			+ sizeof(DDS_HEADER)
			+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
		*bitData = ddsData + offset;
		*bitSize = ddsDataSize - offset;

		return LoaderResult::kOk;
	}

	//--------------------------------------------------------------------------------------
	LoaderResult LoadTextureDataFromFile(
		const wchar_t* fileName,
		u8*& ddsData,
		const DDS_HEADER** header,
		const uint8_t** bitData,
		size_t* bitSize) noexcept
	{
		if (!header || !bitData || !bitSize)
		{
			return LoaderResult::kBadPointer;
		}

		*bitSize = 0;

		BvFile inFile(fileName);
		if (!inFile.IsValid())
			return LoaderResult::kFail;

		i64 fileLen = inFile.GetSize();

		// Need at least enough data to fill the header and magic number to be a valid DDS
		if (fileLen < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
			return LoaderResult::kFail;

		ddsData = new uint8_t[size_t(fileLen)];
		if (!ddsData)
			return LoaderResult::kFail;

		auto read = inFile.Read(reinterpret_cast<char*>(ddsData), fileLen);
		if (read == -1)
		{
			delete[] ddsData;
			return LoaderResult::kFail;
		}

		inFile.Close();

		size_t len = fileLen;

		// DDS files always start with the same magic number ("DDS ")
		auto dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData);
		if (dwMagicNumber != DDS_MAGIC)
		{
			delete[] ddsData;
			return LoaderResult::kFail;
		}

		auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData + sizeof(uint32_t));

		// Verify header to validate DDS file
		if (hdr->size != sizeof(DDS_HEADER) ||
			hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
		{
			delete[] ddsData;
			return LoaderResult::kFail;
		}

		// Check for DX10 extension
		bool bDXT10Header = false;
		if ((hdr->ddspf.flags & DDS_FOURCC) &&
			(MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
		{
			// Must be long enough for both headers and magic value
			if (len < (sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)))
			{
				delete[] ddsData;
				return LoaderResult::kFail;
			}

			bDXT10Header = true;
		}

		// setup the pointers in the process request
		*header = hdr;
		auto offset = sizeof(uint32_t) + sizeof(DDS_HEADER)
			+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
		*bitData = ddsData + offset;
		*bitSize = len - offset;

		return LoaderResult::kOk;
	}

	//--------------------------------------------------------------------------------------
	// Return the BPP for a particular format
	//--------------------------------------------------------------------------------------
	size_t BitsPerPixel(DXGIFormat fmt) noexcept
	{
		switch (fmt)
		{
		case DXGIFormat::DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGIFormat::DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGIFormat::DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R32G32B32_UINT:
		case DXGIFormat::DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGIFormat::DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGIFormat::DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGIFormat::DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGIFormat::DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGIFormat::DXGI_FORMAT_R32G32_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R32G32_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R32G32_UINT:
		case DXGIFormat::DXGI_FORMAT_R32G32_SINT:
		case DXGIFormat::DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGIFormat::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGIFormat::DXGI_FORMAT_Y416:
		case DXGIFormat::DXGI_FORMAT_Y210:
		case DXGIFormat::DXGI_FORMAT_Y216:
			return 64;

		case DXGIFormat::DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGIFormat::DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGIFormat::DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGIFormat::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGIFormat::DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGIFormat::DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGIFormat::DXGI_FORMAT_R16G16_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R16G16_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R16G16_UNORM:
		case DXGIFormat::DXGI_FORMAT_R16G16_UINT:
		case DXGIFormat::DXGI_FORMAT_R16G16_SNORM:
		case DXGIFormat::DXGI_FORMAT_R16G16_SINT:
		case DXGIFormat::DXGI_FORMAT_R32_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_D32_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R32_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R32_UINT:
		case DXGIFormat::DXGI_FORMAT_R32_SINT:
		case DXGIFormat::DXGI_FORMAT_R24G8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGIFormat::DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGIFormat::DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGIFormat::DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGIFormat::DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGIFormat::DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGIFormat::DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGIFormat::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGIFormat::DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_AYUV:
		case DXGIFormat::DXGI_FORMAT_Y410:
		case DXGIFormat::DXGI_FORMAT_YUY2:
			return 32;

		case DXGIFormat::DXGI_FORMAT_P010:
		case DXGIFormat::DXGI_FORMAT_P016:
		case DXGIFormat::DXGI_FORMAT_V408:
			return 24;

		case DXGIFormat::DXGI_FORMAT_R8G8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R8G8_UNORM:
		case DXGIFormat::DXGI_FORMAT_R8G8_UINT:
		case DXGIFormat::DXGI_FORMAT_R8G8_SNORM:
		case DXGIFormat::DXGI_FORMAT_R8G8_SINT:
		case DXGIFormat::DXGI_FORMAT_R16_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R16_FLOAT:
		case DXGIFormat::DXGI_FORMAT_D16_UNORM:
		case DXGIFormat::DXGI_FORMAT_R16_UNORM:
		case DXGIFormat::DXGI_FORMAT_R16_UINT:
		case DXGIFormat::DXGI_FORMAT_R16_SNORM:
		case DXGIFormat::DXGI_FORMAT_R16_SINT:
		case DXGIFormat::DXGI_FORMAT_B5G6R5_UNORM:
		case DXGIFormat::DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGIFormat::DXGI_FORMAT_A8P8:
		case DXGIFormat::DXGI_FORMAT_B4G4R4A4_UNORM:
		case DXGIFormat::DXGI_FORMAT_P208:
		case DXGIFormat::DXGI_FORMAT_V208:
			return 16;

		case DXGIFormat::DXGI_FORMAT_NV12:
		case DXGIFormat::DXGI_FORMAT_420_OPAQUE:
		case DXGIFormat::DXGI_FORMAT_NV11:
			return 12;

		case DXGIFormat::DXGI_FORMAT_R8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_R8_UNORM:
		case DXGIFormat::DXGI_FORMAT_R8_UINT:
		case DXGIFormat::DXGI_FORMAT_R8_SNORM:
		case DXGIFormat::DXGI_FORMAT_R8_SINT:
		case DXGIFormat::DXGI_FORMAT_A8_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC2_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC2_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_BC3_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC3_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_BC5_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC5_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC5_SNORM:
		case DXGIFormat::DXGI_FORMAT_BC6H_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC6H_UF16:
		case DXGIFormat::DXGI_FORMAT_BC6H_SF16:
		case DXGIFormat::DXGI_FORMAT_BC7_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC7_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC7_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_AI44:
		case DXGIFormat::DXGI_FORMAT_IA44:
		case DXGIFormat::DXGI_FORMAT_P8:
			return 8;

		case DXGIFormat::DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGIFormat::DXGI_FORMAT_BC1_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC1_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_BC4_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC4_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC4_SNORM:
			return 4;

		default:
			return 0;
		}
	}

	//--------------------------------------------------------------------------------------
	// Get surface information for a particular format
	//--------------------------------------------------------------------------------------
	LoaderResult GetSurfaceInfo(
		size_t width,
		size_t height,
		DXGIFormat fmt,
		size_t* outNumBytes,
		size_t* outRowBytes,
		size_t* outNumRows) noexcept
	{
		uint64_t numBytes = 0;
		uint64_t rowBytes = 0;
		uint64_t numRows = 0;

		bool bc = false;
		bool packed = false;
		bool planar = false;
		size_t bpe = 0;
		switch (fmt)
		{
		case DXGIFormat::DXGI_FORMAT_BC1_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC1_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_BC4_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC4_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC4_SNORM:
			bc = true;
			bpe = 8;
			break;

		case DXGIFormat::DXGI_FORMAT_BC2_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC2_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_BC3_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC3_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGIFormat::DXGI_FORMAT_BC5_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC5_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC5_SNORM:
		case DXGIFormat::DXGI_FORMAT_BC6H_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC6H_UF16:
		case DXGIFormat::DXGI_FORMAT_BC6H_SF16:
		case DXGIFormat::DXGI_FORMAT_BC7_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_BC7_UNORM:
		case DXGIFormat::DXGI_FORMAT_BC7_UNORM_SRGB:
			bc = true;
			bpe = 16;
			break;

		case DXGIFormat::DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGIFormat::DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGIFormat::DXGI_FORMAT_YUY2:
			packed = true;
			bpe = 4;
			break;

		case DXGIFormat::DXGI_FORMAT_Y210:
		case DXGIFormat::DXGI_FORMAT_Y216:
			packed = true;
			bpe = 8;
			break;

		case DXGIFormat::DXGI_FORMAT_NV12:
		case DXGIFormat::DXGI_FORMAT_420_OPAQUE:
		case DXGIFormat::DXGI_FORMAT_P208:
			planar = true;
			bpe = 2;
			break;

		case DXGIFormat::DXGI_FORMAT_P010:
		case DXGIFormat::DXGI_FORMAT_P016:
			planar = true;
			bpe = 4;
			break;

		default:
			break;
		}

		if (bc)
		{
			uint64_t numBlocksWide = 0;
			if (width > 0)
			{
				numBlocksWide = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
			}
			uint64_t numBlocksHigh = 0;
			if (height > 0)
			{
				numBlocksHigh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
			}
			rowBytes = numBlocksWide * bpe;
			numRows = numBlocksHigh;
			numBytes = rowBytes * numBlocksHigh;
		}
		else if (packed)
		{
			rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
			numRows = uint64_t(height);
			numBytes = rowBytes * height;
		}
		else if (fmt == DXGIFormat::DXGI_FORMAT_NV11)
		{
			rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
			numRows = uint64_t(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
			numBytes = rowBytes * numRows;
		}
		else if (planar)
		{
			rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
			numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
			numRows = height + ((uint64_t(height) + 1u) >> 1);
		}
		else
		{
			size_t bpp = BitsPerPixel(fmt);
			if (!bpp)
				return LoaderResult::kInvalidArg;

			rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
			numRows = uint64_t(height);
			numBytes = rowBytes * height;
		}

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
		static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
		if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
			return LoaderResult::kArithmeticOverflow;
#else
		static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
#endif

		if (outNumBytes)
		{
			*outNumBytes = static_cast<size_t>(numBytes);
		}
		if (outRowBytes)
		{
			*outRowBytes = static_cast<size_t>(rowBytes);
		}
		if (outNumRows)
		{
			*outNumRows = static_cast<size_t>(numRows);
		}

		return LoaderResult::kOk;
	}


	//--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

	DXGIFormat GetDXGIFormat(const DDS_PIXELFORMAT& ddpf) noexcept
	{
		if (ddpf.flags & DDS_RGB)
		{
			// Note that sRGB formats are written using the "DX10" extended header

			switch (ddpf.RGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGIFormat::DXGI_FORMAT_R8G8B8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				{
					return DXGIFormat::DXGI_FORMAT_B8G8R8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0))
				{
					return DXGIFormat::DXGI_FORMAT_B8G8R8X8_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0) aka D3DFMT_X8B8G8R8

				// Note that many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assume
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGIFormat::DXGI_FORMAT_R10G10B10A2_UNORM format directly

				// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				{
					return DXGIFormat::DXGI_FORMAT_R10G10B10A2_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
				{
					return DXGIFormat::DXGI_FORMAT_R16G16_UNORM;
				}

				if (ISBITMASK(0xffffffff, 0, 0, 0))
				{
					// Only 32-bit color channel format in D3D9 was R32F
					return DXGIFormat::DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
				}
				break;

			case 24:
				// No 24bpp DXGI formats aka D3DFMT_R8G8B8
				break;

			case 16:
				if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				{
					return DXGIFormat::DXGI_FORMAT_B5G5R5A1_UNORM;
				}
				if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0))
				{
					return DXGIFormat::DXGI_FORMAT_B5G6R5_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0) aka D3DFMT_X1R5G5B5

				if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
				{
					return DXGIFormat::DXGI_FORMAT_B4G4R4A4_UNORM;
				}

				// NVTT versions 1.x wrote this as RGB instead of LUMINANCE
				if (ISBITMASK(0x00ff, 0, 0, 0xff00))
				{
					return DXGIFormat::DXGI_FORMAT_R8G8_UNORM;
				}
				if (ISBITMASK(0xffff, 0, 0, 0))
				{
					return DXGIFormat::DXGI_FORMAT_R16_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0) aka D3DFMT_X4R4G4B4

				// No 3:3:2:8 or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_A8P8, etc.
				break;

			case 8:
				// NVTT versions 1.x wrote this as RGB instead of LUMINANCE
				if (ISBITMASK(0xff, 0, 0, 0))
				{
					return DXGIFormat::DXGI_FORMAT_R8_UNORM;
				}

				// No 3:3:2 or paletted DXGI formats aka D3DFMT_R3G3B2, D3DFMT_P8
				break;
			}
		}
		else if (ddpf.flags & DDS_LUMINANCE)
		{
			switch (ddpf.RGBBitCount)
			{
			case 16:
				if (ISBITMASK(0xffff, 0, 0, 0))
				{
					return DXGIFormat::DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
				if (ISBITMASK(0x00ff, 0, 0, 0xff00))
				{
					return DXGIFormat::DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
				break;

			case 8:
				if (ISBITMASK(0xff, 0, 0, 0))
				{
					return DXGIFormat::DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x0f,0,0,0xf0) aka D3DFMT_A4L4

				if (ISBITMASK(0x00ff, 0, 0, 0xff00))
				{
					return DXGIFormat::DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
				}
				break;
			}
		}
		else if (ddpf.flags & DDS_ALPHA)
		{
			if (8 == ddpf.RGBBitCount)
			{
				return DXGIFormat::DXGI_FORMAT_A8_UNORM;
			}
		}
		else if (ddpf.flags & DDS_BUMPDUDV)
		{
			switch (ddpf.RGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGIFormat::DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
				}
				if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
				{
					return DXGIFormat::DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
				break;

			case 16:
				if (ISBITMASK(0x00ff, 0xff00, 0, 0))
				{
					return DXGIFormat::DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
				}
				break;
			}

			// No DXGI format maps to DDPF_BUMPLUMINANCE aka D3DFMT_L6V5U5, D3DFMT_X8L8V8U8
		}
		else if (ddpf.flags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC1_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC3_UNORM;
			}

			// While pre-multiplied alpha isn't directly supported by the DXGI formats,
			// they are basically the same as these BC formats so they can be mapped
			if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC3_UNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC4_SNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_BC5_SNORM;
			}

			// BC6H and BC7 are written using the "DX10" extended header

			if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_R8G8_B8G8_UNORM;
			}
			if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_G8R8_G8B8_UNORM;
			}

			if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
			{
				return DXGIFormat::DXGI_FORMAT_YUY2;
			}

			// Check for D3DFORMAT enums being set here
			switch (ddpf.fourCC)
			{
			case 36: // D3DFMT_A16B16G16R16
				return DXGIFormat::DXGI_FORMAT_R16G16B16A16_UNORM;

			case 110: // D3DFMT_Q16W16V16U16
				return DXGIFormat::DXGI_FORMAT_R16G16B16A16_SNORM;

			case 111: // D3DFMT_R16F
				return DXGIFormat::DXGI_FORMAT_R16_FLOAT;

			case 112: // D3DFMT_G16R16F
				return DXGIFormat::DXGI_FORMAT_R16G16_FLOAT;

			case 113: // D3DFMT_A16B16G16R16F
				return DXGIFormat::DXGI_FORMAT_R16G16B16A16_FLOAT;

			case 114: // D3DFMT_R32F
				return DXGIFormat::DXGI_FORMAT_R32_FLOAT;

			case 115: // D3DFMT_G32R32F
				return DXGIFormat::DXGI_FORMAT_R32G32_FLOAT;

			case 116: // D3DFMT_A32B32G32R32F
				return DXGIFormat::DXGI_FORMAT_R32G32B32A32_FLOAT;

				// No DXGI format maps to D3DFMT_CxV8U8
			}
		}

		return DXGIFormat::DXGI_FORMAT_UNKNOWN;
	}

#undef ISBITMASK


	//--------------------------------------------------------------------------------------
	DXGIFormat MakeSRGB(DXGIFormat format) noexcept
	{
		switch (format)
		{
		case DXGIFormat::DXGI_FORMAT_R8G8B8A8_UNORM:
			return DXGIFormat::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case DXGIFormat::DXGI_FORMAT_BC1_UNORM:
			return DXGIFormat::DXGI_FORMAT_BC1_UNORM_SRGB;

		case DXGIFormat::DXGI_FORMAT_BC2_UNORM:
			return DXGIFormat::DXGI_FORMAT_BC2_UNORM_SRGB;

		case DXGIFormat::DXGI_FORMAT_BC3_UNORM:
			return DXGIFormat::DXGI_FORMAT_BC3_UNORM_SRGB;

		case DXGIFormat::DXGI_FORMAT_B8G8R8A8_UNORM:
			return DXGIFormat::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case DXGIFormat::DXGI_FORMAT_B8G8R8X8_UNORM:
			return DXGIFormat::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

		case DXGIFormat::DXGI_FORMAT_BC7_UNORM:
			return DXGIFormat::DXGI_FORMAT_BC7_UNORM_SRGB;

		default:
			return format;
		}
	}


	//--------------------------------------------------------------------------------------
	inline bool IsDepthStencil(DXGIFormat fmt) noexcept
	{
		switch (fmt)
		{
		case DXGIFormat::DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGIFormat::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGIFormat::DXGI_FORMAT_D32_FLOAT:
		case DXGIFormat::DXGI_FORMAT_R24G8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGIFormat::DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGIFormat::DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGIFormat::DXGI_FORMAT_D16_UNORM:
			return true;

		default:
			return false;
		}
	}


	//--------------------------------------------------------------------------------------
	inline void AdjustPlaneResource(
		DXGIFormat fmt,
		size_t height,
		size_t slicePlane,
		TextureDataDesc::SubresourceData& res) noexcept
	{
		switch (fmt)
		{
		case DXGIFormat::DXGI_FORMAT_NV12:
		case DXGIFormat::DXGI_FORMAT_P010:
		case DXGIFormat::DXGI_FORMAT_P016:
			if (!slicePlane)
			{
				// Plane 0
				res.m_SlicePitch = res.m_RowPitch * static_cast<long>(height);
			}
			else
			{
				// Plane 1
				res.m_pData = reinterpret_cast<const uint8_t*>(res.m_pData) + uintptr_t(res.m_RowPitch) * height;
				res.m_SlicePitch = res.m_RowPitch * ((static_cast<long>(height) + 1) >> 1);
			}
			break;

		case DXGIFormat::DXGI_FORMAT_NV11:
			if (!slicePlane)
			{
				// Plane 0
				res.m_SlicePitch = res.m_RowPitch * static_cast<long>(height);
			}
			else
			{
				// Plane 1
				res.m_pData = reinterpret_cast<const uint8_t*>(res.m_pData) + uintptr_t(res.m_RowPitch) * height;
				res.m_RowPitch = (res.m_RowPitch >> 1);
				res.m_SlicePitch = res.m_RowPitch * static_cast<long>(height);
			}
			break;
		}
	}


	//--------------------------------------------------------------------------------------
	LoaderResult FillInitData(size_t width,
		size_t height,
		size_t depth,
		size_t mipCount,
		size_t arraySize,
		size_t numberOfPlanes,
		DXGIFormat format,
		size_t maxsize,
		size_t bitSize,
		const uint8_t* bitData,
		size_t& twidth,
		size_t& theight,
		size_t& tdepth,
		size_t& skipMip,
		BvVector<TextureDataDesc::SubresourceData>& initData)
	{
		if (!bitData)
		{
			return LoaderResult::kBadPointer;
		}

		skipMip = 0;
		twidth = 0;
		theight = 0;
		tdepth = 0;

		size_t NumBytes = 0;
		size_t RowBytes = 0;
		const uint8_t* pEndBits = bitData + bitSize;

		initData.Clear();

		for (size_t p = 0; p < numberOfPlanes; ++p)
		{
			const uint8_t* pSrcBits = bitData;

			for (size_t j = 0; j < arraySize; j++)
			{
				size_t w = width;
				size_t h = height;
				size_t d = depth;
				for (size_t i = 0; i < mipCount; i++)
				{
					LoaderResult hr = GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, nullptr);
					if (hr != LoaderResult::kOk)
						return hr;

					if (NumBytes > UINT32_MAX || RowBytes > UINT32_MAX)
						return LoaderResult::kArithmeticOverflow;

					if ((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
					{
						if (!twidth)
						{
							twidth = w;
							theight = h;
							tdepth = d;
						}

						TextureDataDesc::SubresourceData res =
						{
							pSrcBits,
							0,
							RowBytes,
							NumBytes
						};

						AdjustPlaneResource(format, h, p, res);

						res.m_Offset = (size_t)(pSrcBits - bitData);

						initData.EmplaceBack(res);
					}
					else if (!j)
					{
						// Count number of skipped mipmaps (first item only)
						++skipMip;
					}

					if (pSrcBits + (NumBytes * d) > pEndBits)
					{
						return LoaderResult::kEOF;
					}

					pSrcBits += NumBytes * d;

					w = w >> 1;
					h = h >> 1;
					d = d >> 1;
					if (w == 0)
					{
						w = 1;
					}
					if (h == 0)
					{
						h = 1;
					}
					if (d == 0)
					{
						d = 1;
					}
				}
			}
		}

		return initData.Empty() ? LoaderResult::kFail : LoaderResult::kOk;
	}

	//--------------------------------------------------------------------------------------
	LoaderResult CreateTextureFromDDS(const DDS_HEADER* header,
		const uint8_t* bitData,
		size_t bitSize,
		size_t maxsize,
		unsigned int loadFlags,
		BvVector<TextureDataDesc::SubresourceData>& subresources,
		bool* outIsCubeMap) noexcept(false)
	{
		LoaderResult hr = LoaderResult::kOk;

		uint32_t width = header->width;
		uint32_t height = header->height;
		uint32_t depth = header->depth;

		DDSResourceDimension resDim = DDSResourceDimension::kUnknown;
		uint32_t arraySize = 1;
		DXGIFormat format = DXGIFormat::DXGI_FORMAT_UNKNOWN;
		bool isCubeMap = false;

		size_t mipCount = header->mipMapCount;
		if (0 == mipCount)
		{
			mipCount = 1;
		}

		if ((header->ddspf.flags & DDS_FOURCC) &&
			(MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
		{
			auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>(reinterpret_cast<const char*>(header) + sizeof(DDS_HEADER));

			arraySize = d3d10ext->arraySize;
			if (arraySize == 0)
			{
				return LoaderResult::kInvalidData;
			}

			switch (d3d10ext->dxgiFormat)
			{
			case DXGIFormat::DXGI_FORMAT_AI44:
			case DXGIFormat::DXGI_FORMAT_IA44:
			case DXGIFormat::DXGI_FORMAT_P8:
			case DXGIFormat::DXGI_FORMAT_A8P8:
				return LoaderResult::kNotSupported;

			default:
				if (BitsPerPixel(d3d10ext->dxgiFormat) == 0)
				{
					return LoaderResult::kNotSupported;
				}
			}

			format = d3d10ext->dxgiFormat;

			switch (d3d10ext->resourceDimension)
			{
			case DDSResourceDimension::kTexture1D:
				// D3DX writes 1D textures with a fixed Height of 1
				if ((header->flags & DDS_HEIGHT) && height != 1)
				{
					return LoaderResult::kInvalidData;
				}
				height = depth = 1;
				break;

			case DDSResourceDimension::kTexture2D:
				if (d3d10ext->miscFlag & 0x4 /* RESOURCE_MISC_TEXTURECUBE */)
				{
					arraySize *= 6;
					isCubeMap = true;
				}
				depth = 1;
				break;

			case DDSResourceDimension::kTexture3D:
				if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
				{
					return LoaderResult::kInvalidData;
				}

				if (arraySize > 1)
				{
					return LoaderResult::kNotSupported;
				}
				break;

			default:
				return LoaderResult::kNotSupported;
			}

			resDim = static_cast<DDSResourceDimension>(d3d10ext->resourceDimension);
		}
		else
		{
			format = GetDXGIFormat(header->ddspf);

			if (format == DXGIFormat::DXGI_FORMAT_UNKNOWN)
			{
				return LoaderResult::kNotSupported;
			}

			if (header->flags & DDS_HEADER_FLAGS_VOLUME)
			{
				resDim = DDSResourceDimension::kTexture3D;
			}
			else
			{
				if (header->caps2 & DDS_CUBEMAP)
				{
					// We require all six faces to be defined
					if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
					{
						return LoaderResult::kNotSupported;
					}

					arraySize = 6;
					isCubeMap = true;
				}

				depth = 1;
				resDim = DDSResourceDimension::kTexture2D;

				// Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
			}

			BvAssert(BitsPerPixel(format) != 0, "BPP can't be 0");
		}

		// Bound sizes (for security purposes we don't trust DDS file metadata larger than the Direct3D hardware requirements)
		if (mipCount > kReqMipLevels)
		{
			return LoaderResult::kNotSupported;
		}

		switch (resDim)
		{
		case DDSResourceDimension::kTexture1D:
			if ((arraySize > kReqTexture1DArrayAxisDimension) ||
				(width > kReqTexture1DUDimension))
			{
				return LoaderResult::kNotSupported;
			}
			break;

		case DDSResourceDimension::kTexture2D:
			if (isCubeMap)
			{
				// This is the right bound because we set arraySize to (NumCubes*6) above
				if ((arraySize > kReqTexture2DArrayAxisDimension) ||
					(width > kReqTextureCubeDimension) ||
					(height > kReqTextureCubeDimension))
				{
					return LoaderResult::kNotSupported;
				}
			}
			else if ((arraySize > kReqTexture2DArrayAxisDimension) ||
				(width > kReqTexture2DUOrVDimension) ||
				(height > kReqTexture2DUOrVDimension))
			{
				return LoaderResult::kNotSupported;
			}
			break;

		case DDSResourceDimension::kTexture3D:
			if ((arraySize > 1) ||
				(width > kReqTexture3DUVOrWDimension) ||
				(height > kReqTexture3DUVOrWDimension) ||
				(depth > kReqTexture3DUVOrWDimension))
			{
				return LoaderResult::kNotSupported;
			}
			break;

		default:
			return LoaderResult::kNotSupported;
		}

		// TODO: Figure this bit out - I'd like to not have to rely on the D3D12 API here,
		// so I might query for the list of available formats and their plane counts
		//uint32_t numberOfPlanes = D3D12GetFormatPlaneCount(d3dDevice, format);
		uint32_t numberOfPlanes = 1;
		if (!numberOfPlanes)
			return LoaderResult::kInvalidArg;

		if ((numberOfPlanes > 1) && IsDepthStencil(format))
		{
			// DirectX 12 uses planes for stencil, DirectX 11 does not
			return LoaderResult::kNotSupported;
		}

		if (outIsCubeMap != nullptr)
		{
			*outIsCubeMap = isCubeMap;
		}

		// Create the texture
		size_t numberOfResources = (resDim == DDSResourceDimension::kTexture3D)	? 1 : arraySize;
		numberOfResources *= mipCount;
		numberOfResources *= numberOfPlanes;

		if (numberOfResources > kReqSubresources)
			return LoaderResult::kInvalidArg;

		subresources.Reserve(numberOfResources);

		size_t skipMip = 0;
		size_t twidth = 0;
		size_t theight = 0;
		size_t tdepth = 0;
		hr = FillInitData(width, height, depth, mipCount, arraySize,
			numberOfPlanes, format,
			maxsize, bitSize, bitData,
			twidth, theight, tdepth, skipMip, subresources);

		if (hr != LoaderResult::kOk)
		{
			subresources.Clear();
		}

		return hr;
	}

	//--------------------------------------------------------------------------------------
	DDS_ALPHA_MODE GetAlphaMode(const DDS_HEADER* header) noexcept
	{
		if (header->ddspf.flags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
			{
				auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>(reinterpret_cast<const uint8_t*>(header) + sizeof(DDS_HEADER));
				auto mode = static_cast<DDS_ALPHA_MODE>(d3d10ext->miscFlags2 & DDS_MISC_FLAGS2_ALPHA_MODE_MASK);
				switch (mode)
				{
				case DDS_ALPHA_MODE_STRAIGHT:
				case DDS_ALPHA_MODE_PREMULTIPLIED:
				case DDS_ALPHA_MODE_OPAQUE:
				case DDS_ALPHA_MODE_CUSTOM:
					return mode;

				case DDS_ALPHA_MODE_UNKNOWN:
				default:
					break;
				}
			}
			else if ((MAKEFOURCC('D', 'X', 'T', '2') == header->ddspf.fourCC)
				|| (MAKEFOURCC('D', 'X', 'T', '4') == header->ddspf.fourCC))
			{
				return DDS_ALPHA_MODE_PREMULTIPLIED;
			}
		}

		return DDS_ALPHA_MODE_UNKNOWN;
	}
} // anonymous namespace


//--------------------------------------------------------------------------------------
LoaderResult LoadDDSTextureFromMemory(
	const u8* ddsData,
	size_t ddsDataSize,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	size_t maxsize,
	DDS_ALPHA_MODE* alphaMode,
	bool* isCubeMap)
{
	return LoadDDSTextureFromMemoryEx(
		ddsData,
		ddsDataSize,
		maxsize,
		DDS_LOADER_DEFAULT,
		subresources,
		alphaMode,
		isCubeMap);
}


LoaderResult LoadDDSTextureFromMemoryEx(
	const u8* ddsData,
	size_t ddsDataSize,
	size_t maxsize,
	unsigned int loadFlags,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	DDS_ALPHA_MODE* alphaMode,
	bool* isCubeMap)
{
	if (alphaMode)
	{
		*alphaMode = DDS_ALPHA_MODE_UNKNOWN;
	}
	if (isCubeMap)
	{
		*isCubeMap = false;
	}

	if (!ddsData)
	{
		return LoaderResult::kInvalidArg;
	}

	// Validate DDS file in memory
	const DDS_HEADER* header = nullptr;
	const uint8_t* bitData = nullptr;
	size_t bitSize = 0;

	LoaderResult hr = LoadTextureDataFromMemory(ddsData,
		ddsDataSize,
		&header,
		&bitData,
		&bitSize
	);
	if (hr != LoaderResult::kOk)
	{
		return hr;
	}

	hr = CreateTextureFromDDS(header, bitData, bitSize, maxsize,
		loadFlags, subresources, isCubeMap);
	if (hr == LoaderResult::kOk)
	{
		if (alphaMode)
			*alphaMode = GetAlphaMode(header);
	}

	return hr;
}

//--------------------------------------------------------------------------------------
LoaderResult LoadDDSTextureFromFile(
	const char* szFileName,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	size_t maxsize,
	DDS_ALPHA_MODE* alphaMode,
	bool* isCubeMap)
{
	return LoadDDSTextureFromFileEx(
		szFileName,
		maxsize,
		DDS_LOADER_DEFAULT,
		ddsData,
		subresources,
		alphaMode,
		isCubeMap);
}

LoaderResult LoadDDSTextureFromFile(
	const wchar_t* szFileName,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	size_t maxsize,
	DDS_ALPHA_MODE* alphaMode,
	bool* isCubeMap)
{
	return LoadDDSTextureFromFileEx(
		szFileName,
		maxsize,
		DDS_LOADER_DEFAULT,
		ddsData,
		subresources,
		alphaMode,
		isCubeMap);
}

LoaderResult LoadDDSTextureFromFileEx(
	const char* szFileName,
	size_t maxsize,
	unsigned int loadFlags,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	DDS_ALPHA_MODE* alphaMode,
	bool* isCubeMap)
{
	mbstate_t mbState{};
	auto len = 1 + mbsrtowcs(nullptr, &szFileName, 0, &mbState);
	BvVector<wchar_t> widePath(len);
	mbsrtowcs(widePath.Data(), &szFileName, widePath.Size(), &mbState);

	return LoadDDSTextureFromFileEx(
		widePath.Data(),
		maxsize,
		loadFlags,
		ddsData,
		subresources,
		alphaMode,
		isCubeMap);
}

LoaderResult LoadDDSTextureFromFileEx(
	const wchar_t* szFileName,
	size_t maxsize,
	unsigned int loadFlags,
	u8*& ddsData,
	BvVector<TextureDataDesc::SubresourceData>& subresources,
	DDS_ALPHA_MODE* alphaMode,
	bool* isCubeMap)
{
	if (alphaMode)
	{
		*alphaMode = DDS_ALPHA_MODE_UNKNOWN;
	}
	if (isCubeMap)
	{
		*isCubeMap = false;
	}

	if (!szFileName)
	{
		return LoaderResult::kInvalidArg;
	}

	const DDS_HEADER* header = nullptr;
	const uint8_t* bitData = nullptr;
	size_t bitSize = 0;

	LoaderResult hr = LoadTextureDataFromFile(szFileName,
		ddsData,
		&header,
		&bitData,
		&bitSize
	);
	if (hr != LoaderResult::kOk)
	{
		return hr;
	}

	hr = CreateTextureFromDDS(header, bitData, bitSize, maxsize,
		loadFlags, subresources, isCubeMap);

	if (hr == LoaderResult::kOk)
	{
		if (alphaMode)
			*alphaMode = GetAlphaMode(header);
	}

	delete[] ddsData;

	return hr;
}