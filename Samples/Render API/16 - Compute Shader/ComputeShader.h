#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class ComputeShader : public SampleBase
{
public:
	ComputeShader() {}
	~ComputeShader() {}

	void OnInitialize() override;
	void OnUpdate() override;
	void OnRender() override;
	void DoCompute(u32 w, u32 h);
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateTextures();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
	BvRCRef<IBvShaderResourceLayout> m_SRLCompute;
	BvRCRef<IBvComputePipelineState> m_PSOCompute;
	BvRCRef<IBvTexture> m_RWTex;
	BvRCRef<IBvTextureView> m_RWTexView;
	BvRCRef<IBvSampler> m_Sampler;
	struct PC
	{
		Float2 res;
		f32 time;
	} m_PC{};
};