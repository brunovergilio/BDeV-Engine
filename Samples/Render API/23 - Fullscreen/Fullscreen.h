#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class Fullscreen : public SampleBase
{
public:
	Fullscreen() {}
	~Fullscreen() {}

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

	SwapChainMode m_CurrentMode = SwapChainMode::kWindowed;
	BvMonitor* m_pMonitor = nullptr;
	i32 m_CurrentMonitor = 0;
	bool m_ChangesMade = false;
	bool m_VSync = false;
};