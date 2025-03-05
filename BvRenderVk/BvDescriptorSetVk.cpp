#include "BvDescriptorSetVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvShaderResourceVk.h"
#include "BvBufferViewVk.h"
#include "BvTextureViewVk.h"
#include "BvSamplerVk.h"
#include "BvAccelerationStructureVk.h"
#include "BvUtilsVk.h"


void ResourceDataVk::Set(VkDescriptorType descriptorType, const IBvBufferViewVk* pResource)
{
	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	{
		auto& desc = pResource->GetDesc();
		auto pBuffer = static_cast<const BvBufferVk*>(desc.m_pBuffer);
		
		m_Data.m_BufferInfo.buffer = pBuffer->GetHandle();
		m_Data.m_BufferInfo.offset = desc.m_Offset;
		m_Data.m_BufferInfo.range = desc.m_ElementCount * desc.m_Stride;
		m_DescriptorType = descriptorType;
		break;
	}
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		m_Data.m_BufferView = pResource->GetHandle();
		m_DescriptorType = descriptorType;
		break;
	default:
		BV_ASSERT(nullptr, "Resource doesn't match binding's type");
		break;
	}
}


void ResourceDataVk::Set(VkDescriptorType descriptorType, const IBvTextureViewVk* pResource)
{
	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	{
		auto layout = !IsDepthStencilFormat(pResource->GetDesc().m_Format) ?
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		m_Data.m_ImageInfo.imageLayout = layout;
		m_Data.m_ImageInfo.imageView = pResource->GetHandle();
		m_Data.m_ImageInfo.sampler = VK_NULL_HANDLE;
		m_DescriptorType = descriptorType;
		break;
	}
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	{
		m_Data.m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		m_Data.m_ImageInfo.imageView = pResource->GetHandle();
		m_Data.m_ImageInfo.sampler = VK_NULL_HANDLE;
		m_DescriptorType = descriptorType;
		break;
	}
	default:
		BV_ASSERT(nullptr, "Resource doesn't match binding's type");
		break;
	}
}


void ResourceDataVk::Set(VkDescriptorType descriptorType, const IBvSamplerVk* pResource)
{
	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLER:
		m_Data.m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_Data.m_ImageInfo.imageView = VK_NULL_HANDLE;
		m_Data.m_ImageInfo.sampler = pResource->GetHandle();
		m_DescriptorType = descriptorType;
		break;
	default:
		BV_ASSERT(nullptr, "Resource doesn't match binding's type");
		break;
	}
}


void ResourceDataVk::Set(VkDescriptorType descriptorType, const IBvAccelerationStructureVk* pResource)
{
	switch (descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
		m_Data.m_AccelerationStructure = pResource->GetHandle();
		m_DescriptorType = descriptorType;
		break;
	default:
		BV_ASSERT(nullptr, "Resource doesn't match binding's type");
		break;
	}
}


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
		std::swap(m_Resources, rhs.m_Resources);
	}

	return *this;
}


BvResourceBindingStateVk::~BvResourceBindingStateVk()
{
}


void BvResourceBindingStateVk::SetResource(VkDescriptorType descriptorType, const IBvBufferViewVk* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& data = AddOrRetrieveResourceData(set, binding, arrayIndex);
	data.Set(descriptorType, pResource);
}



void BvResourceBindingStateVk::SetResource(VkDescriptorType descriptorType, const IBvTextureViewVk* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& data = AddOrRetrieveResourceData(set, binding, arrayIndex);
	data.Set(descriptorType, pResource);
}



void BvResourceBindingStateVk::SetResource(VkDescriptorType descriptorType, const IBvSamplerVk* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& data = AddOrRetrieveResourceData(set, binding, arrayIndex);
	data.Set(descriptorType, pResource);
}


void BvResourceBindingStateVk::SetResource(VkDescriptorType descriptorType, const IBvAccelerationStructureVk* pResource, u32 set, u32 binding, u32 arrayIndex)
{
	auto& data = AddOrRetrieveResourceData(set, binding, arrayIndex);
	data.Set(descriptorType, pResource);
}


void BvResourceBindingStateVk::Reset()
{
	m_Bindings.Clear();
	m_Resources.Clear();
}


const ResourceDataVk* BvResourceBindingStateVk::GetResource(const ResourceIdVk& resId) const
{
	auto it = m_Bindings.FindKey(resId);
	return it != m_Bindings.cend() ? &m_Resources[it->second] : nullptr;
}


ResourceDataVk& BvResourceBindingStateVk::AddOrRetrieveResourceData(u32 set, u32 binding, u32 arrayIndex)
{
	ResourceIdVk resId{ set, binding, arrayIndex };
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
		bindingIt.first->second = index;
	}
	m_DirtySets[set] = true;

	return m_Resources[index];
}


BvDescriptorSetVk::BvDescriptorSetVk()
{
}


BvDescriptorSetVk::BvDescriptorSetVk(const IBvRenderDeviceVk* pDevice, VkDescriptorSet descriptorSet)
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
	vkUpdateDescriptorSets(m_pDevice->GetHandle(), (u32)writeSets.Size(), writeSets.Data(), 0, nullptr);
}


BvDescriptorPoolVk::BvDescriptorPoolVk()
{
}


BvDescriptorPoolVk::BvDescriptorPoolVk(const IBvRenderDeviceVk* pDevice, const IBvShaderResourceLayoutVk* pLayout, u32 set, u32 maxAllocationsPerPool)
	: m_pDevice(pDevice), m_MaxAllocationsPerPool(maxAllocationsPerPool), m_pLayout(pLayout), m_SetIndex(set)
{
	BvRobinMap<VkDescriptorType, u32> poolSizes;
	auto it = m_pLayout->GetResources().FindKey(m_SetIndex);
	BV_ASSERT(it != m_pLayout->GetResources().cend(), "Set not found in current Shader Resource Layout");
	auto& resources = it->second;
	m_Layout = m_pLayout->GetSetLayoutHandles().At(m_SetIndex);
	for (auto& resource : resources)
	{
		auto& poolSize = poolSizes[GetVkDescriptorType(resource.second->m_ShaderResourceType)];
		poolSize += resource.second->m_Count;
	}

	m_PoolSizes.Reserve(poolSizes.Size());
	for (auto& poolSize : poolSizes)
	{
		m_PoolSizes.PushBack({ poolSize.first, poolSize.second * m_MaxAllocationsPerPool });
	}

	if (it != m_pLayout->GetResources().cend())
	{
		auto& map = it->second;
		for (auto& res : map)
		{
			if (res.second->m_Bindless)
			{
				m_IsBindless = true;
				break;
			}
		}
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
	VkDescriptorPoolCreateInfo poolCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	//poolCI.pNext = nullptr;
	poolCI.flags = m_IsBindless ? VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT : 0;
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