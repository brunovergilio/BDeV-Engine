#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/System/File/BvPath.h"
#include "../Utils/WolfCommon.h"



struct SpriteTableType
{
	i16 m_Width, m_Height;
	i16 m_OriginX, m_OriginY;
	i16 m_Xl, m_Yl;
	i16 m_Xh, m_Yh;
	i16 m_Shifts;
};

struct SpriteType
{
	u16 sourceoffset[MAXSHIFTS];
	u16 planesize[MAXSHIFTS];
	u16 width[MAXSHIFTS];
	u8 data[];
};

struct PicTableType
{
	i16 m_Width, m_Height;
};


struct FontStruct
{
	i16 m_Height;
	i16 m_Location[256];
	u8 m_Width[256];
};

struct SpriteCompShape
{
	u16 m_LeftPixel, m_RightPixel;
	u16 m_Offsets[64];
};

typedef struct
{
	u16 codeofs[65];
	u16 width[65];
	u8 code[];
}	t_compscale;


constexpr auto kSpriteSize = 64 * 64;


struct VSwap
{
	struct Page
	{
		u32		m_Offset;	// Offset of chunk into file
		u16		m_Length;	// Length of the chunk

		//i16		m_XMSPage;	// If in XMS, (xmsPage * PMPageSize) gives offset into XMS handle

		//u16		m_Locked;	// If set, this page can't be purged
		//i16		m_EMSPage;	// If in EMS, logical page/offset into page
		//i16		m_MainPage;	// If in Main, index into handle array

		//u32		m_LastHit;	// Last frame number of hit
	};

	BvVector<Page> m_Pages;
	BvVector<u32> m_Sprites;
	u16 m_SpriteStart;
	u16 m_SoundStart;
};


class WolfArchiveManager final
{
public:
	struct Chunk
	{
		const u8* m_pData;
		i32 m_CompressedSize;
		i32 m_DecompressedSize;
	};

	WolfArchiveManager() {}
	~WolfArchiveManager() {}
	bool Load(const BvPath& path);

	WolfImage GetImage(i32 chunk);
	WolfImage GetFontImage(i32 chunk);
	const u8* GetSpriteData(i32 spriteIndex) const { return (u8*)(m_VSwap.m_Sprites.Data() + 64 * 64 * spriteIndex); }

private:
	struct HuffNode
	{
		u16 m_Bit0;
		u16 m_Bit1;
	};

	bool DecodeHuffman(i32 chunk, BvVector<u8>& output);
	bool DecodeHuffman(const Chunk& chunk, u8* pOutput, i32 outputSize);

private:
	BvVector<i32> m_Offsets;
	BvVector<u8> m_VGAGraph;
	BvVector<Chunk> m_Chunks;
	BvVector<u8> m_VGADict;
	BvVector<HuffNode> m_HuffmanDict;
	BvVector<PicTableType> m_Pictures;
	VSwap m_VSwap;
	BvVector<u8> m_ScratchMemory;
};