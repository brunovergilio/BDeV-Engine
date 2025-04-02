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

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
};