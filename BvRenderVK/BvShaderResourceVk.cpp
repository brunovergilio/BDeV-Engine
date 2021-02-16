#include "BvShaderResourceVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvSamplerVk.h"
#include <utility>
#include "BvShaderResourceSetPoolVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"


constexpr u32 kMaxDescriptorSets = 8;


BvShaderResourceLayoutVk::BvShaderResourceLayoutVk(const BvRenderDeviceVk & device,
	const ShaderResourceLayoutDesc & shaderResourceLayoutDesc)
	: BvShaderResourceLayout(shaderResourceLayoutDesc), m_Device(device)
{
}


BvShaderResourceLayoutVk::~BvShaderResourceLayoutVk()
{
	Destroy();
}


void BvShaderResourceLayoutVk::Create()
{
	decltype(auto) resources = m_ShaderResourceLayoutDesc.GetSetData();
	BvVector<VkDescriptorSetLayout> layouts(m_Layouts.Size());
	for (decltype(auto) it : resources)
	{
		BvVector<VkDescriptorSetLayoutBinding> bindings;
		BvVector<VkSampler> samplers;
		u32 currSamplerIndex = 0;
		for (auto && item : it.second.m_ShaderResources)
		{
			bindings.PushBack({ item.m_Binding, GetVkDescriptorType(item.m_ShaderResourceType),
				item.m_Count, GetVkShaderStageFlags(item.m_ShaderStages), nullptr });
		}
		for (auto && item : it.second.m_StaticSamplers)
		{
			for (auto & sampler : item.m_Samplers)
			{
				samplers.PushBack(reinterpret_cast<BvSamplerVk *>(sampler)->GetHandle());
			}
			bindings.PushBack({ item.m_Binding, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
				(u32)item.m_Samplers.Size(), GetVkShaderStageFlags(item.m_ShaderStages), samplers.Data() + currSamplerIndex });
			currSamplerIndex += item.m_Samplers.Size();
		}

		VkDescriptorSetLayoutCreateInfo layoutCI{};
		layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCI.bindingCount = bindings.Size();
		layoutCI.pBindings = bindings.Data();

		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		m_Device.GetDeviceFunctions().vkCreateDescriptorSetLayout(m_Device.GetHandle(), &layoutCI, nullptr, &layout);
		m_Layouts.Emplace(it.first, layout);
		layouts.PushBack(layout);
	}

	decltype(auto) pushConstantResources = m_ShaderResourceLayoutDesc.GetPushConstants();
	BvVector<VkPushConstantRange> pushConstants(pushConstantResources.Size());
	for (auto i = 0u; i < pushConstants.Size(); i++)
	{
		pushConstants[i].size = pushConstantResources[i].m_Size;
		pushConstants[i].offset = 0;
		pushConstants[i].stageFlags = GetVkShaderStageFlags(pushConstantResources[i].m_ShaderStages);
	}

	VkPipelineLayoutCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if (layouts.Size() > 0)
	{
		pipelineCI.setLayoutCount = (u32)layouts.Size();
		pipelineCI.pSetLayouts = layouts.Data();
	}
	if (pushConstants.Size() > 0)
	{
		pipelineCI.pushConstantRangeCount = (u32)pushConstants.Size();
		pipelineCI.pPushConstantRanges = pushConstants.Data();
	}

	m_Device.GetDeviceFunctions().vkCreatePipelineLayout(m_Device.GetHandle(), &pipelineCI, nullptr, &m_PipelineLayout);
}


void BvShaderResourceLayoutVk::Destroy()
{
	for (auto && layout : m_Layouts)
	{
		m_Device.GetDeviceFunctions().vkDestroyDescriptorSetLayout(m_Device.GetHandle(), layout.second, nullptr);
		layout.second = VK_NULL_HANDLE;
	}
	m_Layouts.Clear();

	if (m_PipelineLayout)
	{
		m_Device.GetDeviceFunctions().vkDestroyPipelineLayout(m_Device.GetHandle(), m_PipelineLayout, nullptr);
		m_PipelineLayout = VK_NULL_HANDLE;
	}
}


BvShaderResourceSetVk::BvShaderResourceSetVk(const BvRenderDeviceVk & device, const BvShaderResourceLayoutVk * const pLayout,
	const VkDescriptorSet set, BvShaderResourceSetPoolVk * const pDescriptorPool, const u32 setIndex)
	: m_Device(device), m_SetDesc(pLayout->GetDesc().GetSetData().At(setIndex)),
	m_DescriptorSet(set), m_SetIndex(setIndex)
{
	Create();
}


BvShaderResourceSetVk::~BvShaderResourceSetVk()
{
	Destroy();
}


void BvShaderResourceSetVk::Create()
{
	m_pDescriptorData = new DescriptorData;
	m_pDescriptorData->m_WriteSets.Resize(m_SetDesc.m_ShaderResources.Size(), {});

	// Count resources
	uint32_t totalResources = 0;
	for (auto i = 0u; i < m_SetDesc.m_ShaderResources.Size(); i++)
	{
		totalResources += m_SetDesc.m_ShaderResources[i].m_Count;
	}
	m_pDescriptorData->m_Resources.Resize(totalResources);

	// Fill out the structures
	uint32_t resourceIndex = 0;
	for (auto i = 0u; i < m_SetDesc.m_ShaderResources.Size(); i++)
	{
		m_pDescriptorData->m_WriteSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		m_pDescriptorData->m_WriteSets[i].descriptorCount = m_SetDesc.m_ShaderResources[i].m_Count;
		m_pDescriptorData->m_WriteSets[i].dstBinding = m_SetDesc.m_ShaderResources[i].m_Binding;
		m_pDescriptorData->m_WriteSets[i].descriptorType = GetVkDescriptorType(m_SetDesc.m_ShaderResources[i].m_ShaderResourceType);
		m_pDescriptorData->m_WriteSets[i].dstSet = m_DescriptorSet;
		m_pDescriptorData->m_WriteSets[i].pImageInfo = &m_pDescriptorData->m_Resources[resourceIndex].imageInfo;

		resourceIndex += m_SetDesc.m_ShaderResources[i].m_Count;
	}
}


void BvShaderResourceSetVk::Destroy()
{
	m_DescriptorSet = VK_NULL_HANDLE;
	
	BvDelete(m_pDescriptorData);
}


void BvShaderResourceSetVk::SetBuffers(const u32 binding, const u32 count, const BvBufferView * const * const ppBuffers)
{
	VkDescriptorType descriptorType;
	GetDescriptorInfo(binding, descriptorType);

	auto & writeSet = m_pDescriptorData->m_WriteSets[m_pDescriptorData->m_DirtyWriteSetCount++];
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = binding;
	writeSet.descriptorCount = count;
	writeSet.dstArrayElement = 0;
	writeSet.dstSet = m_DescriptorSet;

	auto currResIndex = m_pDescriptorData->m_DirtyResourceInfoCount;
	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		writeSet.pBufferInfo = &m_pDescriptorData->m_Resources[currResIndex].bufferInfo;
		for (auto i = 0; i < count; i++, currResIndex++)
		{
			auto & desc = ppBuffers[i]->GetDesc();
			auto pBuffer = static_cast<const BvBufferVk * const>(desc.m_pBuffer);
			m_pDescriptorData->m_Resources[currResIndex].bufferInfo.buffer = pBuffer->GetHandle();
			m_pDescriptorData->m_Resources[currResIndex].bufferInfo.offset = desc.m_Offset;
			m_pDescriptorData->m_Resources[currResIndex].bufferInfo.range = desc.m_ElementCount * desc.m_Stride;
		}
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		writeSet.pTexelBufferView = &m_pDescriptorData->m_Resources[currResIndex].texelBufferView;
		for (auto i = 0; i < count; i++, currResIndex++)
		{
			auto pBufferView = static_cast<const BvBufferViewVk * const>(ppBuffers[i]);
			m_pDescriptorData->m_Resources[currResIndex].texelBufferView = pBufferView->GetHandle();
		}
		break;
	default:
		BvAssert(0);
	}

	m_pDescriptorData->m_DirtyResourceInfoCount = currResIndex;
}


void BvShaderResourceSetVk::SetTextures(const u32 binding, const u32 count, const BvTextureView * const * const ppTextures)
{
	VkImageLayout layout;
	VkDescriptorType descriptorType;
	GetDescriptorInfo(binding, descriptorType, &layout);

	auto & writeSet = m_pDescriptorData->m_WriteSets[m_pDescriptorData->m_DirtyWriteSetCount++];
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = binding;
	writeSet.descriptorCount = count;
	writeSet.dstArrayElement = 0;
	writeSet.dstSet = m_DescriptorSet;

	auto currResIndex = m_pDescriptorData->m_DirtyResourceInfoCount;
	for (auto i = 0; i < count; i++, currResIndex++)
	{
		m_pDescriptorData->m_Resources[currResIndex].imageInfo.imageLayout = layout;
		m_pDescriptorData->m_Resources[currResIndex].imageInfo.imageView = static_cast<const BvTextureViewVk * const>(ppTextures[i])->GetHandle();
		m_pDescriptorData->m_Resources[currResIndex].imageInfo.sampler = VK_NULL_HANDLE;
	}

	m_pDescriptorData->m_DirtyResourceInfoCount = currResIndex;
}


void BvShaderResourceSetVk::SetSamplers(const u32 binding, const u32 count, const BvSampler * const * const ppSamplers)
{
	VkDescriptorType descriptorType;
	GetDescriptorInfo(binding, descriptorType);

	auto & writeSet = m_pDescriptorData->m_WriteSets[m_pDescriptorData->m_DirtyWriteSetCount++];
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = binding;
	writeSet.descriptorCount = count;
	writeSet.dstArrayElement = 0;
	writeSet.dstSet = m_DescriptorSet;

	auto currResIndex = m_pDescriptorData->m_DirtyResourceInfoCount;
	for (auto i = 0; i < count; i++, currResIndex++)
	{
		m_pDescriptorData->m_Resources[currResIndex].imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_pDescriptorData->m_Resources[currResIndex].imageInfo.imageView = VK_NULL_HANDLE;
		m_pDescriptorData->m_Resources[currResIndex].imageInfo.sampler = static_cast<const BvSamplerVk * const>(ppSamplers[i])->GetHandle();
	}

	m_pDescriptorData->m_DirtyResourceInfoCount = currResIndex;
}


void BvShaderResourceSetVk::Update()
{
	if (m_pDescriptorData->m_DirtyWriteSetCount == 0)
	{
		return;
	}

	m_Device.GetDeviceFunctions().vkUpdateDescriptorSets(m_Device.GetHandle(), m_pDescriptorData->m_DirtyWriteSetCount,
		m_pDescriptorData->m_WriteSets.Data(), 0, nullptr);

	m_pDescriptorData->m_DirtyWriteSetCount = 0;
	m_pDescriptorData->m_DirtyResourceInfoCount = 0;
}


void BvShaderResourceSetVk::GetDescriptorInfo(const u32 binding, VkDescriptorType & descriptorType, VkImageLayout * pImageLayout)
{
	for (auto && resource : m_SetDesc.m_ShaderResources)
	{
		if (resource.m_Binding == binding)
		{
			descriptorType = GetVkDescriptorType(resource.m_ShaderResourceType);

			if (pImageLayout)
			{
				if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
				{
					*pImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
				}
				else if (descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
				{
					*pImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
			}
			
			break;
		}
	}
}