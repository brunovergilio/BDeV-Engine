#pragma once


#include "BvBuffer.h"
#include "BDeV/Utils/BvUtils.h"


struct BufferViewDesc
{
	BvBuffer * m_pBuffer = nullptr;
	u64 m_Offset = 0;
	u64 m_ElementCount = 0;
	u64 m_Stride = 0;
	Format m_Format = Format::kUnknown;
};


class BvBufferView
{
	BV_NOCOPYMOVE(BvBufferView);

public:
	BV_INLINE const BufferViewDesc & GetDesc() const { return m_BufferViewDesc; }
	BV_INLINE BvBuffer * GetBuffer() const { return m_BufferViewDesc.m_pBuffer; }

protected:
	BvBufferView(const BufferViewDesc & bufferViewDesc)
		: m_BufferViewDesc(bufferViewDesc) {}
	virtual ~BvBufferView() = 0 {}

protected:
	BufferViewDesc m_BufferViewDesc;
};