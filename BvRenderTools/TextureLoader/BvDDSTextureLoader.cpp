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
}


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureFromFileImpl(const char* pFilename, void** ppObj)
{
	BvFile file(pFilename, BvFileAccessMode::kRead, BvFileAction::kOpen);
	if (!file.IsValid())
	{
		return IBvTextureLoader::Result::kInvalidArg;
	}

	u64 bufferSize = file.GetSize();
	BvVector<u8> buffer(bufferSize);
	file.Read(buffer.Data(), (u32)bufferSize);
	file.Close();

	BvTextureBlob* pBlob;
	auto result = LoadTextureInternal(buffer, pBlob);
	if (result == IBvTextureLoader::Result::kOk)
	{
		*ppObj = pBlob;
	}

	return result;
}


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureFromMemoryImpl(const void* pBuffer, u64 bufferSize, void** ppObj)
{
	if (!ppObj)
	{
		return IBvTextureLoader::Result::kBadPointer;
	}

	BvVector<u8> buffer(bufferSize);
	memcpy(buffer.Data(), pBuffer, bufferSize);

	BvTextureBlob* pBlob;
	auto result = LoadTextureInternal(buffer, pBlob);
	if (result == IBvTextureLoader::Result::kOk)
	{
		*ppObj = pBlob;
	}

	return result;
}


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureInternal(BvVector<u8>& buffer, BvTextureBlob*& pTextureBlob)
{
	BvVector<SubresourceData> subresources;
	IBvTextureBlob::Info textureInfo;

	auto result = LoadDDSTexture(buffer.Data(), buffer.Size(), textureInfo, subresources);
	if (result == IBvTextureLoader::Result::kOk)
	{
		pTextureBlob = BV_RC_CREATE(BvTextureBlob, buffer, textureInfo, subresources);
	}

	return result;
}


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateDDSTextureLoader(void** ppObj)
		{
			*ppObj = BV_RC_CREATE(BvDDSTextureLoader);
			return true;
		}
	}
}