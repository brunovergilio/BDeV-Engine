#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvShaderResource.h"


class IBvRenderDeviceVk;
class IBvShaderResourceLayoutVk;
class IBvBufferViewVk;
class IBvTextureViewVk;
class IBvSamplerVk;
class IBvAccelerationStructureVk;


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

	u32 m_DynamicOffset;
	Data m_Data;
	VkDescriptorType m_DescriptorType;

	bool Set(VkDescriptorType descriptorType, const IBvBufferViewVk* pResource, u32 dynamicOffset = 0);
	bool Set(VkDescriptorType descriptorType, const IBvTextureViewVk* pResource);
	bool Set(VkDescriptorType descriptorType, const IBvSamplerVk* pResource);
	bool Set(VkDescriptorType descriptorType, const IBvAccelerationStructureVk* pResource);
};


class BvResourceBindingStateVk final
{
public:

	BvResourceBindingStateVk();
	BvResourceBindingStateVk(BvResourceBindingStateVk&& rhs) noexcept;
	BvResourceBindingStateVk& operator=(BvResourceBindingStateVk&& rhs) noexcept;
	~BvResourceBindingStateVk();

	void SetResource(VkDescriptorType descriptorType, const IBvBufferViewVk* pResource, u32 set, u32 binding, u32 arrayIndex, u32 offset = 0);
	void SetResource(VkDescriptorType descriptorType, const IBvTextureViewVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const IBvSamplerVk* pResource, u32 set, u32 binding, u32 arrayIndex);
	void SetResource(VkDescriptorType descriptorType, const IBvAccelerationStructureVk* pResource, u32 set, u32 binding, u32 arrayIndex);

	void Reset();

	const ResourceDataVk* GetResource(const ResourceIdVk& resId) const;

	BV_INLINE bool IsEmpty() const { return m_Resources.Empty(); }
	BV_INLINE bool IsDirty(u32 set) const { auto it = m_DirtySets.FindKey(set); return it != m_DirtySets.cend() ? it->second : false; }
	BV_INLINE void MarkClean(u32 set) { m_DirtySets[set] = false; }

private:
	std::pair<ResourceDataVk*, bool> AddOrRetrieveResourceData(u32 set, u32 binding, u32 arrayIndex);

private:
	BvRobinMap<ResourceIdVk, u32> m_Bindings;
	BvVector<ResourceDataVk> m_Resources;
	BvRobinMap<u32, bool> m_DirtySets;
};


class BvDescriptorSetVk final
{
public:
	BvDescriptorSetVk();
	BvDescriptorSetVk(const IBvRenderDeviceVk* pDevice, VkDescriptorSet descriptorSet);
	BvDescriptorSetVk(BvDescriptorSetVk&& rhs) noexcept;
	BvDescriptorSetVk& operator=(BvDescriptorSetVk&& rhs) noexcept;
	~BvDescriptorSetVk();

	void Update(const BvVector<VkWriteDescriptorSet>& writeSets);

	BV_INLINE VkDescriptorSet GetHandle() const { return m_DescriptorSet; }

private:
	const IBvRenderDeviceVk* m_pDevice = nullptr;
	VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
};


class BvDescriptorPoolVk final
{
public:
	BvDescriptorPoolVk();
	BvDescriptorPoolVk(const IBvRenderDeviceVk* pDevice, const IBvShaderResourceLayoutVk* pLayout, u32 set, u32 maxAllocationsPerPool);
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

	const IBvRenderDeviceVk* m_pDevice = nullptr;
	const IBvShaderResourceLayoutVk* m_pLayout = nullptr;
	VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
	BvVector<PoolData> m_DescriptorPools;
	BvVector<VkDescriptorPoolSize> m_PoolSizes;
	u32 m_SetIndex = 0;
	u32 m_MaxAllocationsPerPool = 0;
	u32 m_CurrPoolIndex = 0;
	bool m_IsBindless = false;
};