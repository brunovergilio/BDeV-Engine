#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvBuffer : public BvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvBuffer);

public:
	virtual void * const Map(const u64 size = 0, const u64 offset = 0) = 0;
	virtual void Unmap() = 0;
	virtual void Flush(const u64 size = 0, const u64 offset = 0) const = 0;
	virtual void Invalidate(const u64 size = 0, const u64 offset = 0) const = 0;

	BV_INLINE const BufferDesc & GetDesc() const { return m_BufferDesc; }
	BV_INLINE void* GetMappedData() const { return m_pMapped; }
	template<typename Type>
	BV_INLINE Type* GetMappedDataAsT() const { return reinterpret_cast<Type*>(m_pMapped); }

protected:
	BvBuffer(const BufferDesc & bufferDesc)
		: m_BufferDesc(bufferDesc) {}
	virtual ~BvBuffer() = 0 {}

protected:
	BufferDesc m_BufferDesc;
	void* m_pMapped = nullptr;
};