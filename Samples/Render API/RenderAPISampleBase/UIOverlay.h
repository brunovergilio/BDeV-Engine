#pragma once


#include "BDeV/BDeV.h"
#include "Third Party/imgui.h"


class UIOverlay
{
public:
	UIOverlay();
	~UIOverlay();

	void Initialize(IBvRenderDevice* pDevice, IBvCommandContext* pContext, IBvShaderCompiler* pCompiler);
	void SetupPipeline(Format swapChainFormat, Format depthFormat = Format::kUnknown, u32 sampleCount = 1);
	void SetupPipeline(IBvRenderPass* pRenderPass = nullptr, u32 subpassIndex = 0, u32 sampleCount = 1);
	void SetupPipeline(Format swapChainFormat, Format depthFormat, IBvRenderPass* pRenderPass, u32 subpassIndex, u32 sampleCount);

	bool Update(f32 dt, BvWindow* pWindow);
	void Render(bool msaa = false);
	void Shutdown();

private:
	void CreateVB(u64 size, u32 count);
	void CreateIB(u64 size, u32 count);

private:
	BvRCRef<IBvRenderDevice> m_Device;
	BvRCRef<IBvShaderCompiler> m_Compiler;
	BvRCRef<IBvBuffer> m_VB;
	BvRCRef<IBvBuffer> m_IB;
	BvRCRef<IBvGraphicsPipelineState> m_Pipeline;
	BvRCRef<IBvGraphicsPipelineState> m_PipelineMSAA;
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvTexture> m_Texture;
	BvRCRef<IBvTextureView> m_TextureView;
	BvRCRef<IBvSampler> m_Sampler;
	BvRCRef<IBvCommandContext> m_Context;
	BvRCRef<IBvShader> m_VS;
	BvRCRef<IBvShader> m_PS;

	u32 m_VertexCount = 0;
	u32 m_IndexCount = 0;

	struct PushConstBlock
	{
		Float44 wvp;
	} m_PC;

	f32 m_OverlayTimer = 0.0f;
};