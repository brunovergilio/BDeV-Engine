#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/Render/BvTexture.h"


struct TextureDataDesc
{
	u32 m_Width;
	u32 m_Height;
	u32 m_DepthOrArraySize;
	u32 m_MipCount;
	BvVector<u8> m_Data;
	struct SubresourceData
	{
		const u8* m_pData;
		size_t m_Offset;
		size_t m_RowPitch;
		size_t m_SlicePitch;
	};
	BvVector<SubresourceData> m_Subresources;
	TextureType m_TextureType;
	Format m_Format;
	bool m_IsCubeMap;
};


class IBvTextureLoader
{
	BV_NOCOPYMOVE(IBvTextureLoader);

public:
	virtual void LoadTexture(const char* const pFilename) = 0;

protected:
	IBvTextureLoader() {}
	virtual ~IBvTextureLoader() {}
};


class IBvTextureBlob
{
	BV_NOCOPYMOVE(IBvTextureBlob);

public:
	virtual const TextureDataDesc& GetTextureData() = 0;

protected:
	IBvTextureBlob() {}
	virtual ~IBvTextureBlob() {}
};