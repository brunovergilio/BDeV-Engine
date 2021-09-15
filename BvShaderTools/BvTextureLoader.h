#pragma once


#include "BvCore/BvCore.h"
#include "BvCore/Container/BvVector.h"


class IBvTextureLoader
{
	BV_NOCOPYMOVE(IBvTextureLoader);

public:

protected:
	IBvTextureLoader() {}
	virtual ~IBvTextureLoader() {}
};


struct TextureDataDesc
{
	u32 m_Width;
	u32 m_Height;
	u32 m_Depth;
	u32 m_ArraySize;
	u32 m_MipCount;
	BvVector<u8> m_Data;
	struct SubresourceData
	{
		u8* m_pBytes;
		size_t m_Offset;
		size_t m_RowPitch;
		size_t m_SlicePitch;
	};
	BvVector<SubresourceData> m_Subresources;
	TextureType m_TextureType;
	u8 m_DXGIFormat;
	bool m_IsCubeMap;
};


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


// Standard version
LoaderResult __cdecl LoadDDSTextureFromMemory(
	_In_ ID3D12Device* d3dDevice,
	_In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
	size_t ddsDataSize,
	_Outptr_ ID3D12Resource** texture,
	std::vector<D3D12_SUBRESOURCE_DATA>& subresources,
	size_t maxsize = 0,
	_Out_opt_ DDS_ALPHA_MODE* alphaMode = nullptr,
	_Out_opt_ bool* isCubeMap = nullptr);

LoaderResult __cdecl LoadDDSTextureFromFile(
	_In_ ID3D12Device* d3dDevice,
	_In_z_ const wchar_t* szFileName,
	_Outptr_ ID3D12Resource** texture,
	std::unique_ptr<uint8_t[]>& ddsData,
	std::vector<D3D12_SUBRESOURCE_DATA>& subresources,
	size_t maxsize = 0,
	_Out_opt_ DDS_ALPHA_MODE* alphaMode = nullptr,
	_Out_opt_ bool* isCubeMap = nullptr);

// Extended version
LoaderResult __cdecl LoadDDSTextureFromMemoryEx(
	_In_ ID3D12Device* d3dDevice,
	_In_reads_bytes_(ddsDataSize) const uint8_t* ddsData,
	size_t ddsDataSize,
	size_t maxsize,
	D3D12_RESOURCE_FLAGS resFlags,
	unsigned int loadFlags,
	_Outptr_ ID3D12Resource** texture,
	std::vector<D3D12_SUBRESOURCE_DATA>& subresources,
	_Out_opt_ DDS_ALPHA_MODE* alphaMode = nullptr,
	_Out_opt_ bool* isCubeMap = nullptr);

LoaderResult __cdecl LoadDDSTextureFromFileEx(
	_In_ ID3D12Device* d3dDevice,
	_In_z_ const wchar_t* szFileName,
	size_t maxsize,
	D3D12_RESOURCE_FLAGS resFlags,
	unsigned int loadFlags,
	_Outptr_ ID3D12Resource** texture,
	std::unique_ptr<uint8_t[]>& ddsData,
	std::vector<D3D12_SUBRESOURCE_DATA>& subresources,
	_Out_opt_ DDS_ALPHA_MODE* alphaMode = nullptr,
	_Out_opt_ bool* isCubeMap = nullptr);