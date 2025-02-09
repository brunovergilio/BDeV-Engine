#include "BvDDSTextureLoader.h"
#include "BDeV/Core/System/File/BvFile.h"
#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"
#include "DDS/BvDDS.h"


BvDDSTextureLoader::BvDDSTextureLoader()
{
}


BvDDSTextureLoader::~BvDDSTextureLoader()
{
	for (auto pBlob : m_Textures)
	{
		delete pBlob;
	}
}


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureFromFile(const char* pFilename, IBvTextureBlob** ppTextureBlob)
{
	BvFile file(pFilename);
	if (!file.IsValid())
	{
		return IBvTextureLoader::Result::kInvalidArg;
	}

	u64 bufferSize = file.GetSize();
	BvVector<u8> buffer(bufferSize);
	file.Read(buffer.Data(), (u32)bufferSize);
	file.Close();

	return LoadTextureInternal(buffer, ppTextureBlob);
}


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob** ppTextureBlob)
{
	BvVector<u8> buffer(bufferSize);
	memcpy(buffer.Data(), pBuffer, bufferSize);

	return LoadTextureInternal(buffer, ppTextureBlob);
}


void BvDDSTextureLoader::DestroyTexture(IBvTextureBlob** ppTextureBlob)
{
	auto pPtr = static_cast<BvTextureBlob*>(*ppTextureBlob);
	delete pPtr;

	*ppTextureBlob = nullptr;
}


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureInternal(BvVector<u8>& buffer, IBvTextureBlob** ppTextureBlob)
{
	ppTextureBlob = nullptr;
	BvVector<SubresourceData> subresources;
	IBvTextureBlob::Info textureInfo;

	auto result = LoadDDSTexture(buffer.Data(), buffer.Size(), textureInfo, subresources);
	if (result == IBvTextureLoader::Result::kOk)
	{
		auto pBlob = new BvTextureBlob(buffer, textureInfo, subresources);
		m_Textures.Emplace(pBlob);
		*ppTextureBlob = pBlob;
	}
	
	return result;
}


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvTextureLoader* GetDDSTextureLoader()
		{
			static BvDDSTextureLoader textureLoader;
			return &textureLoader;
		}
	}
}