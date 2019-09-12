#include "BvVulkanDescriptor.h"
#include "BvRender/RenderAPIs/Vulkan/BvVulkanDevice.h"
#include "BvCore/Utils/BvUtils.h"


BvVulkanDescriptorPool::BvVulkanDescriptorPool(const BvVulkanDevice * const pDevice)
	: m_pDevice(pDevice)
{
	Create();
}


BvVulkanDescriptorPool::~BvVulkanDescriptorPool()
{
	Destroy();
}


void BvVulkanDescriptorPool::Create()
{
	constexpr uint32_t kDefaultPoolSizeCount = 1024;
	constexpr uint32_t kDefaultMaxSets = 1024;

	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, kDefaultPoolSizeCount },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, kDefaultPoolSizeCount },
	};

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = BvArraySize(poolSizes);
	createInfo.pPoolSizes = poolSizes;
	createInfo.maxSets = kDefaultMaxSets;
	vkCreateDescriptorPool(m_pDevice->GetLogical(), &createInfo, nullptr, &m_DescriptorPool);
}


void BvVulkanDescriptorPool::Destroy()
{
	if (m_DescriptorPool)
	{
		vkDestroyDescriptorPool(m_pDevice->GetLogical(), m_DescriptorPool, nullptr);
		m_DescriptorPool = VK_NULL_HANDLE;
	}
}


void BvVulkanDescriptorPool::Allocate(const VkDescriptorSetLayout layout, VkDescriptorSet * const pSets, const uint32_t count)
{
	constexpr uint32_t kMaxDescriptorSets = 16;

	BvAssert(count <= kMaxDescriptorSets);

	VkDescriptorSet sets[kMaxDescriptorSets]{};
	VkDescriptorSetLayout layouts[kMaxDescriptorSets]{};

	for (auto i = 0u; i < count; i++)
	{
		layouts[i] = layout;
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = layouts;

	vkAllocateDescriptorSets(m_pDevice->GetLogical(), &allocInfo, sets);

	for (auto i = 0u; i < count; i++)
	{
		pSets[i] = sets[i];
	}
}


void BvVulkanDescriptorPool::Free(const VkDescriptorSet * const pSets, const uint32_t count)
{
	vkFreeDescriptorSets(m_pDevice->GetLogical(), m_DescriptorPool, count, pSets);
}


BvVulkanDescriptorSetLayout::BvVulkanDescriptorSetLayout(const BvVulkanDevice * const pDevice,
	const VkDescriptorSetLayoutBinding * const pBindings, const uint32_t count)
	: m_pDevice(pDevice)
{
	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = count;
	createInfo.pBindings = pBindings;

	Create(createInfo);
}


BvVulkanDescriptorSetLayout::~BvVulkanDescriptorSetLayout()
{
	Destroy();
}


void BvVulkanDescriptorSetLayout::Create(const VkDescriptorSetLayoutCreateInfo & createInfo)
{
	vkCreateDescriptorSetLayout(m_pDevice->GetLogical(), &createInfo, nullptr, &m_DescriptorSetLayout);
}


void BvVulkanDescriptorSetLayout::Destroy()
{
	if (m_DescriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(m_pDevice->GetLogical(), m_DescriptorSetLayout, nullptr);
		m_DescriptorSetLayout = VK_NULL_HANDLE;
	}
}


BvVulkanDescriptorSet::BvVulkanDescriptorSet(const BvVulkanDevice * const pDevice, const VkDescriptorSet descriptorSet)
	: m_pDevice(pDevice), m_DescriptorSet(descriptorSet)
{
}


BvVulkanDescriptorSet::~BvVulkanDescriptorSet()
{
}


void BvVulkanDescriptorSet::Update(VkWriteDescriptorSet * const pWriteSets, const uint32_t count)
{
	for (auto i = 0U; i < count; i++)
	{
		pWriteSets[i].dstSet = m_DescriptorSet;
	}

	vkUpdateDescriptorSets(m_pDevice->GetLogical(), count, pWriteSets, 0, nullptr);
}