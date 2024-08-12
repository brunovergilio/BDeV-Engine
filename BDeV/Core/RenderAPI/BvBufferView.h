#pragma once


#include "BvBuffer.h"
#include "BDeV/Core/Utils/BvUtils.h"


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