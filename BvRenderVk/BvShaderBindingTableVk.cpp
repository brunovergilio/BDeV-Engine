#include "BvShaderBindingTableVk.h"
#include "BvRenderDeviceVk.h"
#include "BvPipelineStateVk.h"


BvShaderBindingTableVk::BvShaderBindingTableVk(BvRenderDeviceVk* pDevice, const ShaderBindingTableDesc& sbtDesc)
	: BvShaderBindingTable(sbtDesc), m_pDevice(pDevice), m_HandleSize(pDevice->GetDeviceInfo()->m_ExtendedProperties.rayTracingPipelineProps.shaderGroupHandleSize)
{
}


BvShaderBindingTableVk::~BvShaderBindingTableVk()
{
}


BvRenderDevice* BvShaderBindingTableVk::GetDevice()
{
	return m_pDevice;
}