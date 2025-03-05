#pragma once


#include "BvBuffer.h"
#include "BDeV/Core/Utils/BvUtils.h"


BV_OBJECT_DEFINE_ID(IBvBufferView, "22f4c138-f83b-4f9b-aef3-61dbb1724ae9");
class IBvBufferView : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvBufferView);

public:
	virtual const BufferViewDesc& GetDesc() const = 0;

protected:
	IBvBufferView() {}
	~IBvBufferView() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvBufferView);