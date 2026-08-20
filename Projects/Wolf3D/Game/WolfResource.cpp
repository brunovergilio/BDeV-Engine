#include "WolfResource.h"
#include "BDeV/Core/System/File/BvFile.h"


#define MAXEMPTYREAD 1024


// CAL_SetupGrFile() -> ID_CA.C
bool WolfArchiveManager::Load(const BvPath& path)
{
	auto fileList = path.GetFileList();

	for (const auto& filename : fileList)
	{
		if (filename.HasName("VGAHEAD"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
			auto fileSize = file.GetSize();
			m_Offsets.Reserve(NUMCHUNKS + 1);
			u8 sizeBytes[3];
			u32 bytesRead = 0;
			for (auto i = 0; i <= NUMCHUNKS && file.Read(sizeBytes, 3, &bytesRead) && bytesRead == 3; i++)
			{
				auto chunkOffset = i32(sizeBytes[0] | (sizeBytes[1] << 8) | (sizeBytes[2] << 16));
				m_Offsets.PushBack(chunkOffset == 0xFFFFFF ? -1 : chunkOffset);
			}
		}
		else if (filename.HasName("VGADICT"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
			auto fileSize = file.GetSize();
			m_HuffmanDict.Resize(255);
			file.Read(m_HuffmanDict.Data(), sizeof(HuffNode) * m_HuffmanDict.Size());

			// CAL_OptimizeNodes -> ID_CA.C
		}
		else if (filename.HasName("VGAGRAPH"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
			auto fileSize = file.GetSize();
			m_VGAGraph.Resize(fileSize);
			file.Read(m_VGAGraph.Data(), fileSize);
		}
		else if (filename.HasName("VSWAP"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
			
			u16 chunksInFile, spriteStart, soundStart;
			file.ReadT(chunksInFile);
			file.ReadT(spriteStart);
			file.ReadT(soundStart);

			m_VSwap.m_SpriteStart = spriteStart;
			m_VSwap.m_SoundStart = soundStart;
			m_VSwap.m_Pages.Resize(chunksInFile, {});

			u32 skipBytes = 0;
			{
				BvVector<u32> offsets(chunksInFile);
				file.Read(offsets.Data(), offsets.ByteSize());
				BvVector<u16> chunkLenghts(chunksInFile);
				file.Read(chunkLenghts.Data(), chunkLenghts.ByteSize());

				auto startOffset = offsets[0];
				skipBytes = u32(file.GetFilePos());
				BV_ASSERT(skipBytes <= startOffset, "Invalid VSWAP offset data");

				for (auto p = 0; p < m_VSwap.m_Pages.Size(); p++)
				{
					m_VSwap.m_Pages[p].m_Offset = offsets[p];
					m_VSwap.m_Pages[p].m_Length = chunkLenghts[p];
				}
			}

			auto fileSize = file.GetSize();
			file.GoToStart();
			BvVector<u8> buffer(fileSize);
			file.Read(buffer.Data(), buffer.ByteSize());

			m_VSwap.m_Sprites.Resize(kSpriteSize * m_VSwap.m_SoundStart, GetPixelFromPalette(13));
			i32 spriteIndex = 0;
			for (; spriteIndex < m_VSwap.m_SpriteStart; spriteIndex++)
			{
				auto& page = m_VSwap.m_Pages[spriteIndex];
				u8* pMem = &buffer[page.m_Offset];
				auto pSpriteData = m_VSwap.m_Sprites.Data() + kSpriteSize * spriteIndex;
				for (auto y = 0; y < 64; y++)
				{
					for (auto x = 0; x < 64; x++)
					{
						pSpriteData[y * 64 + x] = GetPixelFromPalette(pMem[x * 64 + y]);
					}
				}
			}
			for (; spriteIndex < m_VSwap.m_SoundStart; spriteIndex++)
			{
				auto& page = m_VSwap.m_Pages[spriteIndex];
				u8* pMem = &buffer[page.m_Offset];
				SpriteCompShape* pShape = (SpriteCompShape*)pMem;
				u32 offsetIndex = 0;
				auto pSpriteData = m_VSwap.m_Sprites.Data() + kSpriteSize * spriteIndex;
				for (auto y = pShape->m_LeftPixel; y <= pShape->m_RightPixel; y++, offsetIndex++)
				{
					auto pCmd = (u16*)(pMem + pShape->m_Offsets[offsetIndex]);
					auto pEndCmd = (u16*)(pMem + (offsetIndex == pShape->m_RightPixel ? page.m_Length : pShape->m_Offsets[offsetIndex + 1]));

					while (pCmd < pEndCmd && *pCmd)
					{
						auto endy = pCmd[0] >> 1;
						auto tableOffset = pCmd[1];
						auto starty = pCmd[2] >> 1;

						BV_ASSERT(tableOffset + starty >= sizeof(u16) * (pShape->m_RightPixel - pShape->m_LeftPixel + 3), "Invalid");

						for (auto x = starty; x < endy; x++)
						{
							pSpriteData[x * 64 + y] = GetPixelFromPalette(pMem[tableOffset + x]);
						}

						pCmd += 3;
					}
				}
			}
		}
		else if (filename.HasName("MAPHEAD"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
		}
		else if (filename.HasName("GAMEMAPS"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
		}
		else if (filename.HasName("AUDIOHED"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
		}
		else if (filename.HasName("AUDIOT"))
		{
			auto file = filename.AsFile(BvFileAccessMode::kRead);
		}
	}

	// CAL_ExpandGrChunk() -> ID_CA.C
	i32 compressed;
	i32 decompressed;
	u8* pData;
	m_Chunks.Resize(NUMCHUNKS);
	for (auto i = 0; i < NUMCHUNKS; i++)
	{
		auto pos = m_Offsets[i];
		pData = m_VGAGraph.Data() + pos;
		if (pos < 0)
		{
			m_Chunks[i] = {};
			continue;
		}

		auto next = i + 1;
		while (m_Offsets[next] == -1)
		{
			next++;
		}

		compressed = m_Offsets[next] - pos;

		if (i >= STARTTILE8 && i < STARTEXTERNS)
		{
			//
			// expanded sizes of tile8/16/32 are implicit
			//

#define BLOCK		64
#define MASKBLOCK	128

			if (i < STARTTILE8M)			// tile 8s are all in one chunk!
				decompressed = BLOCK * NUMTILE8;
			else if (i < STARTTILE16)
				decompressed = MASKBLOCK * NUMTILE8M;
			else if (i < STARTTILE16M)	// all other tiles are one/chunk
				decompressed = BLOCK * 4;
			else if (i < STARTTILE32)
				decompressed = MASKBLOCK * 4;
			else if (i < STARTTILE32M)
				decompressed = BLOCK * 16;
			else
				decompressed = MASKBLOCK * 16;
		}
		else
		{
			//
			// everything else has an explicit size longword
			//
			decompressed = (i32)(m_VGAGraph[pos] |
				(m_VGAGraph[pos + 1] << 8) |
				(m_VGAGraph[pos + 2] << 16) |
				(m_VGAGraph[pos + 3] << 24));
			pData += 4;			// skip over length
		}
#undef BLOCK
#undef MASKBLOCK

		m_Chunks[i] = { pData, compressed - 4, decompressed };
	}

	m_Pictures.Resize(NUMPICS);
	if (!DecodeHuffman(m_Chunks[STRUCTPIC], (u8*)m_Pictures.Data(), m_Pictures.Size() * sizeof(PicTableType)))
	{
		return false;
	}

	// Eventually will add a proper check
	return true;
}

WolfImage WolfArchiveManager::GetImage(i32 chunk)
{
	auto picNum = chunk - STARTPICS;

	WolfImage img{};

	if (picNum < 0 || picNum >= NUMPICS)
	{
		return img;
	}

	auto& pic = m_Pictures[picNum];
	m_ScratchMemory.Resize(pic.m_Width * pic.m_Height);
	if (!DecodeHuffman(chunk, m_ScratchMemory))
	{
		return img;
	}

	img.m_Width = pic.m_Width;
	img.m_Height = pic.m_Height;
	img.m_Pixels.Resize(pic.m_Width * pic.m_Height * 4);

	auto planeWidth = pic.m_Width >> 2;
	auto planeSize = planeWidth * pic.m_Height;
	u32* pColorBytes = reinterpret_cast<u32*>(img.m_Pixels.Data());
	for (i32 y = 0; y < pic.m_Height; ++y)
	{
		for (i32 x = 0; x < pic.m_Width; ++x)
		{
			i32 plane = x & 3;
			i32 planeX = x >> 2;

			u8 paletteIndex = m_ScratchMemory[plane * planeSize + y * planeWidth + planeX];

			pColorBytes[y * pic.m_Width + x] = GetPixelFromPalette(paletteIndex);
		}
	}

	return img;
}


WolfImage WolfArchiveManager::GetFontImage(i32 chunk)
{
	WolfImage img{};
	if (chunk < STARTFONT || chunk >= STARTFONTM)
	{
		return img;
	}

	auto& fontChunk = m_Chunks[chunk];
	m_ScratchMemory.Resize(fontChunk.m_DecompressedSize);
	if (!DecodeHuffman(fontChunk, m_ScratchMemory.Data(), m_ScratchMemory.Size()))
	{
		return img;
	}

	auto pFont = (FontStruct*)m_ScratchMemory.Data();

	img.m_Height = pFont->m_Height;
	for (auto c = 0; c < 256; c++)
	{
		//BV_ASSERT(pFont->m_Width[c] >= 0, "Error");
		img.m_Width += pFont->m_Width[c];
	}
	img.m_Pixels.Resize(img.m_Width * img.m_Height * 4);

	u32* pColorBytes = reinterpret_cast<u32*>(img.m_Pixels.Data());
	u8* pSrcData = m_ScratchMemory.Data();

	i32 xOffset = 0;

	for (i32 ch = 0; ch < 256; ++ch)
	{
		i32 glyphWidth = pFont->m_Width[ch];

		for (i32 y = 0; y < pFont->m_Height; ++y)
		{
			for (i32 x = 0; x < glyphWidth; ++x)
			{
				auto offset = pFont->m_Location[ch] + y * glyphWidth + x;
				u32 value = pSrcData[offset];

				pColorBytes[y * img.m_Width + xOffset + x] = (value != 0) * kU32Max;
			}
		}

		xOffset += glyphWidth;
	}

	return img;
}

bool WolfArchiveManager::DecodeHuffman(i32 chunkIndex, BvVector<u8>& output)
{
	const auto& chunk = m_Chunks[chunkIndex];
	if (!chunk.m_pData)
	{
		return false;
	}

	output.Resize(chunk.m_DecompressedSize);

	return DecodeHuffman(chunk, output.Data(), output.Size());
}


bool WolfArchiveManager::DecodeHuffman(const Chunk& chunk, u8* pOutput, i32 outputSize)
{
	if (!chunk.m_pData || chunk.m_DecompressedSize < outputSize)
	{
		return false;
	}

	auto pSrcData = chunk.m_pData;
	auto pEndSrcData = chunk.m_pData + chunk.m_CompressedSize;
	auto pDstData = pOutput;

	HuffNode* pHeadNode = m_HuffmanDict.Data() + (m_HuffmanDict.Size() - 1);
	HuffNode* pNode = pHeadNode;
	i32 expanded = 0;
	while (expanded < outputSize)
	{
		BV_ASSERT(pSrcData < pEndSrcData, "Huffman stream ended prematurely");

		u8 byte = *pSrcData++;

		for (auto i = 0; i < 8 && expanded < outputSize; i++)
		{
			auto value = (byte & (1 << i)) ? pNode->m_Bit1 : pNode->m_Bit0;

			if (value >= 256)
			{
				pNode = m_HuffmanDict.Data() + (value - 256);
			}
			else
			{
				*pDstData++ = u8(value);
				expanded++;

				pNode = pHeadNode;
			}
		}
	}

	BV_ASSERT(expanded == outputSize, "Sizes don't match - expected [%d], got [%d]", outputSize, expanded);

	return expanded == outputSize;
}