#pragma once


#include "BvCommonGl.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/RenderAPI/BvPipelineState.h"
#include <functional>


class BvRenderDeviceGl;
class BvTextureView;
class BvTextureViewGl;
class BvFramebufferGl;
class BvGraphicsPipelineStateGl;

class BvRenderContextStateGl
{
	BV_NOCOPYMOVE(BvRenderContextStateGl);

public:
	BvRenderContextStateGl(const BvRenderDeviceGl& device);
	~BvRenderContextStateGl();

	void SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors, BvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear, const ClearFlags clearFlags);
	void SetPipeline(BvGraphicsPipelineState* pPipeline);

private:
	const BvRenderDeviceGl& m_Device;
	BvVector<BvTextureViewGl*> m_RenderTargets;
	BvVector<ClearColorValue> m_RenderTargetClearValues;
	BvTextureViewGl* m_pDepthStencilTarget = nullptr;
	ClearColorValue m_DepthStencilTargetClearValue{};
	BvFramebufferGl* m_pFramebuffer = nullptr;
	BvGraphicsPipelineStateGl* m_pGraphicsPipeline = nullptr;

	GraphicsPipelineStateDesc m_CurrGraphicsPipelineStateDesc;

	bool m_FramebufferBindNeeded = false;
};