#include "BvDescriptorSetVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvShaderResourceVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"
#include "BvSamplerVk.h"
#include "BvUtilsVk.h"


BvResourceBindingStateVk::BvResourceBindingStateVk()
{
}


BvResourceBindingStateVk::BvResourceBindingStateVk(BvResourceBindingStateVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvResourceBindingStateVk& BvResourceBindingStateVk::operator=(BvResourceBindingStateVk&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_Bindings, rhs.m_Bindings);
	}

	return *this;
}


BvResourceBindingStateVk::~BvResourceBindingStateVk()
{
}


void BvResourceBindingStateVk::SetResource(VkDescriptorType descriptorType, const BvBufferViewVk* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& data = AddOrRetrieveResourceData(set, binding, arrayIndex);

	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		if (data.m_Data.m_BufferView != pResource->GetHandle())
		{
			data.m_Data.m_BufferView = pResource->GetHandle();
			data.m_DescriptorType = descriptorType;
		}
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	{
		auto& desc = pResource->GetDesc();
		auto pBuffer = static_cast<const BvBufferVk*>(desc.m_pBuffer);
		if (data.m_Data.m_BufferInfo.buffer != pBuffer->GetHandle()
			|| data.m_Data.m_BufferInfo.offset != desc.m_Offset
			|| data.m_Data.m_BufferInfo.range != desc.m_ElementCount * desc.m_Stride)
		{
			data.m_Data.m_BufferInfo.buffer = pBuffer->GetHandle();
			data.m_Data.m_BufferInfo.offset = desc.m_Offset;
			data.m_Data.m_BufferInfo.range = desc.m_ElementCount * desc.m_Stride;
			data.m_DescriptorType = descriptorType;
		}
		break;
	}
	default:
		BvAssert(nullptr, "Resource doesn't match binding's type");
		break;
	}
}



void BvResourceBindingStateVk::SetResource(VkDescriptorType descriptorType, const BvTextureViewVk* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& data = AddOrRetrieveResourceData(set, binding, arrayIndex);

	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		if (data.m_Data.m_ImageInfo.imageView != pResource->GetHandle())
		{
			data.m_Data.m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			data.m_Data.m_ImageInfo.imageView = pResource->GetHandle();
			data.m_Data.m_ImageInfo.sampler = VK_NULL_HANDLE;
			data.m_DescriptorType = descriptorType;
		}
		break;
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	{
		auto layout = !IsDepthStencilFormat(pResource->GetDesc().m_Format) ?
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		if (data.m_Data.m_ImageInfo.imageLayout != layout
			|| data.m_Data.m_ImageInfo.imageView != pResource->GetHandle())
		{
			data.m_Data.m_ImageInfo.imageLayout = layout;
			data.m_Data.m_ImageInfo.imageView = pResource->GetHandle();
			data.m_Data.m_ImageInfo.sampler = VK_NULL_HANDLE;
			data.m_DescriptorType = descriptorType;
		}
		break;
	}
	default:
		BvAssert(nullptr, "Resource doesn't match binding's type");
		break;
	}
}



void BvResourceBindingStateVk::SetResource(VkDescriptorType descriptorType, const BvSamplerVk* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& data = AddOrRetrieveResourceData(set, binding, arrayIndex);

	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLER:
		if (data.m_Data.m_ImageInfo.sampler != pResource->GetHandle())
		{
			data.m_Data.m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			data.m_Data.m_ImageInfo.imageView = VK_NULL_HANDLE;
			data.m_Data.m_ImageInfo.sampler = pResource->GetHandle();
			data.m_DescriptorType = descriptorType;
		}
		break;
	default:
		BvAssert(nullptr, "Resource doesn't match binding's type");
		break;
	}
}


void BvResourceBindingStateVk::Reset()
{
	m_Bindings.Clear();
	m_Resources.Clear();
}


const BvResourceBindingStateVk::ResourceData* BvResourceBindingStateVk::GetResource(const ResourceId& resId) const
{
	auto it = m_Bindings.FindKey(resId);
	return it != m_Bindings.cend() ? &m_Resources[it->second] : nullptr;
}


BvResourceBindingStateVk::ResourceData& BvResourceBindingStateVk::AddOrRetrieveResourceData(u32 set, u32 binding, u32 arrayIndex)
{
	ResourceId resId{ set, binding, arrayIndex };
	auto bindingIt = m_Bindings.Emplace(resId, 0);
	u32 index = 0;
	if (!bindingIt.second)
	{
		index = bindingIt.first->second;
	}
	else
	{
		m_Resources.EmplaceBack();
		index = static_cast<u32>(m_Resources.Size()) - 1;
	}

	return m_Resources[index];
}


BvDescriptorSetVk::BvDescriptorSetVk()
{
}


BvDescriptorSetVk::BvDescriptorSetVk(const BvRenderDeviceVk* pDevice, VkDescriptorSet descriptorSet)
	: m_pDevice(pDevice), m_DescriptorSet(descriptorSet)
{
}


BvDescriptorSetVk::BvDescriptorSetVk(BvDescriptorSetVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvDescriptorSetVk& BvDescriptorSetVk::operator=(BvDescriptorSetVk&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_pDevice = rhs.m_pDevice;
		std::swap(m_DescriptorSet, rhs.m_DescriptorSet);
	}

	return *this;
}


BvDescriptorSetVk::~BvDescriptorSetVk()
{
}


void BvDescriptorSetVk::Update(const BvVector<VkWriteDescriptorSet>& writeSets)
{
	//BvVector<VkWriteDescriptorSet> writeDescriptors;
	//auto& resources = descriptorData.GetBindings();
	//for (auto& resourcePair : resources)
	//{
	//	auto& resource = resourcePair.second;
	//	for (auto i = 0; i < resource.m_Count; ++i)
	//	{
	//		VkWriteDescriptorSet writeDescriptor{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	//		//writeDescriptor.pNext = nullptr;
	//		writeDescriptor.dstSet = m_DescriptorSet;
	//		writeDescriptor.dstBinding = resourcePair.first;
	//		writeDescriptor.dstArrayElement = i;
	//		writeDescriptor.descriptorCount = 1;
	//		writeDescriptor.descriptorType = resource.m_DescriptorType;
	//		auto pElement = i == 0 ? &resource.m_Element : &resource.m_Elements[i - 1];
	//		switch (resource.m_DescriptorType)
	//		{
	//		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	//		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	//			writeDescriptor.pBufferInfo = &pElement->bufferInfo;
	//			break;
	//		case VK_DESCRIPTOR_TYPE_SAMPLER:
	//		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	//		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	//			writeDescriptor.pImageInfo = &pElement->imageInfo;
	//			break;
	//		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	//		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
	//			writeDescriptor.pTexelBufferView = &pElement->bufferView;
	//			break;
	//		}
	//	}
	//}

	vkUpdateDescriptorSets(m_pDevice->GetHandle(), (u32)writeSets.Size(), writeSets.Data(), 0, nullptr);
}


BvDescriptorPoolVk::BvDescriptorPoolVk()
{
}


BvDescriptorPoolVk::BvDescriptorPoolVk(const BvRenderDeviceVk* pDevice, const BvShaderResourceLayoutVk* pLayout, u32 set, u32 maxAllocationsPerPool)
	: m_pDevice(pDevice), m_MaxAllocationsPerPool(maxAllocationsPerPool)
{
	BvRobinMap<VkDescriptorType, u32> poolSizes;
	auto it = pLayout->GetDesc().m_ShaderResources.FindKey(set);
	BvAssert(it != pLayout->GetDesc().m_ShaderResources.cend(), "Set not found in current Shader Resource Layout");
	auto& resources = it->second;
	m_Layout = pLayout->GetSetLayoutHandles().At(set);
	for (auto& resource : resources)
	{
		auto& poolSize = poolSizes[GetVkDescriptorType(resource.second.m_ShaderResourceType)];
		poolSize += resource.second.m_Count;
	}

	m_PoolSizes.Reserve(poolSizes.Size());
	for (auto& poolSize : poolSizes)
	{
		m_PoolSizes.PushBack({ poolSize.first, poolSize.second * m_MaxAllocationsPerPool });
	}
}


BvDescriptorPoolVk::BvDescriptorPoolVk(BvDescriptorPoolVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvDescriptorPoolVk& BvDescriptorPoolVk::operator=(BvDescriptorPoolVk&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_pDevice = rhs.m_pDevice;
		std::swap(m_Layout, rhs.m_Layout);
		std::swap(m_CurrPoolIndex, rhs.m_CurrPoolIndex);
		std::swap(m_MaxAllocationsPerPool, rhs.m_MaxAllocationsPerPool);
		std::swap(m_DescriptorPools, rhs.m_DescriptorPools);
		std::swap(m_PoolSizes, rhs.m_PoolSizes);
	}

	return *this;
}


BvDescriptorPoolVk::~BvDescriptorPoolVk()
{
	Destroy();
}


void BvDescriptorPoolVk::Create()
{
	VkDescriptorPoolCreateInfo poolCI{};
	//poolCI.pNext = nullptr;
	//poolCI.flags = 0;
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.maxSets = m_MaxAllocationsPerPool;
	poolCI.poolSizeCount = (u32)m_PoolSizes.Size();
	poolCI.pPoolSizes = m_PoolSizes.Data();

	auto& pool = m_DescriptorPools.EmplaceBack();
	auto result = vkCreateDescriptorPool(m_pDevice->GetHandle(), &poolCI, nullptr, &pool.pool);
	pool.currAllocationCount = 0;
}


void BvDescriptorPoolVk::Destroy()
{
	for (auto& pool : m_DescriptorPools)
	{
		vkDestroyDescriptorPool(m_pDevice->GetHandle(), pool.pool, nullptr);
	}

	m_DescriptorPools.Clear();
}


VkDescriptorSet BvDescriptorPoolVk::Allocate()
{
	if (m_CurrPoolIndex == m_DescriptorPools.Size())
	{
		Create();
	}

	if (m_DescriptorPools[m_CurrPoolIndex].currAllocationCount == m_MaxAllocationsPerPool)
	{
		if (++m_CurrPoolIndex == m_DescriptorPools.Size())
		{
			Create();
		}
	}

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	//allocateInfo.pNext = nullptr;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.descriptorPool = m_DescriptorPools[m_CurrPoolIndex].pool;
	allocateInfo.pSetLayouts = &m_Layout;

	VkDescriptorSet descriptorSet;
	auto result = vkAllocateDescriptorSets(m_pDevice->GetHandle(), &allocateInfo, &descriptorSet);

	++m_DescriptorPools[m_CurrPoolIndex].currAllocationCount;

	return descriptorSet;
}


void BvDescriptorPoolVk::Reset()
{
	for (auto& pool : m_DescriptorPools)
	{
		pool.currAllocationCount = 0;
		vkResetDescriptorPool(m_pDevice->GetHandle(), pool.pool, 0);
	}

	m_CurrPoolIndex = 0;
}