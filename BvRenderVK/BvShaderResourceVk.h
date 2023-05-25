#pragma once


#include "BDeV/RenderAPI/BvShaderResource.h"
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


class BvShaderResourceParamsVk final : public BvShaderResourceParams
{
public:
	BvShaderResourceParamsVk(const BvRenderDeviceVk & device, const BvShaderResourceLayoutVk * const pLayout,
		const VkDescriptorSet set, BvShaderResourceSetPoolVk * const pDescriptorPool, const u32 setIndex);
	~BvShaderResourceParamsVk();

	void Create();
	void Destroy();

	void SetBuffers(const u32 binding, const u32 count, const BvBufferView * const * const ppBuffers, const u32 startIndex = 0) override final;
	void SetTextures(const u32 binding, const u32 count, const BvTextureView * const * const ppTextures, const u32 startIndex = 0) override final;
	void SetSamplers(const u32 binding, const u32 count, const BvSampler * const * const ppSamplers, const u32 startIndex = 0) override final;

	void Update() override final;

	BV_INLINE const BvShaderResourceLayoutVk* GetLayout() const { return &m_Layout; }
	BV_INLINE VkDescriptorSet GetHandle() const { return m_DescriptorSet; }
	BV_INLINE u32 GetSetIndex() const { return m_SetIndex; }

	VkWriteDescriptorSet& GetWriteSet(const u32 binding);

	void GetDescriptorInfo(const u32 binding, VkDescriptorType & descriptorType, VkImageLayout * pImageLayout = nullptr);

private:
	const BvRenderDeviceVk& m_Device;
	const BvShaderResourceLayoutVk& m_Layout;
	
	VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
	BvVector<VkWriteDescriptorSet> m_WriteSets;
	BvVector<class BvShaderVariableVk*> m_ShaderVariables;

	u32 m_SetIndex = 0;
};


class BvShaderVariableVk
{
	BV_NOCOPYMOVE(BvShaderVariableVk);

public:
	BvShaderVariableVk(BvShaderResourceParamsVk& shaderParams, const u32 binding)
		: m_ShaderParams(shaderParams), m_Binding(binding) {}
	virtual ~BvShaderVariableVk() = 0 {}

	virtual void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) = 0;
	virtual void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) = 0;
	virtual void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) = 0;

	BV_INLINE const u32 GetBinding() const { return m_Binding; }

protected:
	BvShaderResourceParamsVk& m_ShaderParams;
	u32 m_Binding = 0;
};


class BvShaderBufferVariableVk final : public BvShaderVariableVk
{
	BV_NOCOPYMOVE(BvShaderBufferVariableVk);

public:
	BvShaderBufferVariableVk(BvShaderResourceParamsVk& shaderParams, const u32 binding, const u32 count)
		: BvShaderVariableVk(shaderParams, binding)
	{
		m_BufferInfos.Resize(count, {});
	}
	~BvShaderBufferVariableVk() {}

	void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) override;
	void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) override;
	void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) override;

protected:
	BvVector<VkDescriptorBufferInfo> m_BufferInfos;
};


class BvShaderImageVariableVk final : public BvShaderVariableVk
{
	BV_NOCOPYMOVE(BvShaderImageVariableVk);

public:
	BvShaderImageVariableVk(BvShaderResourceParamsVk& shaderParams, const u32 binding, const u32 count)
		: BvShaderVariableVk(shaderParams, binding)
	{
		m_ImageInfos.Resize(count, {});
	}
	~BvShaderImageVariableVk() {}

	void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) override;
	void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) override;
	void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) override;

protected:
	BvVector<VkDescriptorImageInfo> m_ImageInfos;
};


class BvShaderBufferViewVariableVk final : public BvShaderVariableVk
{
	BV_NOCOPYMOVE(BvShaderBufferViewVariableVk);

public:
	BvShaderBufferViewVariableVk(BvShaderResourceParamsVk& shaderParams, const u32 binding, const u32 count)
		: BvShaderVariableVk(shaderParams, binding)
	{
		m_BufferViews.Resize(count, {});
	}
	~BvShaderBufferViewVariableVk() {}

	void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) override;
	void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) override;
	void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) override;

protected:
	BvVector<VkBufferView> m_BufferViews;
};