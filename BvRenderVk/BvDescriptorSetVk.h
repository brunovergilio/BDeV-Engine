#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderResource.h"


class BvRenderDeviceVk;
class BvShaderResourceLayoutVk;
class BvBufferViewVk;
class BvTextureViewVk;
class BvSamplerVk;


struct ResourceIdVk
{
	u32 m_Set;
	u32 m_Binding;
	u32 m_ArrayIndex;

	friend bool operator==(const ResourceIdVk& lhs, const ResourceIdVk& rhs)
	{
		return lhs.m_Set == rhs.m_Set && lhs.m_Binding == rhs.m_Binding && lhs.m_ArrayIndex == rhs.m_ArrayIndex;
	}
};

struct ResourceDataVk
{
	union Data
	{
		VkDescriptorBufferInfo m_BufferInfo;
		VkDescriptorImageInfo m_ImageInfo;
		VkBufferView m_BufferView;
	};

	Data m_Data;
	VkDescriptorType m_DescriptorType;

	void Set(VkDescriptorType descriptorType, const BvBufferViewVk* pResource);
	void Set(VkDescriptorType descriptorType, const BvTextureViewVk* pResource);
	void Set(VkDescriptorType descriptorType, const BvSamplerVk* pResource);
};


class BvResourceBindingStateVk final
{
public:

	BvResourceBindingStateVk();
	BvResourceBindingStateVk(BvResourceBindingStateVk&& rhs) noexcept;
	BvResourceBindingStateVk& operator=(BvResourceBindingStateVk&& rhs) noexcept;
	~BvResourceBindingStateVk();

	void SetResource(VkDescriptorType descriptorType, const BvBufferViewVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const BvTextureViewVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const BvSamplerVk* pResource, u32 set, u32 binding, u32 arrayIndex);

	void Reset();

	const ResourceDataVk* GetResource(const ResourceIdVk& resId) const;

	BV_INLINE bool IsEmpty() const { return m_Resources.Empty(); }

private:
	ResourceDataVk& AddOrRetrieveResourceData(u32 set, u32 binding, u32 arrayIndex);

private:
	BvRobinMap<ResourceIdVk, u32> m_Bindings;
	BvVector<ResourceDataVk> m_Resources;
};


class BvDescriptorSetVk final
{
public:
	BvDescriptorSetVk();
	BvDescriptorSetVk(const BvRenderDeviceVk* pDevice, VkDescriptorSet descriptorSet);
	BvDescriptorSetVk(BvDescriptorSetVk&& rhs) noexcept;
	BvDescriptorSetVk& operator=(BvDescriptorSetVk&& rhs) noexcept;
	~BvDescriptorSetVk();

	void Update(const BvVector<VkWriteDescriptorSet>& writeSets);

	BV_INLINE VkDescriptorSet GetHandle() const { return m_DescriptorSet; }

private:
	const BvRenderDeviceVk* m_pDevice = nullptr;
	VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
};


class BvDescriptorPoolVk final
{
public:
	BvDescriptorPoolVk();
	BvDescriptorPoolVk(const BvRenderDeviceVk* pDevice, const BvShaderResourceLayoutVk* pLayout, u32 set, u32 maxAllocationsPerPool);
	BvDescriptorPoolVk(BvDescriptorPoolVk&& rhs) noexcept;
	BvDescriptorPoolVk& operator=(BvDescriptorPoolVk&& rhs) noexcept;
	~BvDescriptorPoolVk();

	void Create();
	void Destroy();

	VkDescriptorSet Allocate();
	void Reset();

	BV_INLINE bool IsValid() const { return m_Layout != VK_NULL_HANDLE; }

private:
	struct PoolData
	{
		VkDescriptorPool pool;
		u32 currAllocationCount;
	};

	const BvRenderDeviceVk* m_pDevice = nullptr;
	VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
	BvVector<PoolData> m_DescriptorPools;
	BvVector<VkDescriptorPoolSize> m_PoolSizes;
	u32 m_MaxAllocationsPerPool = 0;
	u32 m_CurrPoolIndex = 0;
};