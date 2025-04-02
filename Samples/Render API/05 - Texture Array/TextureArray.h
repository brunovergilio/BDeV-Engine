#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class TextureArray : public SampleBase
{
public:
	TextureArray() {}
	~TextureArray() {}

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
	i32 m_MaxArrayIndex = 0;
	i32 m_CurrArrayIndex = 0;
	f32 m_ZPos = -5.0f;
};