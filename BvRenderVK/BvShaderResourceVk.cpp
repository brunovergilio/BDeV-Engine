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


void BvShaderResourceLayoutVk::BvDescriptorDataVk(u32 set, BvDescriptorDataVk& descriptorData)
{
	for (auto i = 0; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& setDesc = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		if (setDesc.m_Set == set)
		{
			for (auto j = 0; j < setDesc.m_ShaderResourceCount; ++j)
			{
				auto& resourceDesc = setDesc.m_pShaderResources[j];
				switch (resourceDesc.m_ShaderResourceType)
				{
				case ShaderResourceType::kConstantBuffer:
				case ShaderResourceType::kStructuredBuffer:
				case ShaderResourceType::kRWStructuredBuffer:
					auto it = descriptorData.m_BufferBindings.Emplace(resourceDesc.m_Binding, BvBufferBindingsVk());
					for (auto k = 0; k < resourceDesc.m_Count; ++k)
					{
						it.first->second.Emplace(k, VkDescriptorBufferInfo());
					}
					break;
				case ShaderResourceType::kTexture:
				case ShaderResourceType::kRWTexture:
				case ShaderResourceType::kSampler:
					auto it = descriptorData.m_ImageBindings.Emplace(resourceDesc.m_Binding, BvImageBindingsVk());
					for (auto k = 0; k < resourceDesc.m_Count; ++k)
					{
						it.first->second.Emplace(k, VkDescriptorImageInfo());
					}
					break;
				case ShaderResourceType::kFormattedBuffer:
				case ShaderResourceType::kRWFormattedBuffer:
					auto it = descriptorData.m_BufferViewBindings.Emplace(resourceDesc.m_Binding, BvBufferViewBindingsVk());
					for (auto k = 0; k < resourceDesc.m_Count; ++k)
					{
						it.first->second.Emplace(k, VK_NULL_HANDLE);
					}
					break;
				}
			}

			break;
		}
	}
}


BvShaderResourceParamsVk::BvShaderResourceParamsVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, BvDescriptorPoolVk& descriptorPool, u32 setIndex)
	: m_Device(device), m_Layout(layout),
	m_DescriptorSet(set), m_SetIndex(setIndex)
{
	Create();
}


BvShaderResourceParamsVk::~BvShaderResourceParamsVk()
{
	Destroy();
}


void BvShaderResourceParamsVk::Create()
{
	const auto& shaderResources = m_Layout.GetDesc().m_PerSetData.At(m_SetIndex).m_ShaderResources;
	m_WriteSets.Reserve(shaderResources.Size());

	// Count resources
	uint32_t totalResources = 0;
	m_ShaderVariables.Resize(shaderResources.Size());
	for (auto i = 0u; i < shaderResources.Size(); i++)
	{
		switch (shaderResources[i].m_ShaderResourceType)
		{
		case ShaderResourceType::kConstantBuffer:
		case ShaderResourceType::kStructuredBuffer:
		case ShaderResourceType::kRWStructuredBuffer:
			m_ShaderVariables[i] = new BvShaderBufferVariableVk(*this, shaderResources[i].m_Binding, shaderResources[i].m_Count);
			break;
		case ShaderResourceType::kFormattedBuffer:
		case ShaderResourceType::kRWFormattedBuffer:
			m_ShaderVariables[i] = new BvShaderBufferViewVariableVk(*this, shaderResources[i].m_Binding, shaderResources[i].m_Count);
			break;
		case ShaderResourceType::kTexture:
		case ShaderResourceType::kRWTexture:
		case ShaderResourceType::kSampler:
			m_ShaderVariables[i] = new BvShaderImageVariableVk(*this, shaderResources[i].m_Binding, shaderResources[i].m_Count);
			break;
		}
	}
}


void BvShaderResourceParamsVk::Destroy()
{
	m_WriteSets.Clear();
	for (auto&& pVariable : m_ShaderVariables)
	{
		delete pVariable;
	}
	m_ShaderVariables.Clear();

	m_DescriptorSet = VK_NULL_HANDLE;
}


void BvShaderResourceParamsVk::SetBuffers(const u32 binding, const u32 count, const BvBufferView * const * const ppBuffers, const u32 startIndex)
{
	for (auto&& pShaderVariable : m_ShaderVariables)
	{
		if (pShaderVariable->GetBinding() == binding)
		{
			pShaderVariable->SetBuffers(count, ppBuffers, startIndex);
			break;
		}
	}
}


void BvShaderResourceParamsVk::SetTextures(const u32 binding, const u32 count, const BvTextureView * const * const ppTextures, const u32 startIndex)
{
	for (auto&& pShaderVariable : m_ShaderVariables)
	{
		if (pShaderVariable->GetBinding() == binding)
		{
			pShaderVariable->SetTextures(count, ppTextures, startIndex);
			break;
		}
	}
}


void BvShaderResourceParamsVk::SetSamplers(const u32 binding, const u32 count, const BvSampler * const * const ppSamplers, const u32 startIndex)
{
	for (auto&& pShaderVariable : m_ShaderVariables)
	{
		if (pShaderVariable->GetBinding() == binding)
		{
			pShaderVariable->SetSamplers(count, ppSamplers, startIndex);
			break;
		}
	}
}


void BvShaderResourceParamsVk::Update()
{
	if (m_WriteSets.Size() == 0)
	{
		return;
	}

	vkUpdateDescriptorSets(m_Device.GetHandle(), m_WriteSets.Size(),
		m_WriteSets.Data(), 0, nullptr);

	m_WriteSets.Clear();
}


VkWriteDescriptorSet& BvShaderResourceParamsVk::GetWriteSet(const u32 binding)
{
	for (auto&& set : m_WriteSets)
	{
		if (set.dstBinding == binding)
		{
			return set;
		}
	}

	m_WriteSets.PushBack({});
	return m_WriteSets.Back();
}


void BvShaderResourceParamsVk::GetDescriptorInfo(u32 binding, VkDescriptorType& descriptorType, VkImageLayout* pImageLayout)
{
	const auto& shaderResources = m_Layout.GetDesc().m_PerSetData.At(m_SetIndex).m_ShaderResources;
	for (auto && resource : shaderResources)
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


void BvShaderBufferVariableVk::SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex)
{
	VkDescriptorType descriptorType;
	m_ShaderParams.GetDescriptorInfo(m_Binding, descriptorType);

	auto& writeSet = m_ShaderParams.GetWriteSet(m_Binding);
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = m_Binding;
	writeSet.descriptorCount = count;
	writeSet.dstArrayElement = startIndex;
	writeSet.dstSet = m_ShaderParams.GetHandle();

	writeSet.pBufferInfo = m_BufferInfos.Data();
	for (auto i = startIndex; i < count && i < m_BufferInfos.Size(); i++)
	{
		auto& desc = ppBuffers[i]->GetDesc();
		auto pBuffer = static_cast<const BvBufferVk* const>(desc.m_pBuffer);
		m_BufferInfos[i].buffer = pBuffer->GetHandle();
		m_BufferInfos[i].offset = desc.m_Offset;
		m_BufferInfos[i].range = desc.m_ElementCount * desc.m_Stride;
	}
}


void BvShaderBufferVariableVk::SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex)
{
	BvAssert(0, "This binding slot is reserved for buffers");
}


void BvShaderBufferVariableVk::SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex)
{
	BvAssert(0, "This binding slot is reserved for buffers");
}


void BvShaderImageVariableVk::SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex)
{
	BvAssert(0, "This binding slot is reserved for images and samplers");
}


void BvShaderImageVariableVk::SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex)
{
	VkImageLayout layout;
	VkDescriptorType descriptorType;
	m_ShaderParams.GetDescriptorInfo(m_Binding, descriptorType, &layout);

	auto& writeSet = m_ShaderParams.GetWriteSet(m_Binding);
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = m_Binding;
	writeSet.descriptorCount = count;
	writeSet.dstArrayElement = startIndex;
	writeSet.dstSet = m_ShaderParams.GetHandle();

	for (auto i = startIndex; i < count && i < m_ImageInfos.Size(); i++)
	{
		m_ImageInfos[i].imageLayout = layout;
		m_ImageInfos[i].imageView = static_cast<const BvTextureViewVk* const>(ppTextures[i])->GetHandle();
		m_ImageInfos[i].sampler = VK_NULL_HANDLE;
	}
}


void BvShaderImageVariableVk::SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex)
{
	VkDescriptorType descriptorType;
	m_ShaderParams.GetDescriptorInfo(m_Binding, descriptorType);

	auto& writeSet = m_ShaderParams.GetWriteSet(m_Binding);
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = m_Binding;
	writeSet.descriptorCount = count;
	writeSet.dstArrayElement = startIndex;
	writeSet.dstSet = m_ShaderParams.GetHandle();

	for (auto i = startIndex; i < count && i < m_ImageInfos.Size(); i++)
	{
		m_ImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_ImageInfos[i].imageView = VK_NULL_HANDLE;
		m_ImageInfos[i].sampler = static_cast<const BvSamplerVk* const>(ppSamplers[i])->GetHandle();
	}
}


void BvShaderBufferViewVariableVk::SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex)
{
	VkDescriptorType descriptorType;
	m_ShaderParams.GetDescriptorInfo(m_Binding, descriptorType);

	auto& writeSet = m_ShaderParams.GetWriteSet(m_Binding);
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = m_Binding;
	writeSet.descriptorCount = count;
	writeSet.dstArrayElement = startIndex;
	writeSet.dstSet = m_ShaderParams.GetHandle();

	writeSet.pTexelBufferView = m_BufferViews.Data();
	for (auto i = startIndex; i < count && i < m_BufferViews.Size(); i++)
	{
		auto pBufferView = static_cast<const BvBufferViewVk* const>(ppBuffers[i]);
		m_BufferViews[i] = pBufferView->GetHandle();
	}
}


void BvShaderBufferViewVariableVk::SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex)
{
	BvAssert(0, "This binding slot is reserved for buffer views");
}


void BvShaderBufferViewVariableVk::SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex)
{
	BvAssert(0, "This binding slot is reserved for buffer views");
}