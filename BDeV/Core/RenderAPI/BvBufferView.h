#pragma once


#include "BvBuffer.h"
#include "BDeV/Core/Utils/BvUtils.h"


BV_OBJECT_DEFINE_ID(BvBufferView, "22f4c138-f83b-4f9b-aef3-61dbb1724ae9");
class BvBufferView : public IBvRenderDeviceObject
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
BV_OBJECT_ENABLE_ID_OPERATOR(BvBufferView);