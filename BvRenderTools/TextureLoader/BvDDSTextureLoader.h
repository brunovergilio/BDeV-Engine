#pragma once


#include "BvTextureLoaderCommon.h"
#include "BDeV/Core/Container/BvRobinSet.h"


class BvDDSTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvDDSTextureLoader);

public:
	BvDDSTextureLoader();
	~BvDDSTextureLoader();

private:
	IBvTextureLoader::Result LoadTextureFromFileImpl(const char* pFilename, void** ppObj) override;
	IBvTextureLoader::Result LoadTextureFromMemoryImpl(const void* pBuffer, u64 bufferSize, void** ppObj) override;

	IBvTextureLoader::Result LoadTextureInternal(BvVector<u8>& buffer, BvTextureBlob*& pTextureBlob);
};
BV_OBJECT_DEFINE_ID(BvDDSTextureLoader, "9f89fb27-ac2c-498b-8d42-4e62edde790d");


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateDDSTextureLoader(void** ppObj);
	}
}