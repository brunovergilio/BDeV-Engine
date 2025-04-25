#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class RayTracing2 : public SampleBase
{
public:
	RayTracing2() {}
	~RayTracing2() {}

	void OnInitialize() override;
	void OnUpdate() override;
	void OnUpdateUI() override;
	void OnRender() override;
	void OnShutdown() override;

private:
	void CreateShaderResourceLayout();
	void CreatePipeline();
	void CreateResources();
	void CreateBLAS();
	void CreateTLAS();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;

	BvRCRef<IBvShaderResourceLayout> m_RaySRL;
	BvRCRef<IBvRayTracingPipelineState> m_RayPSO;
	BvRCRef<IBvTexture> m_Tex;
	BvRCRef<IBvTextureView> m_TexView;
	BvRCRef<IBvSampler> m_Sampler;
	BvRCRef<IBvAccelerationStructure> m_BLAS;
	BvRCRef<IBvAccelerationStructure> m_TLAS;
	BvRCRef<IBvShaderBindingTable> m_SBT;
	BvRCRef<IBvBuffer> m_VBTri;
	BvRCRef<IBvBuffer> m_IBTri;
	BvRCRef<IBvBuffer> m_UBRayData;
	BvRCRef<IBvBufferView> m_UBViewRayData;
	struct RayData
	{
		Float44 viewInv;
		Float44 projInv;
	} *m_pRayData = nullptr;

	Float3 m_BackColor{};
};