#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class MSAA : public SampleBase
{
public:
	MSAA() {}
	~MSAA() {}

	void OnInitialize() override;
	void OnInitializeUI() override;
	void OnUpdate() override;
	void OnUpdateUI() override;
	void OnRender() override;
	void OnRenderUI() override;
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateBuffers();
	void CreateRenderTargets();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
	BvRCRef<IBvGraphicsPipelineState> m_PSOMSAA;
	BvRCRef<IBvBuffer> m_UB;
	BvRCRef<IBvBuffer> m_VB;
	BvRCRef<IBvBuffer> m_IB;
	BvRCRef<IBvBufferView> m_UBView;
	BvRCRef<IBvTexture> m_ColorMS;
	BvRCRef<IBvTextureView> m_ColorViewMS;
	BvRCRef<IBvTexture> m_Depth;
	BvRCRef<IBvTextureView> m_DepthView;
	BvRCRef<IBvTexture> m_DepthMS;
	BvRCRef<IBvTextureView> m_DepthViewMS;
	Float44* m_pWVP = nullptr;
	bool m_UseMSAA = true;
};