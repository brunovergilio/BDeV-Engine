#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class Triangle : public SampleBase
{
public:
	Triangle() {}
	~Triangle() {}

	void OnInitialize() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CompileShaders();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvShader> m_VS;
	BvRCRef<IBvShader> m_PS;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
};