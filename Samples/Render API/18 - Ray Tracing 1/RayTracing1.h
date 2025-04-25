#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class RayTracing1 : public SampleBase
{
public:
	RayTracing1() {}
	~RayTracing1() {}

	void OnInitialize() override;
	void OnUpdate() override;
	void OnUpdateUI() override;
	void OnRender() override;
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateResources();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;

	BvRCRef<IBvShaderResourceLayout> m_RaySRL;
	BvRCRef<IBvRayTracingPipelineState> m_RayPSO;
	BvRCRef<IBvTexture> m_Tex;
	BvRCRef<IBvTextureView> m_TexView;
	BvRCRef<IBvSampler> m_Sampler;
	BvRCRef<IBvAccelerationStructure> m_AS;
	BvRCRef<IBvShaderBindingTable> m_SBT;
	BvRCRef<IBvBuffer> m_ScratchBuffer;

	Float3 m_BackColor{};
};