#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"
#include "BDeV/Core/Utils/BvUtils.h"


BV_OBJECT_DEFINE_ID(BvShaderBindingTable, "9f90a079-c378-4b59-9522-958685fdea9c");
class BvShaderBindingTable : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(BvShaderBindingTable);

public:
	BV_INLINE const ShaderBindingTableDesc& GetDesc() const { return m_SBTDesc; }

protected:
	BvShaderBindingTable(const ShaderBindingTableDesc& sbtDesc)
		: m_SBTDesc(sbtDesc) {}
	~BvShaderBindingTable() {}

protected:
	ShaderBindingTableDesc m_SBTDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvShaderBindingTable);