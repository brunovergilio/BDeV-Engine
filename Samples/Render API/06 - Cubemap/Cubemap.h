#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class Cubemap : public SampleBase
{
public:
	Cubemap() {}
	~Cubemap() {}

	void OnInitialize() override;
	void OnInitializeUI() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;
	void RenderSkybox();

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateBuffers();
	void CreateTextures();
	void CreateRenderTargets();
	void CreateSkyboxResources();

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

	BvRCRef<IBvBuffer> m_UBSkybox;
	BvRCRef<IBvBufferView> m_UBViewSkybox;
	BvRCRef<IBvTexture> m_TextureSkybox;
	BvRCRef<IBvTextureView> m_TextureViewSkybox;
	BvRCRef<IBvSampler> m_SamplerSkybox;
	BvRCRef<IBvGraphicsPipelineState> m_PSOSkybox;

	Float44* m_pWVP = nullptr;
	Float44* m_pWVPSkybox = nullptr;
};