#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"
#include "BDeV/Core/Utils/BvUtils.h"


BV_OBJECT_DEFINE_ID(IBvShaderBindingTable, "9f90a079-c378-4b59-9522-958685fdea9c");
class IBvShaderBindingTable : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvShaderBindingTable);

public:
	virtual const ShaderBindingTableDesc& GetDesc() const = 0;
	virtual void GetDeviceAddressRange(ShaderBindingTableGroupType type, u32 index, DeviceAddressRange& addressRange) const = 0;
	virtual void GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType type, u32 index, DeviceAddressRangeAndStride& addressRangeAndStride) const = 0;

protected:
	IBvShaderBindingTable() {}
	~IBvShaderBindingTable() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderBindingTable);