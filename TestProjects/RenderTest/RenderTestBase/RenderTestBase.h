//#pragma once
//
//
//#include "BvCore/System/Window/BvNativeWindow.h"
//#include "BvRender/BvRenderEngine.h"
//#include "BvRender/BvTexture.h"
//
//
//struct PassData
//{
//	BvRenderPass* pRenderPass = nullptr;
//	BvShaderResourceLayout* pShaderLayout = nullptr;
//	BvGraphicsPipelineState* pGraphicsPSO = nullptr;
//	struct FrameData
//	{
//		BvVector<BvTexture*> renderTargets;
//		BvVector<BvTextureView*> renderTargetViews;
//		BvVector<BvShaderResourceParams*> shaderResourceSets;
//		BvCommandPool* pCommandPool;
//		BvCommandBuffer* pCommandBuffer;
//		BvSemaphore* pSemaphore = nullptr;
//		u64 semaphoreValue = 0;
//	};
//	BvVector<FrameData> frameData;
//};
//
//
//class RenderTestBase
//{
//	BV_NOCOPYMOVE(RenderTestBase);
//
//public:
//	RenderTestBase();
//	virtual ~RenderTestBase();
//
//	virtual void Run();
//
//protected:
//	virtual void Init() = 0;
//	virtual void Process() = 0;
//	virtual void Shutdown() = 0;
//
//protected:
//	BvRenderEngine* m_pEngine = nullptr;
//	BvRenderDevice* m_pDevice = nullptr;
//	BvSwapChain* m_pSwapchain = nullptr;
//};