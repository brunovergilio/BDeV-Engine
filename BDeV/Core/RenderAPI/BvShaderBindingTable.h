#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvObject.h"


//BV_OBJECT_DEFINE_ID(IBvShaderBindingTable, "9f90a079-c378-4b59-9522-958685fdea9c");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderBindingTable);
class IBvShaderBindingTable : public BvRCObj
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