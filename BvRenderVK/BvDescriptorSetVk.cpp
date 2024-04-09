#include "BvDescriptorSetVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvShaderResourceVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"
#include "BvSamplerVk.h"
#include "BvUtilsVk.h"


BvResourceBindingState::BvResourceBindingState(u32 set)
{
}


BvResourceBindingState::BvResourceBindingState(BvResourceBindingState&& rhs)
	: m_Bindings(std::move(rhs.m_Bindings)), m_IsDirty(rhs.m_IsDirty)
{
}


BvResourceBindingState::~BvResourceBindingState()
{
}


void BvResourceBindingState::SetResource(const BvBufferViewVk& resource, u32 binding, u32 arrayIndex /*= 0*/)
{
	auto it = m_Bindings.FindKey(binding);
	if (it != m_Bindings.cend())
	{
		auto& data = it->second;
		if (arrayIndex >= data.m_Count)
		{
			BvAssert(arrayIndex >= data.m_Count, "Trying to set resource in array with index out of bounds");
			return;
		}

		auto pElement = arrayIndex == 0 ? &data.m_Element : &data.m_Elements[arrayIndex - 1];
		switch (data.m_DescriptorType)
		{
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			if (pElement->bufferView != resource.GetHandle())
			{
				pElement->bufferView = resource.GetHandle();

				m_IsDirty = true;
			}
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			auto& desc = resource.GetDesc();
			auto pBuffer = static_cast<const BvBufferVk*>(desc.m_pBuffer);
			if (pElement->bufferInfo.buffer != pBuffer->GetHandle()
				|| pElement->bufferInfo.offset != desc.m_Offset
				|| pElement->bufferInfo.range != desc.m_ElementCount * desc.m_Stride)
			{
				pElement->bufferInfo.buffer = pBuffer->GetHandle();
				pElement->bufferInfo.offset = desc.m_Offset;
				pElement->bufferInfo.range = desc.m_ElementCount * desc.m_Stride;

				m_IsDirty = true;
			}
		default:
			BvAssert(nullptr, "Resource doesn't match binding's type");
			break;
		}
	}
}



void BvResourceBindingState::SetResource(const BvTextureViewVk& resource, u32 binding, u32 arrayIndex /*= 0*/)
{
	auto it = m_Bindings.FindKey(binding);
	if (it != m_Bindings.cend())
	{
		auto& data = it->second;
		if (arrayIndex >= data.m_Count)
		{
			BvAssert(arrayIndex >= data.m_Count, "Trying to set resource in array with index out of bounds");
			return;
		}

		auto pElement = arrayIndex == 0 ? &data.m_Element : &data.m_Elements[arrayIndex - 1];
		switch (data.m_DescriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			if (pElement->imageInfo.imageView != resource.GetHandle())
			{
				pElement->imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				pElement->imageInfo.imageView = resource.GetHandle();
				pElement->imageInfo.sampler = VK_NULL_HANDLE;

				m_IsDirty = true;
			}
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			auto layout = !IsDepthStencilFormat(resource.GetDesc().m_Format) ?
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			if (pElement->imageInfo.imageLayout != layout
				|| pElement->imageInfo.imageView != resource.GetHandle())
			{
				pElement->imageInfo.imageLayout = layout;
				pElement->imageInfo.imageView = resource.GetHandle();
				pElement->imageInfo.sampler = VK_NULL_HANDLE;

				m_IsDirty = true;
			}
			break;
		default:
			BvAssert(nullptr, "Resource doesn't match binding's type");
			break;
		}
	}
}



void BvResourceBindingState::SetResource(const BvSamplerVk& resource, u32 binding, u32 arrayIndex /*= 0*/)
{
	auto it = m_Bindings.FindKey(binding);
	if (it != m_Bindings.cend())
	{
		auto& data = it->second;
		if (arrayIndex >= data.m_Count)
		{
			BvAssert(arrayIndex >= data.m_Count, "Trying to set resource in array with index out of bounds");
			return;
		}

		auto pElement = arrayIndex == 0 ? &data.m_Element : &data.m_Elements[arrayIndex - 1];
		switch (data.m_DescriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			if (pElement->imageInfo.sampler != resource.GetHandle())
			{
				pElement->imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				pElement->imageInfo.imageView = VK_NULL_HANDLE;
				pElement->imageInfo.sampler = resource.GetHandle();

				m_IsDirty = true;
			}
			break;
		default:
			BvAssert(nullptr, "Resource doesn't match binding's type");
			break;
		}
	}
}


BvDescriptorSetVk::BvDescriptorSetVk(const BvRenderDeviceVk& device, VkDescriptorSet descriptorSet)
	: m_Device(device), m_DescriptorSet(descriptorSet)
{
}


BvDescriptorSetVk::BvDescriptorSetVk(BvDescriptorSetVk&& rhs)
	: m_Device(rhs.m_Device), m_DescriptorSet(rhs.m_DescriptorSet)
{
}


BvDescriptorSetVk::~BvDescriptorSetVk()
{

}


void BvDescriptorSetVk::Update(const BvResourceBindingState& descriptorData)
{
	BvVector<VkWriteDescriptorSet> writeDescriptors;
	auto& resources = descriptorData.GetBindings();
	for (auto& resourcePair : resources)
	{
		auto& resource = resourcePair.second;
		for (auto i = 0; i < resource.m_Count; ++i)
		{
			VkWriteDescriptorSet writeDescriptor{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			//writeDescriptor.pNext = nullptr;
			writeDescriptor.dstSet = m_DescriptorSet;
			writeDescriptor.dstBinding = resourcePair.first;
			writeDescriptor.dstArrayElement = i;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.descriptorType = resource.m_DescriptorType;
			auto pElement = i == 0 ? &resource.m_Element : &resource.m_Elements[i - 1];
			switch (resource.m_DescriptorType)
			{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				writeDescriptor.pBufferInfo = &pElement->bufferInfo;
				break;
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				writeDescriptor.pImageInfo = &pElement->imageInfo;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				writeDescriptor.pTexelBufferView = &pElement->bufferView;
				break;
			}
		}
	}

	vkUpdateDescriptorSets(m_Device.GetHandle(), (u32)writeDescriptors.Size(), writeDescriptors.Data(), 0, nullptr);
}


BvDescriptorPoolVk::BvDescriptorPoolVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, u32 set, u32 maxAllocationsPerPool)
	: m_Device(device), m_MaxAllocationsPerPool(maxAllocationsPerPool)
{
	BvRobinMap<VkDescriptorType, u32> poolSizes;
	auto it = layout.GetDesc().m_ShaderResources.FindKey(set);
	BvAssert(it != layout.GetDesc().m_ShaderResources.cend(), "Set not found in current Shader Resource Layout");
	auto& resources = it->second;
	m_Layout = layout.GetSetLayoutHandles().At(set);
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


BvDescriptorPoolVk::BvDescriptorPoolVk(BvDescriptorPoolVk&& rhs)
	: m_Device(rhs.m_Device), m_Layout(rhs.m_Layout), m_CurrPoolIndex(rhs.m_CurrPoolIndex), m_MaxAllocationsPerPool(rhs.m_MaxAllocationsPerPool),
	m_DescriptorPools(std::move(rhs.m_DescriptorPools)), m_PoolSizes(std::move(rhs.m_PoolSizes))
{
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
	auto result = vkCreateDescriptorPool(m_Device.GetHandle(), &poolCI, nullptr, &pool.pool);
}


void BvDescriptorPoolVk::Destroy()
{
	for (auto& pool : m_DescriptorPools)
	{
		vkDestroyDescriptorPool(m_Device.GetHandle(), pool.pool, nullptr);
	}

	m_DescriptorPools.Clear();
}


VkDescriptorSet BvDescriptorPoolVk::Allocate()
{
	u32 poolIndex = -1;
	for (auto i = m_CurrPoolIndex; i < m_DescriptorPools.Size(); ++i)
	{
		if (m_DescriptorPools[i].currAllocationCount < m_MaxAllocationsPerPool)
		{
			poolIndex = i;
			break;
		}
	}

	if (poolIndex == -1)
	{
		Create();
		poolIndex = m_CurrPoolIndex++;
	}

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	//allocateInfo.pNext = nullptr;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.descriptorPool = m_DescriptorPools[poolIndex].pool;
	allocateInfo.pSetLayouts = &m_Layout;

	VkDescriptorSet descriptorSet;
	auto result = vkAllocateDescriptorSets(m_Device.GetHandle(), &allocateInfo, &descriptorSet);
}


void BvDescriptorPoolVk::Reset()
{
	for (auto& pool : m_DescriptorPools)
	{
		pool.currAllocationCount = 0;
		vkResetDescriptorPool(m_Device.GetHandle(), pool.pool, 0);
	}

	m_CurrPoolIndex = 0;
}