#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class RayTracing4 : public SampleBase
{
public:
	RayTracing4() {}
	~RayTracing4() {}

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
	void UpdateTLAS();

private:
	BvRCRef<IBvShaderResourceLayout> m_SRL;
	BvRCRef<IBvGraphicsPipelineState> m_PSO;

	BvRCRef<IBvShaderResourceLayout> m_RaySRL;
	BvRCRef<IBvRayTracingPipelineState> m_RayPSO;
	BvRCRef<IBvTexture> m_Tex;
	BvRCRef<IBvTextureView> m_TexView;
	BvRCRef<IBvSampler> m_Sampler;
	BvRCRef<IBvAccelerationStructure> m_BLAS[2];
	BvRCRef<IBvAccelerationStructure> m_TLAS;
	BvRCRef<IBvShaderBindingTable> m_SBT;
	BvRCRef<IBvBuffer> m_ScratchTLAS;
	BvRCRef<IBvBuffer> m_VB;
	BvRCRef<IBvBuffer> m_IB;
	BvRCRef<IBvBufferView> m_VBView;
	BvRCRef<IBvBufferView> m_IBView;

	BvRCRef<IBvBuffer> m_UBRayData;
	BvRCRef<IBvBufferView> m_UBViewRayData;
	struct RayData
	{
		Float44 viewInv;
		Float44 projInv;
	} *m_pRayData = nullptr;
	TLASInstanceDesc m_Instances[2];

	BvRCRef<IBvBuffer> m_UBHitData;
	BvRCRef<IBvBufferView> m_UBViewHitData;
	struct HitData
	{
		Float44 world[2];
		Float3 lightDir;
	} *m_pHitData = nullptr;

	Float3 m_BackColor{};
	BvMatrix m_WorldPos[2]{ BvMatrix::Identity(), BvMatrix::Identity() };

	bool m_Animate = true;
};