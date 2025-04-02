#include "BvSTBTextureLoader.h"
#include "BDeV/Core/System/File/BvFile.h"
#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "BvRenderTools/TextureLoader/STB/stb_image.h"




BvSTBTextureLoader::BvSTBTextureLoader()
{
}


BvSTBTextureLoader::~BvSTBTextureLoader()
{
}


BvRCRaw<IBvTextureBlob> BvSTBTextureLoader::LoadTextureFromFile(const char* pFilename, IBvTextureLoader::Result* pResult)
{
	BvFile file(pFilename, BvFileAccessMode::kRead, BvFileAction::kOpen);
	if (!file.IsValid())
	{
		if (pResult)
		{
			*pResult = IBvTextureLoader::Result::kInvalidArg;
		}

		return nullptr;
	}

	u64 bufferSize = file.GetSize();
	BvVector<u8> buffer(bufferSize);
	file.Read(buffer.Data(), (u32)bufferSize);
	file.Close();

	return LoadTextureInternal(buffer, pResult);
}


BvRCRaw<IBvTextureBlob> BvSTBTextureLoader::LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureLoader::Result* pResult)
{
	BvVector<u8> buffer(bufferSize);
	memcpy(buffer.Data(), pBuffer, bufferSize);

	return LoadTextureInternal(buffer, pResult);
}


BvTextureBlob* BvSTBTextureLoader::LoadTextureInternal(BvVector<u8>& buffer, IBvTextureLoader::Result* pResult)
{
	BvVector<SubresourceData> subresources;
	IBvTextureBlob::Info textureInfo;

	i32 x, y, comp, desiredComp = 0;
	u32 compSize = 0;
	u8* pData = nullptr;
	Format format = Format::kUnknown;
	if (!stbi_info_from_memory(buffer.Data(), i32(buffer.Size()), &x, &y, &comp))
	{
		if (pResult)
		{
			*pResult = IBvTextureLoader::Result::kNotSupported;
		}
		return nullptr;
	}

	if (stbi_is_16_bit_from_memory(buffer.Data(), buffer.Size()))
	{
		// We don't support 3-component formats with 16-bit values each, so force-change it to 4
		if (comp == 3)
		{
			desiredComp = 4;
		}

		pData = reinterpret_cast<u8*>(stbi_load_16_from_memory(buffer.Data(), buffer.Size(), &x, &y, &comp, desiredComp));
		switch (comp)
		{
		case 4: format = Format::kRGBA16_UNorm; break;
		case 2: format = Format::kRG16_UNorm; break;
		case 1: format = Format::kR16_UNorm; break;
		}
		compSize = 2;
	}
	else if (stbi_is_hdr_from_memory(buffer.Data(), buffer.Size()))
	{
		pData = reinterpret_cast<u8*>(stbi_loadf_from_memory(buffer.Data(), buffer.Size(), &x, &y, &comp, desiredComp));
		switch (comp)
		{
		case 4: format = Format::kRGBA32_Float; break;
		case 3: format = Format::kRGB32_Float; break;
		case 2: format = Format::kRG32_Float; break;
		case 1: format = Format::kR32_Float; break;
		}
		compSize = 4;
	}
	else
	{
		// We don't support 3-component formats with 8-bit values each, so force-change it to 4
		if (comp == 3)
		{
			desiredComp = 4;
		}

		pData = stbi_load_from_memory(buffer.Data(), buffer.Size(), &x, &y, &comp, desiredComp);
		switch (comp)
		{
		case 4: format = Format::kRGBA8_UNorm; break;
		case 2: format = Format::kRG8_UNorm; break;
		case 1: format = Format::kR8_UNorm; break;
		}
		compSize = 1;
	}

	if (!pData)
	{
		if (pResult)
		{
			*pResult = IBvTextureLoader::Result::kInvalidData;
		}
		return nullptr;
	}
	else if (format == Format::kUnknown)
	{
		stbi_image_free(pData);
		if (pResult)
		{
			*pResult = IBvTextureLoader::Result::kNotSupported;
		}
		return nullptr;
	}

	textureInfo.m_TextureType = TextureType::kTexture2D;
	textureInfo.m_Format = format;
	textureInfo.m_Width = x;
	textureInfo.m_Height = y;
	textureInfo.m_Depth = 1;
	textureInfo.m_MipLevels = 1;
	textureInfo.m_ArraySize = 1;
	textureInfo.m_PlaneCount = 1;
	textureInfo.m_IsCubeMap = false;

	auto& subresource = subresources.EmplaceBack();
	subresource.m_pData = pData;
	subresource.m_RowPitch = u32(comp * x * compSize);
	subresource.m_SlicePitch = subresource.m_RowPitch * u32(y);

	buffer.Clear();

	buffer.Resize(subresource.m_SlicePitch);
	memcpy(buffer.Data(), pData, buffer.Size());
	stbi_image_free(pData);

	if (pResult)
	{
		*pResult = IBvTextureLoader::Result::kOk;
	}
	return BV_NEW(BvTextureBlob)(buffer, textureInfo, subresources);
}


void BvSTBTextureLoader::SelfDestroy()
{
	BV_DELETE(this);
}


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvTextureLoader* CreateSTBTextureLoader()
		{
			return BV_NEW(BvSTBTextureLoader)();
		}
	}
}