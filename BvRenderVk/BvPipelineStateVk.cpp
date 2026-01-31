#include "BvPipelineStateVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvRenderPassVk.h"
#include "BvShaderResourceVk.h"
#include "BvUtilsVk.h"
#include "BvShaderVk.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"
#include <string_view>


BvGraphicsPipelineStateVk::BvGraphicsPipelineStateVk(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc, VkPipeline pipeline)
	: m_PipelineStateDesc(pipelineStateDesc), m_pDevice(pDevice), m_Pipeline(pipeline)
{
}


BvGraphicsPipelineStateVk::~BvGraphicsPipelineStateVk()
{
	Destroy();
}


void BvGraphicsPipelineStateVk::Destroy()
{
	if (m_Pipeline)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Pipeline);
		m_Pipeline = nullptr;
	}
}


BvComputePipelineStateVk::BvComputePipelineStateVk(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc & pipelineStateDesc, VkPipeline pipeline)
	: m_PipelineStateDesc(pipelineStateDesc), m_pDevice(pDevice), m_Pipeline(pipeline)
{
}


BvComputePipelineStateVk::~BvComputePipelineStateVk()
{
	Destroy();
}


void BvComputePipelineStateVk::Destroy()
{
	if (m_Pipeline)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Pipeline);
		m_Pipeline = nullptr;
	}
}


BvRayTracingPipelineStateVk::BvRayTracingPipelineStateVk(BvRenderDeviceVk* pDevice, const RayTracingPipelineStateDesc& pipelineStateDesc, VkPipeline pipeline)
	: m_PipelineStateDesc(pipelineStateDesc), m_pDevice(pDevice), m_Pipeline(pipeline)
{
}


BvRayTracingPipelineStateVk::~BvRayTracingPipelineStateVk()
{
	Destroy();
}


void BvRayTracingPipelineStateVk::Destroy()
{
	if (m_Pipeline)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Pipeline);
		m_Pipeline = nullptr;
	}
}


BvPipelineCacheVk::BvPipelineCacheVk(BvRenderDeviceVk* pDevice, VkPipelineCache pipelineCache)
	: m_pDevice(pDevice), m_PipelineCache(pipelineCache)
{

}


BvPipelineCacheVk::~BvPipelineCacheVk()
{
	VkHelpers::DestroyDeviceObject(*m_pDevice, m_PipelineCache);
}


void BvPipelineCacheVk::GetCachedData(u64& size, void* pData)
{
	vkGetPipelineCacheData(*m_pDevice, m_PipelineCache, &size, pData);
}