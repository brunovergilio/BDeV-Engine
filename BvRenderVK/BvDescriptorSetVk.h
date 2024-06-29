#pragma once


#include "BvCommonVk.h"
#include "BDeV/RenderAPI/BvShaderResource.h"


class BvRenderDeviceVk;
class BvShaderResourceLayoutVk;
class BvBufferViewVk;
class BvTextureViewVk;
class BvSamplerVk;


class BvResourceBindingStateVk final
{
public:
	struct ResourceId
	{
		u32 m_Set;
		u32 m_Binding;
		u32 m_ArrayIndex;

		friend bool operator==(const ResourceId& lhs, const ResourceId& rhs)
		{
			return lhs.m_Set == rhs.m_Set && lhs.m_Binding == rhs.m_Binding && lhs.m_ArrayIndex == rhs.m_ArrayIndex;
		}
	};

	struct ResourceData
	{
		union Data
		{
			VkDescriptorBufferInfo m_BufferInfo;
			VkDescriptorImageInfo m_ImageInfo;
			VkBufferView m_BufferView;
		};

		Data m_Data;
		VkDescriptorType m_DescriptorType;
	};

	BvResourceBindingStateVk();
	BvResourceBindingStateVk(BvResourceBindingStateVk&& rhs) noexcept;
	BvResourceBindingStateVk& operator=(BvResourceBindingStateVk&& rhs) noexcept;
	~BvResourceBindingStateVk();

	void SetResource(VkDescriptorType descriptorType, const BvBufferViewVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const BvTextureViewVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const BvSamplerVk* pResource, u32 set, u32 binding, u32 arrayIndex);

	void Reset();

	const ResourceData* GetResource(const ResourceId& resId) const;

private:
	ResourceData& AddOrRetrieveResourceData(u32 set, u32 binding, u32 arrayIndex);

private:
	BvRobinMap<ResourceId, u32> m_Bindings;
	BvVector<ResourceData> m_Resources;
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