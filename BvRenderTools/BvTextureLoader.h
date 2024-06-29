#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/RenderAPI/BvTexture.h"


struct TextureDataDesc
{
	u32 m_Width;
	u32 m_Height;
	u32 m_Depth;
	u32 m_MipLevels;
	u32 m_LayerCount;
	BvVector<u8> m_Data;
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