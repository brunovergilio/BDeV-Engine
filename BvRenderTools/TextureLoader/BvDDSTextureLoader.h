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

	IBvTextureLoader::Result LoadTextureFromFile(const char* pFilename, IBvTextureBlob** ppTextureBlob) override;
	IBvTextureLoader::Result LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob** ppTextureBlob) override;
	void DestroyTexture(IBvTextureBlob** ppTextureBlob) override;

	BV_OBJECT_IMPL_INTERFACE(BvDDSTextureLoader, IBvTextureLoader);

private:
	IBvTextureLoader::Result LoadTextureInternal(BvVector<u8>& buffer, IBvTextureBlob** ppTextureBlob);
	//void GenerateMips(const IBvTextureBlob::Info& textureInfo, const BvVector<u8>& inputBuffer, BvVector<u8>& outputBuffer, BvVector<SubresourceData>& subresources);
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvDDSTextureLoader);


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateDDSTextureLoader(IBvTextureLoader** ppObj);
	}
}