#include "BvTextureLoader.h"
#include "DDS/BvDDS.h"
#include "BDeV/Core/System/File/BvFile.h"
#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


#define STB_IMAGE_IMPLEMENTATION
#include "BvRenderTools/TextureLoader/STB/stb_image.h"


class BvTextureBlob final : public IBvTextureBlob
{
	BV_NOCOPYMOVE(BvTextureBlob);

public:
	BvTextureBlob(BvVector<u8>& buffer, const IBvTextureBlob::Info& info, BvVector<SubresourceData>& subresources)
		: m_Buffer(std::move(buffer)), m_Info(info), m_Subresources(std::move(subresources))
	{
	}
	~BvTextureBlob() {}

	BV_INLINE const IBvTextureBlob::Info& GetInfo() override { return m_Info; }
	BV_INLINE const BvVector<SubresourceData>& GetSubresources() const override { return m_Subresources; }

private:
	BvVector<u8> m_Buffer;
	IBvTextureBlob::Info m_Info;
	BvVector<SubresourceData> m_Subresources;
};


class BvTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvTextureLoader);

public:
	BvTextureLoader();
	~BvTextureLoader();

	LoaderResult LoadTextureFromFile(const char* pFilename, IBvTextureBlob*& pTextureBlob) override;
	LoaderResult LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob*& pTextureBlob) override;
	void DestroyTexture(IBvTextureBlob*& pTextureBlob) override;

private:
	LoaderResult LoadTextureInternal(BvVector<u8>& buffer, IBvTextureBlob*& pTextureBlob);
	//void GenerateMips(const IBvTextureBlob::Info& textureInfo, const BvVector<u8>& inputBuffer, BvVector<u8>& outputBuffer, BvVector<SubresourceData>& subresources);

private:
	BvRobinSet<BvTextureBlob*> m_Textures;
};


BvTextureLoader::BvTextureLoader()
{
}


BvTextureLoader::~BvTextureLoader()
{
	for (auto pBlob : m_Textures)
	{
		delete pBlob;
	}
}


LoaderResult BvTextureLoader::LoadTextureFromFile(const char* pFilename, IBvTextureBlob*& pTextureBlob)
{
	BvFile file(pFilename);
	if (!file.IsValid())
	{
		return LoaderResult::kInvalidArg;
	}

	u64 bufferSize = file.GetSize();
	BvVector<u8> buffer(bufferSize);
	file.Read(buffer.Data(), (u32)bufferSize);
	file.Close();

	return LoadTextureInternal(buffer, pTextureBlob);
}


LoaderResult BvTextureLoader::LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob*& pTextureBlob)
{
	BvVector<u8> buffer(bufferSize);
	memcpy(buffer.Data(), pBuffer, bufferSize);

	return LoadTextureInternal(buffer, pTextureBlob);
}


void BvTextureLoader::DestroyTexture(IBvTextureBlob*& pTextureBlob)
{
	auto pPtr = static_cast<BvTextureBlob*>(pTextureBlob);
	delete pPtr;

	pTextureBlob = nullptr;
}


LoaderResult BvTextureLoader::LoadTextureInternal(BvVector<u8>& buffer, IBvTextureBlob*& pTextureBlob)
{
	pTextureBlob = nullptr;
	BvVector<SubresourceData> subresources;
	IBvTextureBlob::Info textureInfo;

	auto result = LoadDDSTexture(buffer.Data(), buffer.Size(), textureInfo, subresources);
	if (result == LoaderResult::kOk)
	{
		auto pBlob = new BvTextureBlob(buffer, textureInfo, subresources);
		m_Textures.Emplace(pBlob);
		pTextureBlob = pBlob;

		return result;
	}

	if (stbi_is_16_bit_from_memory(buffer.Data(), buffer.Size())
		|| stbi_is_hdr_from_memory(buffer.Data(), buffer.Size()))
	{
		return LoaderResult::kNotSupported;
	}

	{
		i32 x, y, comp;
		auto* pData = stbi_load_from_memory(buffer.Data(), buffer.Size(), &x, &y, &comp, 4);
		if (!pData)
		{
			return LoaderResult::kInvalidData;
		}

		textureInfo.m_TextureType = TextureType::kTexture2D;
		textureInfo.m_Format = Format::kRGBA8_UNorm_SRGB;
		textureInfo.m_Width = x;
		textureInfo.m_Height = y;
		textureInfo.m_Depth = 1;
		textureInfo.m_MipLevels = 1;
		textureInfo.m_LayerCount = 1;
		textureInfo.m_PlaneCount = 1;
		textureInfo.m_IsCubeMap = false;

		auto& subresource = subresources.EmplaceBack();
		subresource.m_pData = pData;
		subresource.m_RowPitch = 4 * x;
		subresource.m_SlicePitch = subresource.m_RowPitch * y;

		stbi_image_free(pData);
		
		auto pBlob = new BvTextureBlob(buffer, textureInfo, subresources);
		m_Textures.Emplace(pBlob);
		pTextureBlob = pBlob;

		return LoaderResult::kOk;
	}

	return LoaderResult::kInvalidData;
}


//void GetPixels(u8* pSrcMip, u32 x, u32 y, u32 rowPitch, u32 components, BvVec (c[4]))
//{
//	u8* pPixel00 = &pSrcMip[y * rowPitch + x];
//	u8* pPixel01 = &pSrcMip[y * rowPitch + (x + components)];
//	u8* pPixel10 = &pSrcMip[(y + 1) * rowPitch + x];
//	u8* pPixel11 = &pSrcMip[(y + 1) * rowPitch + (x + components)];
//
//	c[0].Set(pPixel00[0], pPixel00[1], pPixel00[2], pPixel00[3]);
//	c[1].Set(pPixel01[0], pPixel01[1], pPixel01[2], pPixel01[3]);
//	c[2].Set(pPixel10[0], pPixel10[1], pPixel10[2], pPixel10[3]);
//	c[3].Set(pPixel11[0], pPixel11[1], pPixel11[2], pPixel11[3]);
//}
//
//
//void ApplyBilinearFilter(u32 width, u32 height, u8* pSrcMip, u8* pDstMip, u32 components)
//{
//	u32 rowPitch = width * components;
//	u32 nwdith = width >> 1, nheight = height >> 1;
//	for (auto h = 0u; h < nheight; ++h)
//	{
//		u32 y = h << 1;
//		for (auto w = 0u; w < nwdith; ++w)
//		{
//			u32 x = (w << 1) * components;
//
//			BvVec c[4];
//			GetPixels(pSrcMip, x, y, rowPitch, components, c);
//
//			BvVec res = 0.25f * (c[0] + c[1] + c[2] + c[3]);
//			i32 mul = 1;
//			if (width & 1)
//			{
//				BvVec cx[4];
//				GetPixels(pSrcMip, x + components, y, rowPitch, components, cx);
//				res += 0.25f * (cx[0] + cx[1] + cx[2] + cx[3]);
//				mul <<= 1;
//			}
//			if (height & 1)
//			{
//				BvVec cy[4];
//				GetPixels(pSrcMip, x, y + 1, rowPitch, components, cy);
//				res += 0.25f * (cy[0] + cy[1] + cy[2] + cy[3]);
//				mul <<= 1;
//			}
//			if (mul == 4)
//			{
//				BvVec cxy[4];
//				GetPixels(pSrcMip, x + components, y + 1, rowPitch, components, cxy);
//				res += 0.25f * (cxy[0] + cxy[1] + cxy[2] + cxy[3]);
//			}
//
//			if (mul != 1)
//			{
//				res *= 1.0f / float(mul);
//			}
//
//			u32 dstRowPitch = nwdith * components;
//			pDstMip[h * dstRowPitch + w + 0] = u8(res[0]);
//			pDstMip[h * dstRowPitch + w + 1] = u8(res[1]);
//			pDstMip[h * dstRowPitch + w + 2] = u8(res[2]);
//			pDstMip[h * dstRowPitch + w + 3] = u8(res[3]);
//		}
//	}
//}
//
//
//void BvTextureLoader::GenerateMips(const IBvTextureBlob::Info& textureInfo, const BvVector<u8>& inputBuffer, BvVector<u8>& outputBuffer, BvVector<SubresourceData>& subresources)
//{
//	TextureDesc desc;
//	desc.m_Size.width = textureInfo.m_Width;
//	desc.m_Size.height = textureInfo.m_Height;
//	desc.m_Size.depth = textureInfo.m_Depth;
//	desc.m_LayerCount = textureInfo.m_LayerCount;
//	desc.m_MipLevels = GetMipCount(textureInfo.m_Width, textureInfo.m_Height, textureInfo.m_Depth);
//
//	u64 totalSize = GetBufferSizeForTextureData(desc, 4);
//	outputBuffer.Resize(totalSize);
//
//	memcpy(&outputBuffer[0], subresources[0].m_pData, subresources[0].m_SlicePitch);
//	u32 width = 0u, height = 0u;
//	u64 currOffset = 0;
//	for (auto i = 1u; i < desc.m_MipLevels; ++i)
//	{
//		width = std::max(textureInfo.m_Width >> (i - 1), 1u);
//		height = std::max(textureInfo.m_Height >> (i - 1), 1u);
//		u64 nextOffset = GetTextureDataOffset(desc, i, 0, 4);
//
//		ApplyBilinearFilter(width, height, &outputBuffer[currOffset], &outputBuffer[nextOffset], GetFormatInfo(textureInfo.m_Format).m_ElementCount);
//
//		currOffset = nextOffset;
//	}
//}


namespace BvRenderTools
{
	IBvTextureLoader* GetTextureLoader()
	{
		static BvTextureLoader textureLoader;
		return &textureLoader;
	}
}