#include "BvShaderResourceVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvSamplerVk.h"
#include <utility>
#include "BvDescriptorSetVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"


BvShaderResourceLayoutVk::BvShaderResourceLayoutVk(const BvRenderDeviceVk& device, u32 shaderResourceCount,
	const ShaderResourceDesc* pShaderResourceDescs, const ShaderResourceConstantDesc& shaderResourceConstantDesc)
	: m_Device(device)
{
	u32 samplerCount = 0;
	for (auto i = 0; i < shaderResourceCount; ++i)
	{
		m_ShaderResourceLayoutDesc.m_ShaderResources[pShaderResourceDescs[i].m_Set][pShaderResourceDescs[i].m_Binding] = pShaderResourceDescs[i];
		if (pShaderResourceDescs[i].m_ppStaticSamplers != nullptr)
		{
			samplerCount += pShaderResourceDescs[i].m_Count;
		}
	}

	m_ShaderResourceLayoutDesc.m_Samplers.Resize(samplerCount);
	u32 samplerIndex = 0;
	for (auto& set : m_ShaderResourceLayoutDesc.m_ShaderResources)
	{
		for (auto& resource : set.second)
		{
			if (resource.second.m_ppStaticSamplers)
			{
				for (auto i = 0; i < resource.second.m_Count; ++i)
				{
					m_ShaderResourceLayoutDesc.m_Samplers[samplerIndex++] = resource.second.m_ppStaticSamplers[i];
				}
				resource.second.m_ppStaticSamplers = m_ShaderResourceLayoutDesc.m_Samplers.Data() + samplerIndex;
			}
		}
	}

	m_ShaderResourceLayoutDesc.m_ShaderResourceConstant = shaderResourceConstantDesc;

	Create();
}


BvShaderResourceLayoutVk::~BvShaderResourceLayoutVk()
{
	Destroy();
}


void BvShaderResourceLayoutVk::Create()
{
	BvVector<VkDescriptorSetLayout> layouts(m_ShaderResourceLayoutDesc.m_ShaderResources.Size());
	u32 layoutIndex = 0;
	for (auto& currSet : m_ShaderResourceLayoutDesc.m_ShaderResources)
	{
		BvVector<VkDescriptorSetLayoutBinding> bindings;
		BvVector<VkSampler> samplers;
		u32 currSamplerIndex = 0;
		for (auto& currResourcePair : currSet.second)
		{
			auto& currResource = currResourcePair.second;
			if (currResource.m_ppStaticSamplers == nullptr)
			{
				bindings.PushBack({ currResource.m_Binding, GetVkDescriptorType(currResource.m_ShaderResourceType),
					currResource.m_Count, GetVkShaderStageFlags(currResource.m_ShaderStages), nullptr });
			}
			else
			{
				for (auto i = 0; i < currResource.m_Count; ++i)
				{
					samplers.PushBack(reinterpret_cast<const BvSamplerVk*>(currResource.m_ppStaticSamplers[i])->GetHandle());
				}
				bindings.PushBack({ currResource.m_Binding, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
					currResource.m_Count, GetVkShaderStageFlags(currResource.m_ShaderStages), samplers.Data() + currSamplerIndex });
				currSamplerIndex += currResource.m_Count;
			}
		}

		VkDescriptorSetLayoutCreateInfo layoutCI{};
		layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCI.bindingCount = bindings.Size();
		layoutCI.pBindings = bindings.Data();

		vkCreateDescriptorSetLayout(m_Device.GetHandle(), &layoutCI, nullptr, &layouts[layoutIndex]);
		m_Layouts[currSet.first] = layouts[layoutIndex++];
	}

	auto& shaderResourceConstant = m_ShaderResourceLayoutDesc.m_ShaderResourceConstant;
	VkPushConstantRange pushConstants{ GetVkShaderStageFlags(shaderResourceConstant.m_ShaderStages), 0, shaderResourceConstant.m_Size };

	VkPipelineLayoutCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if (layouts.Size() > 0)
	{
		pipelineCI.setLayoutCount = (u32)layouts.Size();
		pipelineCI.pSetLayouts = layouts.Data();
	}
	if (pushConstants.size > 0)
	{
		pipelineCI.pushConstantRangeCount = 1;
		pipelineCI.pPushConstantRanges = &pushConstants;
	}

	vkCreatePipelineLayout(m_Device.GetHandle(), &pipelineCI, nullptr, &m_PipelineLayout);
}


void BvShaderResourceLayoutVk::Destroy()
{
	if (m_PipelineLayout)
	{
		vkDestroyPipelineLayout(m_Device.GetHandle(), m_PipelineLayout, nullptr);
		m_PipelineLayout = VK_NULL_HANDLE;
	}

	for (auto && layout : m_Layouts)
	{
		vkDestroyDescriptorSetLayout(m_Device.GetHandle(), layout.second, nullptr);
	}
	m_Layouts.Clear();
}


BvShaderResourceParamsVk::BvShaderResourceParamsVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, BvDescriptorPoolVk& descriptorPool, u32 setIndex)
	: m_Device(device), m_Layout(layout)
{
}


BvShaderResourceParamsVk::~BvShaderResourceParamsVk()
{
}


void BvShaderResourceParamsVk::SetResources(u32 count, const BvBufferView* const* ppBuffers, u32 binding, u32 startIndex)
{
}


void BvShaderResourceParamsVk::SetResources(u32 count, const BvTextureView* const* ppTextures, u32 binding, u32 startIndex)
{
}


void BvShaderResourceParamsVk::SetResources(u32 count, const BvSampler* const* ppSamplers, u32 binding, u32 startIndex)
{
}


void BvShaderResourceParamsVk::Bind()
{
}