#include "BvShaderResourceSetPoolVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvShaderResourceVk.h"


const u32 kMmaxDescriptorSetsPerAllocation = 16;


BvShaderResourceSetPoolVk::BvShaderResourceSetPoolVk(const BvRenderDeviceVk & device, const ShaderResourceSetPoolDesc & desc)
	: BvShaderResourceSetPool(desc), m_Device(device)
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
	if (desc.m_ConstantBufferCount > 0)		{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kConstantBuffer)		, desc.m_ConstantBufferCount	});	 }
	if (desc.m_StructuredBufferCount > 0)	{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kStructuredBuffer)	, desc.m_StructuredBufferCount	});	 }
	if (desc.m_RWStructuredBufferCount > 0)	{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kRWStructuredBuffer)	, desc.m_RWStructuredBufferCount});	 }
	if (desc.m_FormattedBufferCount > 0)	{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kFormattedBuffer)	, desc.m_FormattedBufferCount	});	 }
	if (desc.m_RWFormattedBufferCount > 0)	{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kRWFormattedBuffer)	, desc.m_RWFormattedBufferCount });	 }
	if (desc.m_TextureCount > 0)			{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kTexture)			, desc.m_TextureCount			});	 }
	if (desc.m_RWTextureCount > 0)			{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kRWTexture)			, desc.m_RWTextureCount			});	 }
	if (desc.m_SamplerCount > 0)			{ poolSizes.PushBack({ GetVkDescriptorType(ShaderResourceType::kSampler)			, desc.m_SamplerCount			});	 }

	u32 maxSets = 0;
	for (auto && poolSize : poolSizes)
	{
		maxSets += poolSize.descriptorCount;
	}

	VkDescriptorPoolCreateInfo poolCI{};
	//poolCI.pNext = nullptr;
	poolCI.flags = GetVkDescriptorPoolCreateFlags(m_ShaderResourceSetPoolDesc.m_Flags);
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.maxSets = maxSets;
	poolCI.poolSizeCount = (u32)poolSizes.Size();
	poolCI.pPoolSizes = poolSizes.Data();

	auto result = m_Device.GetDeviceFunctions().vkCreateDescriptorPool(m_Device.GetHandle(), &poolCI, nullptr, &m_DescriptorPool);
}


void BvShaderResourceSetPoolVk::Destroy()
{
	for (auto&& pLayout : m_UsedSets)
	{
		for (auto&& pSet : pLayout.second)
		{
			delete pSet;
		}
		pLayout.second.Clear();
	}
	m_UsedSets.Clear();

	for (auto&& pLayout : m_FreeSets)
	{
		for (auto&& pSet : pLayout.second)
		{
			delete pSet;
		}
		pLayout.second.Clear();
	}
	m_FreeSets.Clear();

	if (m_DescriptorPool)
	{
		m_Device.GetDeviceFunctions().vkDestroyDescriptorPool(m_Device.GetHandle(), m_DescriptorPool, nullptr);
		m_DescriptorPool = VK_NULL_HANDLE;
	}
}


void BvShaderResourceSetPoolVk::AllocateSets(u32 count, BvShaderResourceSet ** ppSets,
	const BvShaderResourceLayout * const pLayout, u32 set /*= 0*/)
{
	// Check for free, recycled sets
	if ((m_ShaderResourceSetPoolDesc.m_Flags & ShaderResourcePoolFlags::kRecycleDescriptors) == ShaderResourcePoolFlags::kRecycleDescriptors)
	{
		u32 i = 0;
		auto& freeSets = m_FreeSets[pLayout];
		auto freeSetCount = freeSets.Size();
		for (; i < count && i < freeSetCount; i++)
		{
			ppSets[i] = freeSets.Back();
			freeSets.PopBack();
		}

		if (i > 0)
		{
			count -= i;
			ppSets += i;

			if (count == 0)
			{
				return;
			}
		}
	}

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
		BvAssert(pIter != pLayouts.cend(), "Invalid set");
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
	allocateInfo.pSetLayouts = descriptorSetLayouts;

	auto result = m_Device.GetDeviceFunctions().vkAllocateDescriptorSets(m_Device.GetHandle(), &allocateInfo, descriptorSets);
	for (u32 i = 0; i < allocateInfo.descriptorSetCount; i++)
	{
		auto pSet = new BvShaderResourceSetVk(m_Device, pLayoutVk, descriptorSets[i], this, set);
		m_UsedSets[pLayout].EmplaceBack(pSet);
		ppSets[i] = pSet;
	}
}


void BvShaderResourceSetPoolVk::FreeSets(u32 setCount, BvShaderResourceSet ** ppSets)
{
	if ((m_ShaderResourceSetPoolDesc.m_Flags & ShaderResourcePoolFlags::kRecycleDescriptors) == ShaderResourcePoolFlags::kRecycleDescriptors)
	{
		for (auto i = 0u; i < setCount; i++)
		{
			auto pSetVk = reinterpret_cast<BvShaderResourceSetVk*>(ppSets[i]);
			auto& usedSets = m_UsedSets[pSetVk->GetLayout()];
			auto& freeSets = m_FreeSets[pSetVk->GetLayout()];
			for (auto j = 0u; j < usedSets.Size(); j++)
			{
				if (usedSets[j] == pSetVk)
				{
					if (j != usedSets.Size() - 1)
					{
						std::swap(usedSets[j], usedSets[usedSets.Size() - 1]);
					}
					usedSets.PopBack();
					freeSets.EmplaceBack(pSetVk);
					break;
				}
			}
		}
	}
	else
	{
		if (setCount > kMmaxDescriptorSetsPerAllocation)
		{
			FreeSets(setCount - kMmaxDescriptorSetsPerAllocation, ppSets + kMmaxDescriptorSetsPerAllocation);
		}

		VkDescriptorSet descriptorSets[kMmaxDescriptorSetsPerAllocation];
		u32 count = setCount > kMmaxDescriptorSetsPerAllocation ? kMmaxDescriptorSetsPerAllocation : setCount;
		for (auto i = 0u; i < count; i++)
		{
			auto pDsVk = reinterpret_cast<BvShaderResourceSetVk*>(ppSets[i]);
			auto& usedSets = m_UsedSets[pDsVk->GetLayout()];
			descriptorSets[i] = pDsVk->GetHandle();

			for (auto j = 0u; j < usedSets.Size(); j++)
			{
				if (usedSets[j] == pDsVk)
				{
					if (j != usedSets.Size() - 1)
					{
						std::swap(usedSets[j], usedSets[usedSets.Size() - 1]);
					}
					usedSets.PopBack();
					delete pDsVk;
					break;
				}
			}
		}

		m_Device.GetDeviceFunctions().vkFreeDescriptorSets(m_Device.GetHandle(), m_DescriptorPool, count, descriptorSets);
	}
}