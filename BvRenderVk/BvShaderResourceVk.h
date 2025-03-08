#pragma once


#include "BDeV/Core/RenderAPI/BvShaderResource.h"
#include "BvDescriptorSetVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvShaderResourceLayoutVk, "66b68f82-ff43-4f16-877a-e005b07d5e0c");
class IBvShaderResourceLayoutVk : public IBvShaderResourceLayout
{
protected:
	struct ResourceId
	{
		u32 m_Binding;
		u32 m_Set;

		friend bool operator==(const ResourceId& lhs, const ResourceId& rhs)
		{
			return lhs.m_Binding == rhs.m_Binding && lhs.m_Set == rhs.m_Set;
		}
	};

	struct PushConstantId
	{
		u32 m_Size;
		u32 m_Binding;
		u32 m_Set;

		friend bool operator==(const PushConstantId& lhs, const PushConstantId& rhs)
		{
			return lhs.m_Size == rhs.m_Size && lhs.m_Binding == rhs.m_Binding
				&& lhs.m_Set == rhs.m_Set;
		}
	};

	struct PushConstantData
	{
		VkShaderStageFlags m_ShaderStages;
		u32 m_Offset;
	};

public:
	virtual const BvRobinMap<u32, VkDescriptorSetLayout>& GetSetLayoutHandles() const = 0;
	virtual VkPipelineLayout GetPipelineLayoutHandle() const = 0;
	virtual bool IsValid() const = 0;
	virtual const ShaderResourceSetDesc* GetResourceSet(u32 set) const = 0;
	virtual const ShaderResourceDesc* GetResource(u32 binding, u32 set) const = 0;
	virtual PushConstantData* GetPushConstantData(u32 size, u32 binding, u32 set) const = 0;

protected:
	IBvShaderResourceLayoutVk() {}
	~IBvShaderResourceLayoutVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderResourceLayoutVk);


class BvShaderResourceLayoutVk final : public IBvShaderResourceLayoutVk
{
public:
	BvShaderResourceLayoutVk(BvRenderDeviceVk* pDevice, const ShaderResourceLayoutDesc& srlDesc);
	~BvShaderResourceLayoutVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE const ShaderResourceLayoutDesc& GetDesc() const override { return m_ShaderResourceLayoutDesc; }
	BV_INLINE const BvRobinMap<u32, VkDescriptorSetLayout>& GetSetLayoutHandles() const override { return m_Layouts; }
	BV_INLINE VkPipelineLayout GetPipelineLayoutHandle() const override { return m_PipelineLayout; }
	BV_INLINE bool IsValid() const override { return m_PipelineLayout != VK_NULL_HANDLE; }
	const ShaderResourceSetDesc* GetResourceSet(u32 set) const override;
	const ShaderResourceDesc* GetResource(u32 binding, u32 set) const override;
	PushConstantData* GetPushConstantData(u32 size, u32 binding, u32 set) const override;

	BV_OBJECT_IMPL_INTERFACE(IBvShaderResourceLayoutVk, IBvShaderResourceLayout, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	ShaderResourceLayoutDesc m_ShaderResourceLayoutDesc;
	BvRobinMap<u32, VkDescriptorSetLayout> m_Layouts{};
	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
	ShaderResourceDesc* m_pShaderResources = nullptr;
	ShaderResourceConstantDesc* m_pShaderConstants = nullptr;
	ShaderResourceSetDesc* m_pShaderResourceSets = nullptr;
	IBvSampler** m_ppStaticSamplers = nullptr;
	BvRobinMap<ResourceId, const ShaderResourceDesc*> m_Resources;
	BvRobinMap<PushConstantId, PushConstantData> m_PushConstantOffsets;
};


class BvShaderResourceParamsVk final : public BvShaderResourceParams
{
public:
	BvShaderResourceParamsVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, u32 set);
	~BvShaderResourceParamsVk();

	void SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 binding, u32 startIndex = 0) override;

	void Bind() override;

	BV_INLINE const BvShaderResourceLayoutVk* GetLayout() const { return &m_Layout; }
	BV_INLINE VkDescriptorSet GetHandle() const { return m_DescriptorSet.GetHandle(); }
	BV_INLINE u32 GetSetIndex() const { return m_Set; }

private:
	VkWriteDescriptorSet& PrepareWriteSet(VkDescriptorType descriptorType, u32 count, u32 binding, u32 startIndex);
	VkDescriptorType GetDescriptorType(u32 binding) const;

private:
	const BvRenderDeviceVk& m_Device;
	const BvShaderResourceLayoutVk& m_Layout;
	BvDescriptorPoolVk m_DescriptorPool;
	BvDescriptorSetVk m_DescriptorSet;
	struct SetData
	{
		BvVector<VkWriteDescriptorSet> m_WriteSets;
		BvVector<VkWriteDescriptorSetAccelerationStructureKHR> m_ASWriteSets;
		BvVector<u32> m_WriteSetDataIndices;
		BvVector<VkDescriptorBufferInfo> m_BufferInfos;
		BvVector<VkDescriptorImageInfo> m_ImageInfos;
		BvVector<VkBufferView> m_BufferViews;
		BvVector<VkAccelerationStructureKHR> m_AccelerationStructures;
	} * m_pSetData = nullptr;
	u32 m_Set = 0;
};


BV_CREATE_CAST_TO_VK(IBvShaderResourceLayout)
BV_CREATE_CAST_TO_VK(BvShaderResourceParams)