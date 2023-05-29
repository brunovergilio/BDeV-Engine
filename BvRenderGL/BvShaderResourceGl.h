#pragma once


#include "BDeV/RenderAPI/BvShaderResource.h"
#include "BvCommonGl.h"


class BvRenderDeviceGl;
class BvShaderResourceSetPoolGl;


class BvShaderResourceLayoutGl final : public BvShaderResourceLayout
{
public:
	BvShaderResourceLayoutGl(const BvRenderDeviceGl& device, const ShaderResourceLayoutDesc& shaderResourceLayoutDesc);
	~BvShaderResourceLayoutGl();

private:
	const BvRenderDeviceGl& m_Device;
};


class BvShaderResourceParamsGl final : public BvShaderResourceParams
{
public:
	BvShaderResourceParamsGl(const BvRenderDeviceGl& device, const BvShaderResourceLayoutGl& layout);
	~BvShaderResourceParamsGl();

	void Create();
	void Destroy();

	void SetBuffers(const u32 binding, const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) override final;
	void SetTextures(const u32 binding, const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) override final;
	void SetSamplers(const u32 binding, const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) override final;

	void Update() override final;

	BV_INLINE u32 GetSetIndex() const { return m_SetIndex; }

private:
	const BvRenderDeviceGl& m_Device;
	const BvShaderResourceLayoutGl& m_Layout;

	BvVector<class BvShaderVariableGl*> m_ShaderVariables;

	u32 m_SetIndex = 0;
};


class BvShaderVariableGl
{
	BV_NOCOPYMOVE(BvShaderVariableGl);

public:
	BvShaderVariableGl(BvShaderResourceParamsGl& shaderParams, const u32 binding)
		: m_ShaderParams(shaderParams), m_Binding(binding) {}
	virtual ~BvShaderVariableGl() = 0 {}

	virtual void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) = 0;
	virtual void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) = 0;
	virtual void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) = 0;

	BV_INLINE const u32 GetBinding() const { return m_Binding; }

protected:
	BvShaderResourceParamsGl& m_ShaderParams;
	u32 m_Binding = 0;
};


class BvShaderBufferVariableGl final : public BvShaderVariableGl
{
	BV_NOCOPYMOVE(BvShaderBufferVariableGl);

public:
	BvShaderBufferVariableGl(BvShaderResourceParamsGl& shaderParams, const u32 binding, const u32 count)
		: BvShaderVariableGl(shaderParams, binding)
	{
		m_BufferInfos.Resize(count, {});
	}
	~BvShaderBufferVariableGl() {}

	void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) override;
	void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) override;
	void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) override;

protected:
	BvVector<GLuint> m_BufferInfos;
};


class BvShaderImageVariableGl final : public BvShaderVariableGl
{
	BV_NOCOPYMOVE(BvShaderImageVariableGl);

public:
	BvShaderImageVariableGl(BvShaderResourceParamsGl& shaderParams, const u32 binding, const u32 count)
		: BvShaderVariableGl(shaderParams, binding)
	{
		m_ImageInfos.Resize(count, {});
	}
	~BvShaderImageVariableGl() {}

	void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) override;
	void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) override;
	void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) override;

protected:
	BvVector<GLuint> m_ImageInfos;
};


class BvShaderBufferViewVariableGl final : public BvShaderVariableGl
{
	BV_NOCOPYMOVE(BvShaderBufferViewVariableGl);

public:
	BvShaderBufferViewVariableGl(BvShaderResourceParamsGl& shaderParams, const u32 binding, const u32 count)
		: BvShaderVariableGl(shaderParams, binding)
	{
		m_BufferViews.Resize(count, {});
	}
	~BvShaderBufferViewVariableGl() {}

	void SetBuffers(const u32 count, const BvBufferView* const* const ppBuffers, const u32 startIndex = 0) override;
	void SetTextures(const u32 count, const BvTextureView* const* const ppTextures, const u32 startIndex = 0) override;
	void SetSamplers(const u32 count, const BvSampler* const* const ppSamplers, const u32 startIndex = 0) override;

protected:
	BvVector<GLuint> m_BufferViews;
};