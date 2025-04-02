#pragma once


#include "BvTextureLoaderCommon.h"
#include "BDeV/Core/Container/BvRobinSet.h"


//BV_OBJECT_DEFINE_ID(BvDDSTextureLoader, "9f89fb27-ac2c-498b-8d42-4e62edde790d");
//BV_OBJECT_ENABLE_ID_OPERATOR(BvDDSTextureLoader);
class BvDDSTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvDDSTextureLoader);

public:
	BvDDSTextureLoader();
	~BvDDSTextureLoader();

	BvRCRaw<IBvTextureBlob> LoadTextureFromFile(const char* pFilename, IBvTextureLoader::Result* pResult = nullptr) override;
	BvRCRaw<IBvTextureBlob> LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureLoader::Result* pResult = nullptr) override;

	//BV_OBJECT_IMPL_INTERFACE(BvDDSTextureLoader, IBvTextureLoader);

private:
	BvTextureBlob* LoadTextureInternal(BvVector<u8>& buffer, IBvTextureLoader::Result* pResult);
	//void GenerateMips(const IBvTextureBlob::Info& textureInfo, const BvVector<u8>& inputBuffer, BvVector<u8>& outputBuffer, BvVector<SubresourceData>& subresources);
	void SelfDestroy() override;
};


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvTextureLoader* CreateDDSTextureLoader();
	}
}