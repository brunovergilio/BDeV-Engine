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


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureFromFile(const char* pFilename, const BvUUID& objId, void** ppObj)
{
	if (!ppObj || (objId != BV_OBJECT_ID(BvTextureBlob) && objId != BV_OBJECT_ID(IBvTextureBlob)))
	{
		return IBvTextureLoader::Result::kBadPointer;
	}

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


IBvTextureLoader::Result BvDDSTextureLoader::LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, const BvUUID& objId, void** ppObj)
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
		pTextureBlob = BV_NEW(BvTextureBlob)(buffer, textureInfo, subresources);
	}

	return result;
}


void BvDDSTextureLoader::SelfDestroy()
{
	BV_DELETE(this);
}


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateDDSTextureLoader(const BvUUID& objId, void** ppObj)
		{
			if (!ppObj || (objId != BV_OBJECT_ID(BvDDSTextureLoader) && objId != BV_OBJECT_ID(IBvTextureLoader)))
			{
				return false;
			}

			*ppObj = BV_NEW(BvDDSTextureLoader)();
			return true;
		}
	}
}