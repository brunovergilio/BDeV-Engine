#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvTexture.h"


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
		u32 m_LayerCount;
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


class IBvTextureLoader
{
	BV_NOCOPYMOVE(IBvTextureLoader);

public:
	virtual LoaderResult LoadTextureFromFile(const char* pFilename, IBvTextureBlob*& pTextureBlob) = 0;
	virtual LoaderResult LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob*& pTextureBlob) = 0;
	virtual void DestroyTexture(IBvTextureBlob*& pTextureBlob) = 0;

protected:
	IBvTextureLoader() {}
	virtual ~IBvTextureLoader() {}
};


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvTextureLoader* GetTextureLoader();
	}
}