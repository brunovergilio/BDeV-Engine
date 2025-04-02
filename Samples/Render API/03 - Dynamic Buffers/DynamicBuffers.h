#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class DynamicBuffers : public SampleBase
{
public:
	DynamicBuffers() {}
	~DynamicBuffers() {}

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
	size_t m_ItemStride = 0;
	i32 m_ItemCountPerDimension = 3;
	i32 m_MaxItemsPerDimension = 5;
};