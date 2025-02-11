#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvTextureLoader.h"


BV_OBJECT_DEFINE_ID(BvTextureBlob, "325881a9-ae9c-4b09-8173-2a2f3d911d9b");
class BvTextureBlob final : public IBvTextureBlob
{
	BV_NOCOPYMOVE(BvTextureBlob);

public:
	BvTextureBlob(BvVector<u8>& buffer, const IBvTextureBlob::Info& info, BvVector<SubresourceData>& subresources)
		: m_Buffer(std::move(buffer)), m_Info(info), m_Subresources(std::move(subresources))
	{
	}
	~BvTextureBlob() {}

	BV_INLINE const IBvTextureBlob::Info& GetInfo() override { return m_Info; }
	BV_INLINE const BvVector<SubresourceData>& GetSubresources() const override { return m_Subresources; }

	BV_OBJECT_IMPL_INTERFACE(BvTextureBlob, IBvTextureBlob);

private:
	BvVector<u8> m_Buffer;
	IBvTextureBlob::Info m_Info;
	BvVector<SubresourceData> m_Subresources;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvTextureBlob);