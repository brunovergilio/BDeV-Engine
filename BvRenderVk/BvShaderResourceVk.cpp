#include "BvShaderResourceVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvSamplerVk.h"
#include "BvDescriptorSetVk.h"
#include "BvBufferVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"
#include "BvAccelerationStructureVk.h"
#include "BvUtilsVk.h"
#include <utility>


BvShaderResourceLayoutVk::BvShaderResourceLayoutVk(BvRenderDeviceVk* pDevice, const ShaderResourceLayoutCreateDesc& srlDesc,
	BvVector<VkDescriptorSetLayout>& layouts, VkPipelineLayout pipelineLayout)
	: m_ShaderResourceLayoutDesc(srlDesc), m_pDevice(pDevice), m_Layouts(std::move(layouts)), m_PipelineLayout(pipelineLayout)
{
	//std::sort(m_ShaderResourceLayoutDesc.m_ShaderResourceSets.cbegin(), m_ShaderResourceLayoutDesc.m_ShaderResourceSets.cend());

	u32 currPushConstantOffset = 0;
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSets.Size(); ++i)
	{
		//std::sort(m_ShaderResourceLayoutDesc.m_ShaderResourceSets[i].m_Resources.cbegin(), m_ShaderResourceLayoutDesc.m_ShaderResourceSets[i].m_Resources.cend());
		auto& set = m_ShaderResourceLayoutDesc.m_ShaderResourceSets[i];
		for (auto j = 0u; j < set.m_Resources.Size(); ++j)
		{
			auto& resource = set.m_Resources[j];
			m_Resources.Emplace({ resource.m_Binding, set.m_Index }, &resource);
		}

		for (auto j = 0u; j < set.m_Constants.Size(); ++j)
		{
			auto& constant = set.m_Constants[j];
			m_PushConstantOffsets.Emplace({ constant.m_Size, constant.m_Binding, set.m_Index }, PushConstantData{ GetVkShaderStageFlags(constant.m_ShaderStages), currPushConstantOffset });
			currPushConstantOffset += constant.m_Size;
		}
	}
}


BvShaderResourceLayoutVk::~BvShaderResourceLayoutVk()
{
	Destroy();
}


const ShaderResourceSetDesc* BvShaderResourceLayoutVk::GetResourceSet(u32 set) const
{
	for (auto i = 0u; i < m_ShaderResourceLayoutDesc.m_ShaderResourceSets.Size(); ++i)
	{
		auto pSet = &m_ShaderResourceLayoutDesc.m_ShaderResourceSets[i];
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


void BvShaderResourceLayoutVk::Destroy()
{
	if (m_PipelineLayout)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_PipelineLayout);
		m_PipelineLayout = VK_NULL_HANDLE;

		for (auto layout : m_Layouts)
		{
			VkHelpers::DestroyDeviceObject(*m_pDevice, layout);
		}
		m_Layouts.Clear();
	}
}


BvShaderResourceParamsVk::BvShaderResourceParamsVk(BvRenderDeviceVk* pDevice, const BvShaderResourceLayoutVk* pLayout, u32 set)
	: m_pDevice(pDevice), m_pLayout(pLayout), m_Set(set), m_DescriptorPool(pDevice, pLayout, set, 1), m_DescriptorSet(m_DescriptorPool.Allocate())
{
	auto pSet = m_pLayout->GetResourceSet(set);
	BV_ASSERT(pSet != nullptr, "Set not found in current Shader Resource Layout");
	u32 bufferInfoCount = 0;
	u32 imageInfoCount = 0;
	u32 bufferViewCount = 0;
	u32 asCount = 0;

	for (auto i = 0u; i < pSet->m_Resources.Size(); ++i)
	{
		auto& res = pSet->m_Resources[i];
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
		case ShaderResourceType::kAccelerationStructure:
			asCount += res.m_Count;
		}
	}

	m_pSetData = BV_NEW(SetData);
	m_pSetData->m_BufferInfos.Reserve(bufferInfoCount);
	m_pSetData->m_ImageInfos.Reserve(imageInfoCount);
	m_pSetData->m_BufferViews.Reserve(bufferViewCount);
	m_pSetData->m_AccelerationStructures.Reserve(asCount);
	m_pSetData->m_ASWriteSets.Reserve(asCount);
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

	vkUpdateDescriptorSets(*m_pDevice, m_pSetData->m_WriteSets.Size(), m_pSetData->m_WriteSets.Data(), 0, nullptr);
	m_pSetData->m_WriteSets.Clear();
	m_pSetData->m_WriteSetDataIndices.Clear();
}


VkWriteDescriptorSet& BvShaderResourceParamsVk::PrepareWriteSet(VkDescriptorType descriptorType, u32 count, u32 binding, u32 startIndex)
{
	m_pSetData->m_WriteSets.PushBack({ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
	auto& writeSet = m_pSetData->m_WriteSets.Back();
	writeSet.dstSet = m_DescriptorSet;
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