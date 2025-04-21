#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class MeshShader : public SampleBase
{
public:
	MeshShader() {}
	~MeshShader() {}

	void OnInitialize() override;
	void OnInitializeUI() override;
	void OnUpdate() override;
	void OnUpdateUI() override;
	void OnRender() override;
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateRenderTargets();
	void CreateBuffers();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
	BvRCRef<IBvBuffer> m_SB1;
	BvRCRef<IBvBuffer> m_SB2;
	BvRCRef<IBvBufferView> m_SBView1;
	BvRCRef<IBvBufferView> m_SBView2;
	BvRCRef<IBvTexture> m_Depth;
	BvRCRef<IBvTextureView> m_DepthView;
	BvRCRef<IBvQuery> m_Query;
	PipelineStatistics m_PSOStats{};

	struct PushConstants
	{
		Float44 viewProj;
		Float3 cameraPos;
		float time;
	} m_PC{};
};