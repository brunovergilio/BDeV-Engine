#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Utils/BvObject.h"


BV_OBJECT_DEFINE_ID(IBvTextureBlob, "8fd865cd-3b81-4a99-9497-0d0d0a07df59");
class IBvTextureBlob : public BvObjectBase
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
BV_OBJECT_ENABLE_ID_OPERATOR(IBvTextureBlob);


BV_OBJECT_DEFINE_ID(IBvTextureLoader, "8dcfd9b9-56cf-447d-ba1c-0c545b29d706");
class IBvTextureLoader : public BvObjectBase
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

protected:
	IBvTextureLoader() {}
	~IBvTextureLoader() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvTextureLoader);