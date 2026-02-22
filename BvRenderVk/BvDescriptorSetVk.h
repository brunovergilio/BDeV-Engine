#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderResource.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvRenderDeviceVk;
class BvShaderResourceLayoutVk;
class BvBufferViewVk;
class BvTextureViewVk;
class BvSamplerVk;
class BvAccelerationStructureVk;


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
		VkAccelerationStructureKHR m_AccelerationStructure;
	};

	u32 m_Binding;
	u32 m_ArrayIndex;
	u32 m_DynamicOffset;
	Data m_Data;
	VkDescriptorType m_DescriptorType;

	bool Set(VkDescriptorType descriptorType, const BvBufferViewVk* pResource, u32 binding, u32 arrayIndex, u32 dynamicOffset = 0);
	bool Set(VkDescriptorType descriptorType, const BvTextureViewVk* pResource, u32 binding, u32 arrayIndex);
	bool Set(VkDescriptorType descriptorType, const BvSamplerVk* pResource, u32 binding, u32 arrayIndex);
	bool Set(VkDescriptorType descriptorType, const BvAccelerationStructureVk* pResource, u32 binding, u32 arrayIndex);
};


class BvResourceBindingStateVk final
{
public:
	BvResourceBindingStateVk();
	~BvResourceBindingStateVk();

	void SetResource(VkDescriptorType descriptorType, const BvBufferViewVk* pResource, u32 set, u32 binding, u32 arrayIndex, u32 offset = 0);
	void SetResource(VkDescriptorType descriptorType, const BvTextureViewVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const BvSamplerVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const BvAccelerationStructureVk* pResource, u32 set, u32 binding, u32 arrayIndex);

	void Reset();

	const ResourceDataVk* GetResource(const ResourceIdVk& resId) const;
	std::pair<u32, const ResourceDataVk*> GetResources(u32 set) const;

	BV_INLINE bool IsDirty(u32 set) const { return m_DirtySets.Size() > set && m_DirtySets[set]; }
	BV_INLINE void MarkClean(u32 set) { if (m_DirtySets.Size() > set) { m_DirtySets[set] = false; } }

private:
	std::pair<ResourceDataVk*, bool> AddOrRetrieveResourceData(u32 set, u32 binding, u32 arrayIndex);

private:
	BvRobinMap<ResourceIdVk, u32> m_Bindings;
	BvVector<BvVector<ResourceDataVk>> m_PerSetResources;
	BvVector<bool> m_DirtySets;
};


class BvDescriptorPoolVk final
{
public:
	BvDescriptorPoolVk(BvRenderDeviceVk* pDevice, const BvShaderResourceLayoutVk* pLayout, u32 set, u32 maxAllocationsPerPool);
	~BvDescriptorPoolVk();

	void Create();
	void Destroy();

	VkDescriptorSet Allocate();
	void RecycleDescriptor(VkDescriptorSet descriptorSet);
	void Reset();

private:
	struct PoolData
	{
		VkDescriptorPool pool;
		u32 currAllocationCount;
	};

	BvRenderDeviceVk* m_pDevice = nullptr;
	const BvShaderResourceLayoutVk* m_pLayout = nullptr;
	VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
	BvVector<PoolData> m_DescriptorPools;
	BvVector<VkDescriptorPoolSize> m_PoolSizes;
	BvVector<VkDescriptorSet> m_FreeDescriptorSets;
	u32 m_SetIndex = 0;
	u32 m_MaxAllocationsPerPool = 0;
	u32 m_CurrPoolIndex = 0;
	bool m_IsBindless = false;
};