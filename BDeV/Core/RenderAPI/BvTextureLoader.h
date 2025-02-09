#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvVector.h"


class IBvTextureBlob
{
	BV_NOCOPYMOVE(IBvTextureBlob);

public:
	struct Info
	{
		u32 m_Width;
		u32 m_Height;
		u32 m_Depth;
		u32 m_MipLevels;
		u32 m_ArraySize;
		u32 m_PlaneCount;
		TextureType m_TextureType;
		Format m_Format;
		bool m_IsCubeMap;
	};

	virtual const Info& GetInfo() = 0;
	virtual const BvVector<SubresourceData>& GetSubresources() const = 0;

protected:
	IBvTextureBlob() {}
	virtual ~IBvTextureBlob() {}
};


class IBvTextureLoader
{
	BV_NOCOPYMOVE(IBvTextureLoader);

public:
	enum class Result : u8
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

	virtual IBvTextureLoader::Result LoadTextureFromFile(const char* pFilename, IBvTextureBlob** ppTextureBlob) = 0;
	virtual IBvTextureLoader::Result LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob** ppTextureBlob) = 0;
	virtual void DestroyTexture(IBvTextureBlob** ppTextureBlob) = 0;

protected:
	IBvTextureLoader() {}
	virtual ~IBvTextureLoader() {}
};