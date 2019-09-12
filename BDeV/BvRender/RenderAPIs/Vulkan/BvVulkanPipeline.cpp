#include "BvVulkanPipeline.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"


BvVulkanPipelineLayout::BvVulkanPipelineLayout(const BvVulkanDevice * const pDevice, const VkPipelineLayoutCreateInfo & createInfo)
	: m_pDevice(pDevice)
{
	Create(createInfo);
}


BvVulkanPipelineLayout::~BvVulkanPipelineLayout()
{
	Destroy();
}


void BvVulkanPipelineLayout::Create(const VkPipelineLayoutCreateInfo & createInfo)
{
	vkCreatePipelineLayout(m_pDevice->GetLogical(), &createInfo, nullptr, &m_PipelineLayout);
}


void BvVulkanPipelineLayout::Destroy()
{
	if (m_PipelineLayout)
	{
		vkDestroyPipelineLayout(m_pDevice->GetLogical(), m_PipelineLayout, nullptr);
	}
}


BvVulkanPipelineCache::BvVulkanPipelineCache(const BvVulkanDevice * const pDevice)
	: m_pDevice(pDevice)
{
	Create();
}


BvVulkanPipelineCache::~BvVulkanPipelineCache()
{
	Destroy();
}


void BvVulkanPipelineCache::Create()
{
	VkPipelineCacheCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	vkCreatePipelineCache(m_pDevice->GetLogical(), &createInfo, nullptr, &m_PipelineCache);
}


void BvVulkanPipelineCache::Destroy()
{
	if (m_PipelineCache)
	{
		vkDestroyPipelineCache(m_pDevice->GetLogical(), m_PipelineCache, nullptr);
		m_PipelineCache = VK_NULL_HANDLE;
	}
}


BvVulkanPipeline::BvVulkanPipeline(const BvVulkanDevice * const pDevice,
	const VkGraphicsPipelineCreateInfo & createInfo, const VkPipelineCache pipelineCache)
	: m_pDevice(pDevice)
{
	Create(createInfo, pipelineCache);
}


BvVulkanPipeline::~BvVulkanPipeline()
{
	Destroy();
}


void BvVulkanPipeline::Create(const VkGraphicsPipelineCreateInfo & createInfo, const VkPipelineCache pipelineCache)
{
	vkCreateGraphicsPipelines(m_pDevice->GetLogical(), pipelineCache, 1, &createInfo, nullptr, &m_Pipeline);
}


void BvVulkanPipeline::Destroy()
{
	if (m_Pipeline)
	{
		vkDestroyPipeline(m_pDevice->GetLogical(), m_Pipeline, nullptr);
		m_Pipeline = VK_NULL_HANDLE;
	}
}