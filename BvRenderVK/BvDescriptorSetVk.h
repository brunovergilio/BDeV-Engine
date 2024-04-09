#pragma once


#include "BvCommonVk.h"
#include "BDeV/RenderAPI/BvShaderResource.h"


class BvRenderDeviceVk;
class BvShaderResourceLayoutVk;
class BvBufferViewVk;
class BvTextureViewVk;
class BvSamplerVk;


class BvResourceBindingState final
{
public:
	struct DescriptorData
	{
		union Data
		{
			VkDescriptorBufferInfo bufferInfo;
			VkDescriptorImageInfo imageInfo;
			VkBufferView bufferView;
		};

		// I'm having one element separate from the vector because I believe that in most cases only one element will be needed,
		// so I won't have to make another dynamic allocation. If more is needed, I can use the vector for the extra elements
		Data m_Element;
		BvVector<Data> m_Elements;
		u32 m_Count;
		VkDescriptorType m_DescriptorType;
	};

	BvResourceBindingState(u32 set);
	BvResourceBindingState(BvResourceBindingState&& rhs);
	~BvResourceBindingState();

	void SetResource(const BvBufferViewVk& resource, u32 binding, u32 arrayIndex = 0);
	void SetResource(const BvTextureViewVk& resource, u32 binding, u32 arrayIndex = 0);
	void SetResource(const BvSamplerVk& resource, u32 binding, u32 arrayIndex = 0);

	BV_INLINE auto& GetBindings() const { return m_Bindings; }
	BV_INLINE bool IsDirty() const { return m_IsDirty; }

private:
	BvRobinMap<u32, BvRobinMap<u32, DescriptorData>> m_Bindings;
	bool m_IsDirty = false;
};


class BvDescriptorSetVk final
{
public:
	BvDescriptorSetVk(const BvRenderDeviceVk& device, VkDescriptorSet descriptorSet);
	BvDescriptorSetVk(BvDescriptorSetVk&& rhs);
	~BvDescriptorSetVk();

	void Update(const BvResourceBindingState& descriptorData);

	BV_INLINE VkDescriptorSet GetHandle() const { return m_DescriptorSet; }

private:
	const BvRenderDeviceVk& m_Device;
	VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
};


class BvDescriptorPoolVk final
{
public:
	BvDescriptorPoolVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, u32 set, u32 maxAllocationsPerPool);
	BvDescriptorPoolVk(BvDescriptorPoolVk&& rhs);
	~BvDescriptorPoolVk();

	void Create();
	void Destroy();

	VkDescriptorSet Allocate();
	void Reset();

private:
	struct PoolData
	{
		VkDescriptorPool pool;
		u32 currAllocationCount;
	};

	const BvRenderDeviceVk& m_Device;
	VkDescriptorSetLayout m_Layout;
	BvVector<PoolData> m_DescriptorPools;
	BvVector<VkDescriptorPoolSize> m_PoolSizes;
	u32 m_MaxAllocationsPerPool;
	u32 m_CurrPoolIndex;
};