#include "BvTextureLoader.h"
#include "BDeV/System/File/BvFile.h"
#include "BvRenderTools/TextureLoader/DDS/BvDDS.h"
#include "BDeV/Container/BvRobinSet.h"


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
	LoaderResult LoadTextureFromFile(const wchar_t* pFilename, IBvTextureBlob*& pTextureBlob) override;
	LoaderResult LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob*& pTextureBlob) override;
	void DestroyTexture(IBvTextureBlob*& pTextureBlob) override;

private:
	LoaderResult LoadTextureInternal(BvVector<u8>& buffer, IBvTextureBlob*& pTextureBlob);

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
	mbstate_t mbState{};
	auto len = 1 + mbsrtowcs(nullptr, &pFilename, 0, &mbState);
	BvVector<wchar_t> widePath(len);
	if (mbsrtowcs(widePath.Data(), &pFilename, widePath.Size(), &mbState) == (size_t)-1)
	{
		return LoaderResult::kInvalidArg;
	}

	return LoadTextureFromFile(widePath.Data(), pTextureBlob);
}


LoaderResult BvTextureLoader::LoadTextureFromFile(const wchar_t* pFilename, IBvTextureBlob*& pTextureBlob)
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


namespace BvRenderTools
{
	IBvTextureLoader* GetTextureLoader()
	{
		static BvTextureLoader textureLoader;
		return &textureLoader;
	}
}