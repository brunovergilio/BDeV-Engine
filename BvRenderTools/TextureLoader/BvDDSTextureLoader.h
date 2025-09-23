#pragma once


#include "BvTextureLoaderCommon.h"
#include "BDeV/Core/Container/BvRobinSet.h"


BV_OBJECT_DEFINE_ID(BvDDSTextureLoader, "9f89fb27-ac2c-498b-8d42-4e62edde790d");
class BvDDSTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvDDSTextureLoader);

public:
	BvDDSTextureLoader();
	~BvDDSTextureLoader();

	//BV_OBJECT_IMPL_INTERFACE(BvDDSTextureLoader, IBvTextureLoader);

private:
	IBvTextureLoader::Result LoadTextureFromFile(const char* pFilename, const BvUUID& objId, void** ppObj) override;
	IBvTextureLoader::Result LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, const BvUUID& objId, void** ppObj) override;

	IBvTextureLoader::Result LoadTextureInternal(BvVector<u8>& buffer, BvTextureBlob*& pTextureBlob);
	void SelfDestroy() override;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvDDSTextureLoader);


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateDDSTextureLoader(const BvUUID& objId, void** ppObj);
	}
}