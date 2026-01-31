#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvVector.h"


class IBvTextureBlob : public BvRCObj
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
	~IBvTextureBlob() {}
};
BV_OBJECT_DEFINE_ID(IBvTextureBlob, "8fd865cd-3b81-4a99-9497-0d0d0a07df59");


class IBvTextureLoader : public BvRCObj
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

	template<BvRCType T>
	BV_INLINE Result LoadTextureFromFile(const char* pFilename, T** ppObj)
	{
		return LoadTextureFromFileImpl(pFilename, reinterpret_cast<void**>(ppObj));
	}

	template<BvRCType T>
	BV_INLINE Result LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, T** ppObj)
	{
		return LoadTextureFromMemoryImpl(pBuffer, bufferSize, reinterpret_cast<void**>(ppObj));
	}

protected:
	IBvTextureLoader() {}
	~IBvTextureLoader() {}

	virtual Result LoadTextureFromFileImpl(const char* pFilename, void** ppObj) = 0;
	virtual Result LoadTextureFromMemoryImpl(const void* pBuffer, u64 bufferSize, void** ppObj) = 0;
};
BV_OBJECT_DEFINE_ID(IBvTextureLoader, "8dcfd9b9-56cf-447d-ba1c-0c545b29d706");