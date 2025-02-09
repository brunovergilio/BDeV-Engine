#pragma once


#include "BDeV/Core/RenderAPI/BvShaderResource.h"
#include "BvDescriptorSetVk.h"


class BvRenderDeviceVk;


class BvShaderResourceLayoutVk final : public BvShaderResourceLayout
{
public:
	BvShaderResourceLayoutVk(BvRenderDeviceVk* pDevice, u32 shaderResourceCount,
		const ShaderResourceDesc* pShaderResourceDescs, const ShaderResourceConstantDesc* pShaderResourceConstantDesc);
	~BvShaderResourceLayoutVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE const BvRobinMap<u32, VkDescriptorSetLayout>& GetSetLayoutHandles() const { return m_Layouts; }
	BV_INLINE VkPipelineLayout GetPipelineLayoutHandle() const { return m_PipelineLayout; }

private:
	void Create();
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvRobinMap<u32, VkDescriptorSetLayout> m_Layouts{};
	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
};


class BvShaderResourceParamsVk final : public BvShaderResourceParams
{
public:
	BvShaderResourceParamsVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, u32 set);
	~BvShaderResourceParamsVk();

	void SetConstantBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetRWStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetRWFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetTextures(u32 count, const BvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetRWTextures(u32 count, const BvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) override;
	void SetSamplers(u32 count, const BvSampler* const* ppResources, u32 binding, u32 startIndex = 0) override;

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
		BvVector<u32> m_WriteSetDataIndices;
		BvVector<VkDescriptorBufferInfo> m_BufferInfos;
		BvVector<VkDescriptorImageInfo> m_ImageInfos;
		BvVector<VkBufferView> m_BufferViews;
	} * m_pSetData = nullptr;
	u32 m_Set = 0;
};


BV_CREATE_CAST_TO_VK(BvShaderResourceLayout)
BV_CREATE_CAST_TO_VK(BvShaderResourceParams)