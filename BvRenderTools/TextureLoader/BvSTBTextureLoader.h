#pragma once


#include "BvTextureLoaderCommon.h"
#include "BDeV/Core/Container/BvRobinSet.h"


class BvSTBTextureLoader final : public IBvTextureLoader
{
	BV_NOCOPYMOVE(BvSTBTextureLoader);

public:
	BvSTBTextureLoader();
	~BvSTBTextureLoader();

	IBvTextureLoader::Result LoadTextureFromFile(const char* pFilename, IBvTextureBlob** ppTextureBlob) override;
	IBvTextureLoader::Result LoadTextureFromMemory(const void* pBuffer, u64 bufferSize, IBvTextureBlob** ppTextureBlob) override;
	void DestroyTexture(IBvTextureBlob** ppTextureBlob) override;

private:
	IBvTextureLoader::Result LoadTextureInternal(BvVector<u8>& buffer, IBvTextureBlob** ppTextureBlob);
	//void GenerateMips(const IBvTextureBlob::Info& textureInfo, const BvVector<u8>& inputBuffer, BvVector<u8>& outputBuffer, BvVector<SubresourceData>& subresources);

private:
	BvRobinSet<BvTextureBlob*> m_Textures;
};


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvTextureLoader* GetSTBTextureLoader();
	}
}