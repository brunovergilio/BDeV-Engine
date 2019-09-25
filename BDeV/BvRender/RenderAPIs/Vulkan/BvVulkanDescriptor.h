#pragma once


#include "BvRender/BvRenderDefines.h"


class BvVulkanDevice;


class BvVulkanDescriptorPool
{
public:
	BvVulkanDescriptorPool(const BvVulkanDevice * const pDevice);
	~BvVulkanDescriptorPool();

	void Create();
	void Destroy();

	void Allocate(const VkDescriptorSetLayout layout, VkDescriptorSet * const pSets, const uint32_t count = 1);
	void Free(const VkDescriptorSet * const pSets, const uint32_t count);

	BV_INLINE const VkDescriptorPool Get() const { return m_DescriptorPool; }

private:
	const BvVulkanDevice * const m_pDevice = nullptr;
	VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
};


class BvVulkanDescriptorSetLayout
{
public:
	BvVulkanDescriptorSetLayout(const BvVulkanDevice * const pDevice,
		const VkDescriptorSetLayoutBinding * const pBindings, const uint32_t count);
	~BvVulkanDescriptorSetLayout();

	void Create(const VkDescriptorSetLayoutCreateInfo & createInfo);
	void Destroy();

	const VkDescriptorSetLayout Get() const { return m_DescriptorSetLayout; }

private:
	const BvVulkanDevice * const m_pDevice = nullptr;
	VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
};


class BvVulkanDescriptorSet
{
public:
	BvVulkanDescriptorSet(const BvVulkanDevice * const pDevice, const VkDescriptorSet descriptorSet);
	~BvVulkanDescriptorSet();

	void Update(VkWriteDescriptorSet * const pWriteSets, const uint32_t count);

	const VkDescriptorSet Get() const { return m_DescriptorSet; }

private:
	const BvVulkanDevice * const m_pDevice = nullptr;
	VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
};