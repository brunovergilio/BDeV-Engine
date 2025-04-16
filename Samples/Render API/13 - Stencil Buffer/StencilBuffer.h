#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class StencilBuffer : public SampleBase
{
public:
	StencilBuffer() {}
	~StencilBuffer() {}

	void OnInitialize() override;
	void OnInitializeUI() override;
	void OnUpdate() override;
	void OnUpdateUI() override;
	void OnRender() override;
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateBuffers();
	void CreateRenderTargets();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvShaderResourceLayout> m_SRLOutline;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
	BvRCRef<IBvGraphicsPipelineState> m_PSOOutline;
	BvRCRef<IBvBuffer> m_VB;
	BvRCRef<IBvBuffer> m_IB;
	BvRCRef<IBvBuffer> m_UB;
	BvRCRef<IBvBufferView> m_UBView;
	BvRCRef<IBvBuffer> m_UBOutline;
	BvRCRef<IBvBufferView> m_UBViewOutline;
	BvRCRef<IBvTexture> m_Depth;
	BvRCRef<IBvTextureView> m_DepthView;
	Float44* m_pWVP = nullptr;
	Float44* m_pWVPOutline = nullptr;
	bool m_DrawOutline = true;
	f32 m_OutlineScale = 1.05f;
	Float4 m_OutlineColor{1.0f, 1.0f, 1.0f, 1.0f};
};