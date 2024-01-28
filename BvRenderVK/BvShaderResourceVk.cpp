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
	Create();
}


BvShaderResourceLayoutVk::~BvShaderResourceLayoutVk()
{
	Destroy();
}


void BvShaderResourceLayoutVk::Create()
{
	auto& resources = m_ShaderResourceLayoutDesc.m_PerSetData;
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
		vkCreateDescriptorSetLayout(m_Device.GetHandle(), &layoutCI, nullptr, &layout);
		m_Layouts.Emplace(it.first, layout);
		layouts.PushBack(layout);
	}

	auto& pushConstantResources = m_ShaderResourceLayoutDesc.m_PushConstants;
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
		layout.second = VK_NULL_HANDLE;
	}
	m_Layouts.Clear();
}


BvShaderResourceParamsVk::BvShaderResourceParamsVk(const BvRenderDeviceVk & device, const BvShaderResourceLayoutVk * const pLayout,
	const VkDescriptorSet set, BvShaderResourceSetPoolVk * const pDescriptorPool, const u32 setIndex)
	: m_Device(device), m_Layout(*pLayout),
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


void BvShaderResourceParamsVk::GetDescriptorInfo(const u32 binding, VkDescriptorType & descriptorType, VkImageLayout * pImageLayout)
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