#pragma once


#include "BDeV/Core/RenderAPI/BvShaderResource.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;
class BvDescriptorPoolVk;


class BvShaderResourceLayoutVk final : public BvShaderResourceLayout
{
public:
	BvShaderResourceLayoutVk(BvRenderDeviceVk* pDevice, u32 shaderResourceCount,
		const ShaderResourceDesc* pShaderResourceDescs, const ShaderResourceConstantDesc& shaderResourceConstantDesc);
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
	BvShaderResourceParamsVk(const BvRenderDeviceVk& device, const BvShaderResourceLayoutVk& layout, BvDescriptorPoolVk& descriptorPool, u32 set);
	~BvShaderResourceParamsVk();

	void SetResources(u32 count, const BvBufferView* const* ppBuffers, u32 binding, u32 startIndex = 0) override final;
	void SetResources(u32 count, const BvTextureView* const* ppTextures, u32 binding, u32 startIndex = 0) override final;
	void SetResources(u32 count, const BvSampler* const* ppSamplers, u32 binding, u32 startIndex = 0) override final;

	void Bind() override final;

	BV_INLINE const BvShaderResourceLayoutVk* GetLayout() const { return &m_Layout; }
	//BV_INLINE VkDescriptorSet GetHandle() const { return m_pDescriptorSet->GetHandle(); }
	BV_INLINE u32 GetSetIndex() const { return m_Set; }

private:
	const BvRenderDeviceVk& m_Device;
	const BvShaderResourceLayoutVk& m_Layout;
	u32 m_Set = 0;
};


BV_CREATE_CAST_TO_VK(BvShaderResourceLayout)
BV_CREATE_CAST_TO_VK(BvShaderResourceParams)