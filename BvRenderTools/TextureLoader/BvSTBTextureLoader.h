#pragma once


#include "BvTextureLoaderCommon.h"
#include "BDeV/Core/Container/BvRobinSet.h"


//BV_OBJECT_DEFINE_ID(BvSTBTextureLoader, "35f0de17-c560-4af4-8cf8-f5b70189e2ee");
//BV_OBJECT_ENABLE_ID_OPERATOR(BvSTBTextureLoader);
class BvSTBTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvSTBTextureLoader);

public:
	BvSTBTextureLoader();
	~BvSTBTextureLoader();

	BvRCRaw<IBvTextureBlob> LoadTextureFromFile(const char* pFilename, IBvTextureLoader::Result* pResult = nullptr) override;
	BvRCRaw<IBvTextureBlob> LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureLoader::Result* pResult = nullptr) override;

	//BV_OBJECT_IMPL_INTERFACE(BvSTBTextureLoader, IBvTextureLoader);

private:
	BvTextureBlob* LoadTextureInternal(BvVector<u8>& buffer, IBvTextureLoader::Result* pResult);
	//void GenerateMips(const IBvTextureBlob::Info& textureInfo, const BvVector<u8>& inputBuffer, BvVector<u8>& outputBuffer, BvVector<SubresourceData>& subresources);
	void SelfDestroy() override;
};


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvTextureLoader* CreateSTBTextureLoader();
	}
}