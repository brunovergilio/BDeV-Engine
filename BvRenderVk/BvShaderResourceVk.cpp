#include "BvShaderResourceVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvSamplerVk.h"
#include "BvDescriptorSetVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"
#include "BvUtilsVk.h"
#include <utility>


BvShaderResourceLayoutVk::BvShaderResourceLayoutVk(BvRenderDeviceVk* pDevice, u32 shaderResourceCount,
	const ShaderResourceDesc* pShaderResourceDescs, const ShaderResourceConstantDesc& shaderResourceConstantDesc)
	: m_pDevice(pDevice)
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


BvRenderDevice* BvShaderResourceLayoutVk::GetDevice()
{
	return m_pDevice;
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

		vkCreateDescriptorSetLayout(m_pDevice->GetHandle(), &layoutCI, nullptr, &layouts[layoutIndex]);
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

	vkCreatePipelineLayout(m_pDevice->GetHandle(), &pipelineCI, nullptr, &m_PipelineLayout);
}


void BvShaderResourceLayoutVk::Destroy()
{
	if (m_PipelineLayout)
	{
		vkDestroyPipelineLayout(m_pDevice->GetHandle(), m_PipelineLayout, nullptr);
		m_PipelineLayout = VK_NULL_HANDLE;
	}

	for (auto && layout : m_Layouts)
	{
		vkDestroyDescriptorSetLayout(m_pDevice->GetHandle(), layout.second, nullptr);
	}
	m_Layouts.Clear();
}


BvShaderResourceParamsVk::BvShaderResourceParamsVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, u32 setIndex)
	: m_Device(device), m_Layout(layout), m_Set(setIndex), m_DescriptorPool(&device, &layout, setIndex, 1), m_DescriptorSet(&device, m_DescriptorPool.Allocate())
{
	u32 bufferInfoCount = 0;
	u32 imageInfoCount = 0;
	u32 bufferViewCount = 0;

	auto& sr = layout.GetDesc().m_ShaderResources;
	auto itSet = sr.FindKey(m_Set);
	if (itSet == sr.cend())
	{
		return;
	}

	for (auto& res : itSet->second)
	{
		switch (res.second.m_ShaderResourceType)
		{
		case ShaderResourceType::kConstantBuffer:
		case ShaderResourceType::kStructuredBuffer:
		case ShaderResourceType::kRWStructuredBuffer:
			bufferInfoCount += res.second.m_Count;
			break;
		case ShaderResourceType::kTexture:
		case ShaderResourceType::kRWTexture:
		case ShaderResourceType::kSampler:
			imageInfoCount += res.second.m_Count;
			break;
		case ShaderResourceType::kFormattedBuffer:
		case ShaderResourceType::kRWFormattedBuffer:
			bufferViewCount += res.second.m_Count;
			break;
		}
	}

	m_pSetData = BV_NEW(SetData);
	m_pSetData->m_BufferInfos.Reserve(bufferInfoCount);
	m_pSetData->m_ImageInfos.Reserve(imageInfoCount);
	m_pSetData->m_BufferViews.Reserve(bufferViewCount);
}


BvShaderResourceParamsVk::~BvShaderResourceParamsVk()
{
	if (m_pSetData)
	{
		BV_DELETE(m_pSetData);
	}
}


void BvShaderResourceParamsVk::SetConstantBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& desc = pResource->GetDesc();
		auto pBuffer = TO_VK(desc.m_pBuffer);

		auto& bufferInfo = m_pSetData->m_BufferInfos.EmplaceBack();
		bufferInfo.buffer = pBuffer->GetHandle();
		bufferInfo.offset = desc.m_Offset;
		bufferInfo.range = desc.m_ElementCount * desc.m_Stride;
	}
}


void BvShaderResourceParamsVk::SetStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& desc = pResource->GetDesc();
		auto pBuffer = TO_VK(desc.m_pBuffer);

		auto& bufferInfo = m_pSetData->m_BufferInfos.EmplaceBack();
		bufferInfo.buffer = pBuffer->GetHandle();
		bufferInfo.offset = desc.m_Offset;
		bufferInfo.range = desc.m_ElementCount * desc.m_Stride;
	}
}


void BvShaderResourceParamsVk::SetRWStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& desc = pResource->GetDesc();
		auto pBuffer = TO_VK(desc.m_pBuffer);

		auto& bufferInfo = m_pSetData->m_BufferInfos.EmplaceBack();
		bufferInfo.buffer = pBuffer->GetHandle();
		bufferInfo.offset = desc.m_Offset;
		bufferInfo.range = desc.m_ElementCount * desc.m_Stride;
	}
}


void BvShaderResourceParamsVk::SetFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& bufferView = m_pSetData->m_BufferViews.EmplaceBack();
		bufferView = pResource->GetHandle();
	}
}


void BvShaderResourceParamsVk::SetRWFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& bufferView = m_pSetData->m_BufferViews.EmplaceBack();
		bufferView = pResource->GetHandle();
	}
}


void BvShaderResourceParamsVk::SetTextures(u32 count, const BvTextureView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& imageInfo = m_pSetData->m_ImageInfos.EmplaceBack();
		imageInfo.imageView = pResource->GetHandle();
		imageInfo.imageLayout = IsDepthOrStencilFormat(pResource->GetDesc().m_Format) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		imageInfo.sampler = VK_NULL_HANDLE;
	}
}


void BvShaderResourceParamsVk::SetRWTextures(u32 count, const BvTextureView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& imageInfo = m_pSetData->m_ImageInfos.EmplaceBack();
		imageInfo.imageView = pResource->GetHandle();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.sampler = VK_NULL_HANDLE;
	}
}


void BvShaderResourceParamsVk::SetSamplers(u32 count, const BvSampler* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_SAMPLER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		auto& imageInfo = m_pSetData->m_ImageInfos.EmplaceBack();
		imageInfo.imageView = VK_NULL_HANDLE;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.sampler = pResource->GetHandle();
	}
}


void BvShaderResourceParamsVk::Bind()
{
	for (auto i = 0; i < m_pSetData->m_WriteSets.Size(); ++i)
	{
		auto& writeSet = m_pSetData->m_WriteSets[i];
		u32 index = m_pSetData->m_WriteSetDataIndices[i];
		switch (writeSet.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			writeSet.pImageInfo = &m_pSetData->m_ImageInfos[index];
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			writeSet.pBufferInfo = &m_pSetData->m_BufferInfos[index];
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			writeSet.pTexelBufferView = &m_pSetData->m_BufferViews[index];
			break;
		}
	}

	m_DescriptorSet.Update(m_pSetData->m_WriteSets);
	m_pSetData->m_WriteSets.Clear();
	m_pSetData->m_WriteSetDataIndices.Clear();
}


VkWriteDescriptorSet& BvShaderResourceParamsVk::PrepareWriteSet(VkDescriptorType descriptorType, u32 count, u32 binding, u32 startIndex)
{
	if (!m_pSetData)
	{
		m_pSetData = BV_NEW(SetData);
	}

	m_pSetData->m_WriteSets.PushBack({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
	auto& writeSet = m_pSetData->m_WriteSets.Back();
	writeSet.dstSet = m_DescriptorSet.GetHandle();
	writeSet.descriptorType = descriptorType;
	writeSet.dstBinding = binding;
	writeSet.dstArrayElement = startIndex;
	writeSet.descriptorCount = count;

	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	case VK_DESCRIPTOR_TYPE_SAMPLER:
		m_pSetData->m_WriteSetDataIndices.EmplaceBack(m_pSetData->m_ImageInfos.Size());
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		m_pSetData->m_WriteSetDataIndices.EmplaceBack(m_pSetData->m_BufferInfos.Size());
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		m_pSetData->m_WriteSetDataIndices.EmplaceBack(m_pSetData->m_BufferViews.Size());
		break;
	}

	return writeSet;
}


VkDescriptorType BvShaderResourceParamsVk::GetDescriptorType(u32 binding) const
{
	VkDescriptorType descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
	auto& sr = m_Layout.GetDesc().m_ShaderResources;
	auto itSet = sr.FindKey(m_Set);
	if (itSet == sr.cend())
	{
		auto itBinding = itSet->second.FindKey(binding);
		if (itBinding == itSet->second.cend())
		{
			descriptorType = GetVkDescriptorType(itBinding->second.m_ShaderResourceType);
		}
	}

	return descriptorType;
}