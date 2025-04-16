#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class GeometryShader : public SampleBase
{
public:
	GeometryShader() {}
	~GeometryShader() {}

	void OnInitialize() override;
	void OnInitializeUI() override;
	void OnUpdate() override;
	void OnRender() override;
	void RenderNormals();
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateBuffers();
	void CreateRenderTargets();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
	BvRCRef<IBvBuffer> m_VB;
	BvRCRef<IBvBuffer> m_IB;
	BvRCRef<IBvBuffer> m_UB;
	BvRCRef<IBvBufferView> m_UBView;
	BvRCRef<IBvTexture> m_Depth;
	BvRCRef<IBvTextureView> m_DepthView;
	Float44* m_pWVP = nullptr;

	BvRCRef<IBvGraphicsPipelineState> m_PSONormal;
	BvRCRef<IBvShaderResourceLayout> m_SRLNormal;
	BvRCRef<IBvBuffer> m_UBNormal;
	BvRCRef<IBvBufferView> m_UBViewNormal;
	Float44* m_pNormalData = nullptr;
};