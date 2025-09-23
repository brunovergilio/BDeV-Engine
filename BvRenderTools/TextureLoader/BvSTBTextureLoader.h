#pragma once


#include "BvTextureLoaderCommon.h"
#include "BDeV/Core/Container/BvRobinSet.h"


BV_OBJECT_DEFINE_ID(BvSTBTextureLoader, "35f0de17-c560-4af4-8cf8-f5b70189e2ee");
class BvSTBTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvSTBTextureLoader);

public:
	BvSTBTextureLoader();
	~BvSTBTextureLoader();

	//BV_OBJECT_IMPL_INTERFACE(BvSTBTextureLoader, IBvTextureLoader);

private:
	IBvTextureLoader::Result LoadTextureFromFile(const char* pFilename, const BvUUID& objId, void** ppObj) override;
	IBvTextureLoader::Result LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, const BvUUID& objId, void** ppObj) override;

	IBvTextureLoader::Result LoadTextureInternal(BvVector<u8>& buffer, BvTextureBlob*& pTextureBlob);
	void SelfDestroy() override;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvSTBTextureLoader);


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateSTBTextureLoader(const BvUUID& objId, void** ppObj);
	}
}