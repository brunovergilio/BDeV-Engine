#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderBindingTable.h"


class BvRenderDevice;
class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvShaderBindingTableVk, "0bb5abad-c53b-47bd-b939-989cb9d2e58d");
class BvShaderBindingTableVk : public BvShaderBindingTable
{
	BV_NOCOPYMOVE(BvShaderBindingTableVk);

public:
	BvShaderBindingTableVk(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc);
	~BvShaderBindingTableVk();

	BvRenderDevice* GetDevice() override;

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	u32 m_HandleSize = 0;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvShaderBindingTableVk);


BV_CREATE_CAST_TO_VK(BvShaderBindingTable)