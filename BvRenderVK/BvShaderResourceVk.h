#pragma once


#include "BvRender/BvShaderResource.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;
class BvShaderResourceSetPoolVk;


class BvShaderResourceLayoutVk final : public BvShaderResourceLayout
{
public:
	BvShaderResourceLayoutVk(const BvRenderDeviceVk & device, const ShaderResourceLayoutDesc & shaderResourceLayoutDesc);
	~BvShaderResourceLayoutVk();

	void Create();
	void Destroy();

	BV_INLINE const BvRobinMap<u32, VkDescriptorSetLayout> & GetSetLayoutHandles() const { return m_Layouts; }
	BV_INLINE VkPipelineLayout GetPipelineLayoutHandle() const { return m_PipelineLayout; }

private:
	const BvRenderDeviceVk & m_Device;
	BvRobinMap<u32, VkDescriptorSetLayout> m_Layouts{};
	VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
};


class BvShaderResourceSetVk final : public BvShaderResourceSet
{
public:
	BvShaderResourceSetVk(const BvRenderDeviceVk & device, const BvShaderResourceLayoutVk * const pLayout,
		const VkDescriptorSet set, BvShaderResourceSetPoolVk * const pDescriptorPool, const u32 setIndex);
	virtual ~BvShaderResourceSetVk();

	void Create();
	void Destroy();

	void SetBuffers(const u32 binding, const u32 count, const BvBufferView * const * const ppBuffers) override final;
	void SetTextures(const u32 binding, const u32 count, const BvTextureView * const * const ppTextures) override final;
	void SetSamplers(const u32 binding, const u32 count, const BvSampler * const * const ppSamplers) override final;

	void Update() override final;

	BV_INLINE const BvShaderResourceLayoutVk* GetLayout() const { return &m_Layout; }
	BV_INLINE VkDescriptorSet GetHandle() const { return m_DescriptorSet; }
	BV_INLINE u32 GetSetIndex() const { return m_SetIndex; }

private:
	void GetDescriptorInfo(const u32 binding, VkDescriptorType & descriptorType, VkImageLayout * pImageLayout = nullptr);

private:
	struct DescriptorData
	{
		struct ResourceInfo
		{
			union
			{
				VkDescriptorImageInfo imageInfo;
				VkDescriptorBufferInfo bufferInfo;
				VkBufferView texelBufferView;
			};
		};

		BvVector<VkWriteDescriptorSet> m_WriteSets;
		BvVector<ResourceInfo> m_Resources;
		u32 m_DirtyWriteSetCount = 0;
		u32 m_DirtyResourceInfoCount = 0;
	} * m_pDescriptorData = nullptr;

	const BvRenderDeviceVk & m_Device;
	const BvShaderResourceLayoutVk& m_Layout;
	VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

	u32 m_SetIndex = 0;
};