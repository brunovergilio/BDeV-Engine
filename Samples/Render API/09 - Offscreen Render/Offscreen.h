#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class Offscreen : public SampleBase
{
public:
	Offscreen() {}
	~Offscreen() {}

	void OnInitialize() override;
	void OnUpdate() override;
	void RenderOffscreen();
	void OnRender() override;
	void OnShutdown() override;


private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateBuffers();
	void CreateRenderTargets();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRLOffscreen;
	BvRCRef<IBvGraphicsPipelineState> m_PSOOffscreen;
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSOColor;
	BvRCRef<IBvGraphicsPipelineState> m_PSODepth;
	BvRCRef<IBvBuffer> m_VB;
	BvRCRef<IBvBuffer> m_IB;
	BvRCRef<IBvBuffer> m_UB;
	BvRCRef<IBvBufferView> m_UBView;
	BvRCRef<IBvTexture> m_Color;
	BvRCRef<IBvTextureView> m_ColorView;
	BvRCRef<IBvTexture> m_Depth;
	BvRCRef<IBvTextureView> m_DepthView;
	BvRCRef<IBvSampler> m_Sampler;
	Float44* m_pWVP = nullptr;
	struct PCData
	{
		Float4 m_PosUV[6];
	} m_PCColor, m_PCDepth;
};