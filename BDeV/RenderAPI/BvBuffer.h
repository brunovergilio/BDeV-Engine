#pragma once


#include "BvCommon.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvEnum.h"


enum class BufferUsage : u16
{
	kUndefined			= 0,
	kTransferSrc		= BvBit(0),
	kTransferDst		= BvBit(1),
	kUniformBuffer		= BvBit(2),
	kStorageBuffer		= BvBit(3),
	kUniformTexelBuffer = BvBit(4),
	kStorageTexelBuffer = BvBit(5),
	kIndexBuffer		= BvBit(6),
	kVertexBuffer		= BvBit(7),
	kIndirectBuffer		= BvBit(8),
};
BV_USE_ENUM_CLASS_OPERATORS(BufferUsage);


struct BufferDesc
{
	u64 m_Size = 0;
	u32 m_Alignment = 0;
	BufferUsage m_UsageFlags = BufferUsage::kUndefined;
	MemoryFlags m_MemoryFlags = MemoryFlags::kDeviceLocal;
};


class BvBuffer
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