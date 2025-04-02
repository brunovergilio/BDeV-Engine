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


BvRCRaw<IBvTextureBlob> BvDDSTextureLoader::LoadTextureFromFile(const char* pFilename, IBvTextureLoader::Result* pResult)
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


BvRCRaw<IBvTextureBlob> BvDDSTextureLoader::LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureLoader::Result* pResult)
{
	BvVector<u8> buffer(bufferSize);
	memcpy(buffer.Data(), pBuffer, bufferSize);

	return LoadTextureInternal(buffer, pResult);
}


BvTextureBlob* BvDDSTextureLoader::LoadTextureInternal(BvVector<u8>& buffer, IBvTextureLoader::Result* pResult)
{
	BvVector<SubresourceData> subresources;
	IBvTextureBlob::Info textureInfo;

	auto result = LoadDDSTexture(buffer.Data(), buffer.Size(), textureInfo, subresources);
	if (pResult)
	{
		*pResult = result;
	}

	return result == IBvTextureLoader::Result::kOk ? BV_NEW(BvTextureBlob)(buffer, textureInfo, subresources) : nullptr;
}


void BvDDSTextureLoader::SelfDestroy()
{
	BV_DELETE(this);
}


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvTextureLoader* CreateDDSTextureLoader()
		{
			return BV_NEW(BvDDSTextureLoader)();
		}
	}
}