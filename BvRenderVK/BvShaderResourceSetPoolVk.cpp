#include "BvShaderResourceSetPoolVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvShaderResourceVk.h"


const u32 kMmaxDescriptorSetsPerAllocation = 16;


BvShaderResourceSetPoolVk::BvShaderResourceSetPoolVk(const BvRenderDeviceVk & device, const ShaderResourceSetPoolDesc & desc)
	: m_Device(device)
{
	Create(desc);
}


BvShaderResourceSetPoolVk::~BvShaderResourceSetPoolVk()
{
	Destroy();
}


void BvShaderResourceSetPoolVk::Create(const ShaderResourceSetPoolDesc & desc)
{
	constexpr auto kMaxPoolSizes = 8u;
	BvFixedVector<VkDescriptorPoolSize, kMaxPoolSizes> poolSizes;
	poolSizes[0].type = GetVkDescriptorType(ShaderResourceType::kConstantBuffer);
	poolSizes[1].type = GetVkDescriptorType(ShaderResourceType::kStructuredBuffer);
	poolSizes[2].type = GetVkDescriptorType(ShaderResourceType::kRWStructuredBuffer);
	poolSizes[3].type = GetVkDescriptorType(ShaderResourceType::kFormattedBuffer);
	poolSizes[4].type = GetVkDescriptorType(ShaderResourceType::kRWFormattedBuffer);
	poolSizes[5].type = GetVkDescriptorType(ShaderResourceType::kTexture);
	poolSizes[6].type = GetVkDescriptorType(ShaderResourceType::kRWTexture);
	poolSizes[7].type = GetVkDescriptorType(ShaderResourceType::kSampler);

	poolSizes[0].descriptorCount = desc.m_ConstantBufferCount;
	poolSizes[1].descriptorCount = desc.m_StructuredBufferCount;
	poolSizes[2].descriptorCount = desc.m_RWStructuredBufferCount;
	poolSizes[3].descriptorCount = desc.m_FormattedBufferCount;
	poolSizes[4].descriptorCount = desc.m_RWFormattedBufferCount;
	poolSizes[5].descriptorCount = desc.m_TextureCount;
	poolSizes[6].descriptorCount = desc.m_RWTextureCount;
	poolSizes[7].descriptorCount = desc.m_SamplerCount;

	u32 maxSets = 0;
	for (auto && poolSize : poolSizes)
	{
		maxSets += poolSize.descriptorCount;
	}

	VkDescriptorPoolCreateInfo poolCI{};
	//poolCI.flags = 0;
	//poolCI.pNext = nullptr;
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.maxSets = maxSets;
	poolCI.poolSizeCount = (u32)poolSizes.Size();
	poolCI.pPoolSizes = poolSizes.Data();

	auto result = m_Device.GetDeviceFunctions().vkCreateDescriptorPool(m_Device.GetHandle(), &poolCI, nullptr, &m_DescriptorPool);
}


void BvShaderResourceSetPoolVk::Destroy()
{
	for (auto&& pSet : m_Sets)
	{
		BvDelete(pSet);
	}

	if (m_DescriptorPool)
	{
		m_Device.GetDeviceFunctions().vkDestroyDescriptorPool(m_Device.GetHandle(), m_DescriptorPool, nullptr);
		m_DescriptorPool = VK_NULL_HANDLE;
	}
}


void BvShaderResourceSetPoolVk::AllocateSets(const u32 count, BvShaderResourceSet ** ppSets,
	const BvShaderResourceLayout * const pLayout, const u32 set /*= 0*/)
{
	if (count > kMmaxDescriptorSetsPerAllocation)
	{
		AllocateSets(count - kMmaxDescriptorSetsPerAllocation, ppSets + kMmaxDescriptorSetsPerAllocation, pLayout, set);
	}
	VkDescriptorSet descriptorSets[kMmaxDescriptorSetsPerAllocation];
	VkDescriptorSetLayout descriptorSetLayouts[kMmaxDescriptorSetsPerAllocation];

	VkDescriptorSetLayout layoutVk = VK_NULL_HANDLE;
	auto pLayoutVk = static_cast<const BvShaderResourceLayoutVk * const>(pLayout);
	{
		auto & pLayouts = pLayoutVk->GetSetLayoutHandles();
		auto pIter = pLayouts.FindKey(set);
		BvAssert(pIter != pLayouts.cend());
		layoutVk = pIter->second;
	}

	auto descriptorCount = count > kMmaxDescriptorSetsPerAllocation ? kMmaxDescriptorSetsPerAllocation : count;
	for (auto i = 0u; i < count; i++)
	{
		descriptorSetLayouts[i] = layoutVk;
	}


	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	//allocateInfo.pNext = nullptr;
	allocateInfo.descriptorSetCount = descriptorCount;
	allocateInfo.descriptorPool = m_DescriptorPool;
	allocateInfo.pSetLayouts;

	auto result = m_Device.GetDeviceFunctions().vkAllocateDescriptorSets(m_Device.GetHandle(), &allocateInfo, descriptorSets);
	for (u32 i = 0; i < allocateInfo.descriptorSetCount; i++)
	{
		auto pSet = new BvShaderResourceSetVk(m_Device, pLayoutVk, descriptorSets[i], this, set);
		m_Sets.EmplaceBack(pSet);
		ppSets[i] = pSet;
	}
}


void BvShaderResourceSetPoolVk::FreeSets(const u32 setCount, BvShaderResourceSet ** ppSets)
{
	if (setCount > kMmaxDescriptorSetsPerAllocation)
	{
		FreeSets(setCount - kMmaxDescriptorSetsPerAllocation, ppSets + kMmaxDescriptorSetsPerAllocation);
	}

	VkDescriptorSet descriptorSets[kMmaxDescriptorSetsPerAllocation];
	u32 count = setCount > kMmaxDescriptorSetsPerAllocation ? kMmaxDescriptorSetsPerAllocation : setCount;
	for (auto i = 0u; i < count; i++)
	{
		auto pDsVk = reinterpret_cast<BvShaderResourceSetVk *>(ppSets[i]);
		descriptorSets[i] = pDsVk->GetHandle();

		for (auto j = 0u; j < m_Sets.Size(); j++)
		{
			if (m_Sets[j] == pDsVk)
			{
				if (j != m_Sets.Size() - 1)
				{
					std::swap(m_Sets[j], m_Sets[m_Sets.Size() - 1]);
				}
				m_Sets.PopBack();
				BvDelete(pDsVk);
				break;
			}
		}
	}

	// Maybe shouldn't be freeing them, will cause fragmentation
	m_Device.GetDeviceFunctions().vkFreeDescriptorSets(m_Device.GetHandle(), m_DescriptorPool, count, descriptorSets);
}