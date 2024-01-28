#include "BvRenderContextStateGl.h"
#include "BvRenderDeviceGl.h"
#include "BvFramebufferGl.h"
#include "BvPipelineStateGl.h"
#include "BvTypeConversionsGl.h"


BvRenderContextStateGl::BvRenderContextStateGl(const BvRenderDeviceGl& device)
	: m_Device(device)
{
	m_CurrGraphicsPipelineStateDesc.m_RasterizerStateDesc.m_CullMode = CullMode::kBack;
	m_CurrGraphicsPipelineStateDesc.m_RasterizerStateDesc.m_FrontFace = FrontFace::kCounterClockwise;
	m_CurrGraphicsPipelineStateDesc.m_DepthStencilDesc.m_DepthWriteEnable = true;
	m_CurrGraphicsPipelineStateDesc.m_DepthStencilDesc.m_DepthOp = CompareOp::kAlways;
	m_CurrGraphicsPipelineStateDesc.m_DepthStencilDesc.m_StencilWriteMask = kU8Max;
}


BvRenderContextStateGl::~BvRenderContextStateGl()
{
}


void BvRenderContextStateGl::SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors, BvTextureView* const pDepthStencilTarget, const ClearColorValue& depthClear, const ClearFlags clearFlags)
{
	//DecommitRenderTargets();

	if (!renderTargetCount && !pDepthStencilTarget)
	{
		return;
	}

	m_RenderTargets.Resize(renderTargetCount);
	m_RenderTargetClearValues.Resize(renderTargetCount);
	for (auto i = 0; i < m_RenderTargets.Size(); i++)
	{
		m_RenderTargets[i] = reinterpret_cast<BvTextureViewGl*>(pRenderTargets[i]);
	}

	m_pDepthStencilTarget = nullptr;
	if (pDepthStencilTarget)
	{
		m_pDepthStencilTarget = reinterpret_cast<BvTextureViewGl*>(pDepthStencilTarget);
		m_DepthStencilTargetClearValue = depthClear;
	}

	//m_RenderTargetsBindNeeded = true;

	FramebufferDesc framebufferDesc;
	for (auto pRenderTarget : m_RenderTargets)
	{
		framebufferDesc.m_RenderTargetViews.PushBack(pRenderTarget);
	}
	if (m_pDepthStencilTarget)
	{
		framebufferDesc.m_pDepthStencilView = m_pDepthStencilTarget;
	}

	auto pFramebufferManager = m_Device.GetFramebufferManager();
	auto pFramebuffer = pFramebufferManager->GetFramebuffer(framebufferDesc);
	if (pFramebuffer != m_pFramebuffer)
	{
		m_FramebufferBindNeeded = true;
		m_pFramebuffer = pFramebuffer;
	}
}


void BvRenderContextStateGl::SetPipeline(BvGraphicsPipelineState* pPipeline)
{
	if (m_pGraphicsPipeline)
	{
		const auto& pipelineStateDesc = m_pGraphicsPipeline->GetDesc();
		const auto& extendedFeatures = m_Device.GetGPUInfo().m_ExtendedFeatures;

		const auto& inputAssembly = pipelineStateDesc.m_InputAssemblyStateDesc;
		if (inputAssembly.m_PrimitiveRestart)
		{
			glEnable(GL_PRIMITIVE_RESTART);
			glPrimitiveRestartIndex(inputAssembly.m_IndexFormatForPrimitiveRestart == IndexFormat::kU32 ? 0xFFFFFFFFu : 0xFFFFu);
		}
		else
		{
			glDisable(GL_PRIMITIVE_RESTART);
		}

		const auto& tessState = pipelineStateDesc.m_TessellationStateDesc;
		if (tessState.m_PatchControlPoints != 0)
		{
			GLint maxPatchVertices = 0;
			glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);
			glPatchParameteri(GL_PATCH_VERTICES, std::min((i32)tessState.m_PatchControlPoints, maxPatchVertices));
		}

		const auto& rasterState = pipelineStateDesc.m_RasterizerStateDesc;
		glFrontFace(GetGlFrontFace(rasterState.m_FrontFace));
		if (rasterState.m_CullMode != CullMode::kNone)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GetGlCullMode(rasterState.m_CullMode));
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (rasterState.m_EnableDepthBias)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			if (extendedFeatures.polygonOffsetClamp)
			{
				glPolygonOffsetClampEXT(rasterState.m_DepthBiasSlope, rasterState.m_DepthBias, rasterState.m_DepthBiasClamp);
			}
			else
			{
				glPolygonOffset(rasterState.m_DepthBiasSlope, rasterState.m_DepthBias);
			}
		}
		else
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
		}

		if (rasterState.m_EnableDepthClip)
		{
			glEnable(GL_DEPTH_CLAMP);
		}
		else
		{
			glDisable(GL_DEPTH_CLAMP);
		}

		if (rasterState.m_EnableConservativeRasterization)
		{
			if (extendedFeatures.nvConservativeRaster)
			{
				glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
			}
			else if (extendedFeatures.intelConservativeRaster)
			{
				glEnable(GL_CONSERVATIVE_RASTERIZATION_INTEL);
			}
		}
		else
		{
			if (extendedFeatures.nvConservativeRaster)
			{
				glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
			}
			else if (extendedFeatures.intelConservativeRaster)
			{
				glDisable(GL_CONSERVATIVE_RASTERIZATION_INTEL);
			}
		}

		const auto& depthStencil = pipelineStateDesc.m_DepthStencilDesc;
		if (depthStencil.m_DepthTestEnable)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		glDepthMask(depthStencil.m_DepthWriteEnable ? GL_TRUE : GL_FALSE);
		glDepthFunc(GetGlCompareOp(depthStencil.m_DepthOp));
		if (depthStencil.m_StencilTestEnable)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
		glStencilMask(depthStencil.m_StencilWriteMask);
		//glStencilMask(depthStencil.m_StencilReadMask);
		if (depthStencil.m_DepthBoundsTestEnable)
		{
			if (extendedFeatures.depthBoundsTest)
			{
				glEnable(GL_DEPTH_BOUNDS_TEST_EXT);
			}
		}
		else
		{
			if (extendedFeatures.depthBoundsTest)
			{
				glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
			}
		}
		if (depthStencil.m_StencilTestEnable)
		{
			//glStencilFuncSeparate(GL_FRONT, GetGlCompareOp(depthStencil.m_StencilFront.m_StencilFunc), 0, depthStencil.m_StencilReadMask);
			glStencilOpSeparate(GL_FRONT, GetGlStencilOp(depthStencil.m_StencilFront.m_StencilFailOp),
				GetGlStencilOp(depthStencil.m_StencilFront.m_StencilDepthFailOp), GetGlStencilOp(depthStencil.m_StencilFront.m_StencilPassOp));
			glStencilOpSeparate(GL_BACK, GetGlStencilOp(depthStencil.m_StencilBack.m_StencilFailOp),
				GetGlStencilOp(depthStencil.m_StencilBack.m_StencilDepthFailOp), GetGlStencilOp(depthStencil.m_StencilBack.m_StencilPassOp));
		}

		bool blendEnabled = false;
		const auto& blendState = pipelineStateDesc.m_BlendStateDesc;
		for (auto i = 0u; i < blendState.m_BlendAttachments.Size(); i++)
		{
			if (blendState.m_BlendAttachments[i].m_BlendEnable)
			{
				if (!blendEnabled)
				{
					glEnable(GL_BLEND);
					blendEnabled = true;
				}

				glEnablei(GL_BLEND, i);
				auto srcBlend = GetGlBlendFactor(blendState.m_BlendAttachments[i].m_SrcBlend);
				auto dstBlend = GetGlBlendFactor(blendState.m_BlendAttachments[i].m_DstBlend);
				auto srcBlendAlpha = GetGlBlendFactor(blendState.m_BlendAttachments[i].m_SrcBlendAlpha);
				auto dstBlendAlpha = GetGlBlendFactor(blendState.m_BlendAttachments[i].m_DstBlendAlpha);
				glBlendFuncSeparatei(i, srcBlend, dstBlend, srcBlendAlpha, dstBlendAlpha);

				auto blendOp = GetGlBlendOp(blendState.m_BlendAttachments[i].m_BlendOp);
				auto alphaBlendOp = GetGlBlendOp(blendState.m_BlendAttachments[i].m_AlphaBlendOp);
				glBlendEquationSeparatei(i, blendOp, alphaBlendOp);
			}

			glColorMaski(i,
				(blendState.m_BlendAttachments[i].m_RenderTargetWriteMask & (u8)ColorMask::kRed) ? GL_TRUE : GL_FALSE,
				(blendState.m_BlendAttachments[i].m_RenderTargetWriteMask & (u8)ColorMask::kGreen) ? GL_TRUE : GL_FALSE,
				(blendState.m_BlendAttachments[i].m_RenderTargetWriteMask & (u8)ColorMask::kBlue) ? GL_TRUE : GL_FALSE,
				(blendState.m_BlendAttachments[i].m_RenderTargetWriteMask & (u8)ColorMask::kAlpha) ? GL_TRUE : GL_FALSE);
		}
		if (!blendEnabled)
		{
			glDisable(GL_BLEND);
		}

		if (blendEnabled)
		{
			if (blendState.m_AlphaToCoverageEnable)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				glSampleMaski(0, pipelineStateDesc.m_SampleMask);
			}
			else
			{
				glSampleMaski(0, kU32Max);
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
		}
	}
	else
	{

	}
}