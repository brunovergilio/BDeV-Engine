#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class Texture3D : public SampleBase
{
public:
	Texture3D() {}
	~Texture3D() {}

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
	void CreateTextures();
	void CreateRenderTargets();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;
	BvRCRef<IBvBuffer> m_UB;
	BvRCRef<IBvBuffer> m_VB;
	BvRCRef<IBvBuffer> m_IB;
	BvRCRef<IBvBufferView> m_UBView;
	BvRCRef<IBvTexture> m_Depth;
	BvRCRef<IBvTextureView> m_DepthView;
	BvRCRef<IBvTexture> m_Texture;
	BvRCRef<IBvTextureView> m_TextureView;
	BvRCRef<IBvSampler> m_Sampler;
	Float44* m_pWVP = nullptr;
	f32 m_CurrDepthSlice = 0.0f;
};