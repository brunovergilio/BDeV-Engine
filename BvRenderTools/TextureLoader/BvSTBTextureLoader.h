#pragma once


#include "BvTextureLoaderCommon.h"
#include "BDeV/Core/Container/BvRobinSet.h"


class BvSTBTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvSTBTextureLoader);

public:
	BvSTBTextureLoader();
	~BvSTBTextureLoader();

private:
	IBvTextureLoader::Result LoadTextureFromFileImpl(const char* pFilename, void** ppObj) override;
	IBvTextureLoader::Result LoadTextureFromMemoryImpl(const void* pBuffer, u64 bufferSize, void** ppObj) override;

	IBvTextureLoader::Result LoadTextureInternal(BvVector<u8>& buffer, BvTextureBlob*& pTextureBlob);
};
BV_OBJECT_DEFINE_ID(BvSTBTextureLoader, "35f0de17-c560-4af4-8cf8-f5b70189e2ee");


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateSTBTextureLoader(void** ppObj);
	}
}