#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/RenderAPI/BvTextureLoader.h"


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

private:
	BvVector<u8> m_Buffer;
	IBvTextureBlob::Info m_Info;
	BvVector<SubresourceData> m_Subresources;
};