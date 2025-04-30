#include "BvShaderResourceVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvSamplerVk.h"
#include "BvDescriptorSetVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"
#include "BvAccelerationStructureVk.h"
#include "BvUtilsVk.h"
#include <utility>


BV_VK_DEVICE_RES_DEF(BvShaderResourceLayoutVk)


BvShaderResourceLayoutVk::BvShaderResourceLayoutVk(BvRenderDeviceVk* pDevice, const ShaderResourceLayoutDesc& srlDesc)
	: m_ShaderResourceLayoutDesc(srlDesc), m_pDevice(pDevice)
{
	u32 totalResourceCount = 0;
	u32 totalSamplerCount = 0;
	u32 totalConstantCount = 0;
	// Count all the resources we have so we can make internal copies of them
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		totalResourceCount += set.m_ResourceCount;
		for (auto j = 0u; j < set.m_ResourceCount; ++j)
		{
			auto& res = set.m_pResources[j];
			if (res.m_ppStaticSamplers != nullptr)
			{
				totalSamplerCount += res.m_Count;
			}
		}
		totalConstantCount += set.m_ConstantCount;
	}

	// Now we allocate space for the objects we need
	m_pShaderResourceSets = m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount ?
		BV_NEW_ARRAY(ShaderResourceSetDesc, m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount) : nullptr;
	m_pShaderResources = totalResourceCount ? BV_NEW_ARRAY(ShaderResourceDesc, totalResourceCount) : nullptr;
	m_ppStaticSamplers = totalSamplerCount ? BV_NEW_ARRAY(IBvSampler*, totalSamplerCount) : nullptr;
	m_pShaderConstants = totalConstantCount > 0 ? BV_NEW_ARRAY(ShaderResourceConstantDesc, totalConstantCount) : nullptr;

	// Start moving data
	u32 currResource = 0;
	u32 currSampler = 0;
	u32 currConstant = 0;

	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		u32 firstResource = currResource;
		for (auto j = 0u; j < set.m_ResourceCount; ++j)
		{
			auto& res = m_pShaderResources[currResource];
			res = set.m_pResources[j];

			if (res.m_ppStaticSamplers != nullptr)
			{
				memcpy(&m_ppStaticSamplers[currSampler], res.m_ppStaticSamplers, sizeof(IBvSampler*) * res.m_Count);
				res.m_ppStaticSamplers = &m_ppStaticSamplers[currSampler];
				currSampler += res.m_Count;
			}

			++currResource;
		}
		std::sort(m_pShaderResources + firstResource, m_pShaderResources + currResource);

		u32 firstConstant = currConstant;
		for (auto j = 0u; j < set.m_ConstantCount; ++j)
		{
			auto& constant = m_pShaderConstants[currConstant];
			constant = set.m_pConstants[j];
			++currConstant;
		}
		std::sort(m_pShaderConstants + firstConstant, m_pShaderConstants + currConstant);

		m_pShaderResourceSets[i] = set;
		m_pShaderResourceSets[i].m_pResources = &m_pShaderResources[firstResource];
	}
	std::sort(m_pShaderResourceSets, m_pShaderResourceSets + m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount);

	m_ShaderResourceLayoutDesc.m_pShaderResourceSets = m_pShaderResourceSets;

	Create();
}


BvShaderResourceLayoutVk::~BvShaderResourceLayoutVk()
{
	Destroy();

	if (m_pShaderConstants)
	{
		BV_DELETE_ARRAY(m_pShaderConstants);
	}

	if (m_ppStaticSamplers)
	{
		BV_DELETE_ARRAY(m_ppStaticSamplers);
	}

	if (m_pShaderResources)
	{
		BV_DELETE_ARRAY(m_pShaderResources);
	}

	if (m_pShaderResourceSets)
	{
		BV_DELETE_ARRAY(m_ShaderResourceLayoutDesc.m_pShaderResourceSets);
	}
}


const ShaderResourceSetDesc* BvShaderResourceLayoutVk::GetResourceSet(u32 set) const
{
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto pSet = &m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		if (pSet->m_Index == set)
		{
			return pSet;
		}
	}

	return nullptr;
}


const ShaderResourceDesc* BvShaderResourceLayoutVk::GetResource(u32 binding, u32 set) const
{
	auto it = m_Resources.FindKey({ binding, set });
	return it != m_Resources.cend() ? it->second : nullptr;
}


BvShaderResourceLayoutVk::PushConstantData* BvShaderResourceLayoutVk::GetPushConstantData(u32 size, u32 binding, u32 set) const
{
	auto it = m_PushConstantOffsets.FindKey({ size, binding, set });
	return it != m_PushConstantOffsets.cend() ? &it->second : nullptr;
}


void BvShaderResourceLayoutVk::Create()
{
	BvVector<VkDescriptorSetLayout> layouts(m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount);

	BvVector<VkDescriptorSetLayoutBinding> bindings;
	BvVector<VkDescriptorBindingFlags> flags;
	BvVector<VkSampler> samplers;
	BvVector<VkPushConstantRange> pushConstants;
	u32 layoutIndex = 0;
	u32 currPushConstantOffset = 0;
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSetCount; ++i)
	{
		auto& set = m_ShaderResourceLayoutDesc.m_pShaderResourceSets[i];
		u32 currSamplerIndex = 0;
		bool isBindful = false;
		bool isBindless = false;
		for (auto j = 0u; j < set.m_ResourceCount; ++j)
		{
			auto& currResource = set.m_pResources[j];
			if (currResource.m_ppStaticSamplers == nullptr)
			{
				bindings.PushBack({ currResource.m_Binding, GetVkDescriptorType(currResource.m_ShaderResourceType),
					currResource.m_Count, GetVkShaderStageFlags(currResource.m_ShaderStages), nullptr });
			}
			else
			{
				for (auto k = 0; k < currResource.m_Count; ++k)
				{
					samplers.PushBack(TO_VK(currResource.m_ppStaticSamplers[k])->GetHandle());
				}
				bindings.PushBack({ currResource.m_Binding, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
					currResource.m_Count, GetVkShaderStageFlags(currResource.m_ShaderStages), samplers.Data() + currSamplerIndex });
				currSamplerIndex += currResource.m_Count;
			}
			if (currResource.m_Bindless)
			{
				isBindless = true;
				flags.EmplaceBack() = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			}
			else
			{
				isBindful = true;
			}

			m_Resources.Emplace({ currResource.m_Binding, set.m_Index }, &currResource);
		}

		if (isBindful && isBindless)
		{
			// I decided to keep these separate for the moment, as dealing with both on the same set would be dreadful
			BV_ASSERT(false, "Can't have bindful and bindless resources on the same set");
			Destroy();
			return;
		}

		VkDescriptorSetLayoutCreateInfo layoutCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutCI.bindingCount = bindings.Size();
		layoutCI.pBindings = bindings.Data();

		VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
		if (isBindless)
		{
			flagsCI.bindingCount = flags.Size();
			flagsCI.pBindingFlags = flags.Data();
			layoutCI.pNext = &flagsCI;
			layoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		}

		vkCreateDescriptorSetLayout(m_pDevice->GetHandle(), &layoutCI, nullptr, &layouts[layoutIndex]);
		m_Layouts[set.m_Index] = layouts[layoutIndex++];

		bindings.Clear();
		samplers.Clear();

		for (auto j = 0u; j < set.m_ConstantCount; ++j)
		{
			auto& constant = set.m_pConstants[j];
			pushConstants.PushBack({ GetVkShaderStageFlags(constant.m_ShaderStages), currPushConstantOffset, constant.m_Size });
			m_PushConstantOffsets.Emplace({ constant.m_Size, constant.m_Binding, set.m_Index }, PushConstantData{ pushConstants.Back().stageFlags, currPushConstantOffset });
			currPushConstantOffset += constant.m_Size;
		}
	}

	VkPipelineLayoutCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	if (layouts.Size() > 0)
	{
		pipelineCI.setLayoutCount = u32(layouts.Size());
		pipelineCI.pSetLayouts = layouts.Data();
	}

	if (pushConstants.Size() > 0)
	{
		pipelineCI.pushConstantRangeCount = u32(pushConstants.Size());
		pipelineCI.pPushConstantRanges = pushConstants.Data();
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

	for (auto& layout : m_Layouts)
	{
		vkDestroyDescriptorSetLayout(m_pDevice->GetHandle(), layout.second, nullptr);
	}
	m_Layouts.Clear();
}


BvShaderResourceParamsVk::BvShaderResourceParamsVk(BvRenderDeviceVk* pDevice, const BvShaderResourceLayoutVk* pLayout, u32 set)
	: m_pDevice(pDevice), m_pLayout(pLayout), m_Set(set), m_DescriptorPool(pDevice, pLayout, set, 1), m_DescriptorSet(pDevice, m_DescriptorPool.Allocate())
{
	auto pSet = m_pLayout->GetResourceSet(set);
	BV_ASSERT(pSet != nullptr, "Set not found in current Shader Resource Layout");
	u32 bufferInfoCount = 0;
	u32 imageInfoCount = 0;
	u32 bufferViewCount = 0;

	for (auto i = 0u; i < pSet->m_ResourceCount; ++i)
	{
		auto& res = pSet->m_pResources[i];
		switch (res.m_ShaderResourceType)
		{
		case ShaderResourceType::kConstantBuffer:
		case ShaderResourceType::kStructuredBuffer:
		case ShaderResourceType::kRWStructuredBuffer:
			bufferInfoCount += res.m_Count;
			break;
		case ShaderResourceType::kTexture:
		case ShaderResourceType::kRWTexture:
		case ShaderResourceType::kSampler:
			imageInfoCount += res.m_Count;
			break;
		case ShaderResourceType::kFormattedBuffer:
		case ShaderResourceType::kRWFormattedBuffer:
			bufferViewCount += res.m_Count;
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


void BvShaderResourceParamsVk::SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex)
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


void BvShaderResourceParamsVk::SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex)
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


void BvShaderResourceParamsVk::SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex)
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


void BvShaderResourceParamsVk::SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		m_pSetData->m_BufferViews.EmplaceBack(pResource->GetHandle());
	}
}


void BvShaderResourceParamsVk::SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		m_pSetData->m_BufferViews.EmplaceBack(pResource->GetHandle());
	}
}


void BvShaderResourceParamsVk::SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex)
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


void BvShaderResourceParamsVk::SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex)
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


void BvShaderResourceParamsVk::SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 binding, u32 startIndex)
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


void BvShaderResourceParamsVk::SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 binding, u32 startIndex)
{
	auto& writeSet = PrepareWriteSet(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, count, binding, startIndex);
	for (auto i = 0; i < count; ++i)
	{
		auto pResource = TO_VK(ppResources[i]);
		m_pSetData->m_AccelerationStructures.EmplaceBack(pResource->GetHandle());
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
		case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			auto& asWriteSet = m_pSetData->m_ASWriteSets.EmplaceBack();
			asWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			asWriteSet.pNext = nullptr;
			asWriteSet.accelerationStructureCount = writeSet.descriptorCount;
			asWriteSet.pAccelerationStructures = &m_pSetData->m_AccelerationStructures[index];
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
	case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
		m_pSetData->m_WriteSetDataIndices.EmplaceBack(m_pSetData->m_AccelerationStructures.Size());
		break;
	}

	return writeSet;
}


VkDescriptorType BvShaderResourceParamsVk::GetDescriptorType(u32 binding) const
{
	if (auto pResource = m_pLayout->GetResource(binding, m_Set))
	{
		return GetVkDescriptorType(pResource->m_ShaderResourceType);
	}

	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}