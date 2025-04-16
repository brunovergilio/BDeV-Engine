#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class TessellationShader : public SampleBase
{
public:
	TessellationShader() {}
	~TessellationShader() {}

	void OnInitialize() override;
	void OnUpdate() override;
	void OnUpdateUI() override;
	void OnRender() override;
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
	BvRCRef<IBvGraphicsPipelineState> m_PSOTri;
	i32 m_CurrOption = 0;
	f32 m_TessData[6]{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
};