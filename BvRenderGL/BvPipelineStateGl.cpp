#include "BvPipelineStateGl.h"
#include "BvTypeConversionsGl.h"
#include "BvRenderDeviceGl.h"


BvGraphicsPipelineStateGl::BvGraphicsPipelineStateGl(const BvRenderDeviceGl& device, const GraphicsPipelineStateDesc& pipelineStateDesc)
	: BvGraphicsPipelineState(pipelineStateDesc), m_Device(device)
{
}


BvGraphicsPipelineStateGl::~BvGraphicsPipelineStateGl()
{
}


void BvGraphicsPipelineStateGl::SetStates()
{
	const auto& extendedFeatures = m_Device.GetGPUInfo().m_ExtendedFeatures;

	const auto& inputAssembly = m_PipelineStateDesc.m_InputAssemblyStateDesc;
	if (inputAssembly.m_PrimitiveRestart)
	{
		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(inputAssembly.m_IndexFormatForPrimitiveRestart == IndexFormat::kU32 ? 0xFFFFFFFFu : 0xFFFFu);
	}
	else
	{
		glDisable(GL_PRIMITIVE_RESTART);
	}

	const auto& tessState = m_PipelineStateDesc.m_TessellationStateDesc;
	if (tessState.m_PatchControlPoints != 0)
	{
		GLint maxPatchVertices = 0;
		glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);
		glPatchParameteri(GL_PATCH_VERTICES, std::min((i32)tessState.m_PatchControlPoints, maxPatchVertices));
	}

	const auto& rasterState = m_PipelineStateDesc.m_RasterizerStateDesc;
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

	const auto& depthStencil = m_PipelineStateDesc.m_DepthStencilDesc;
	if (depthStencil.m_DepthTestEnable)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
	glDepthMask(depthStencil.m_DepthWriteEnable ? 1 : 0);
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
	const auto& blendState = m_PipelineStateDesc.m_BlendStateDesc;
	for (auto i = 0u; i < blendState.m_BlendAttachments.Size(); i++)
	{
		if (blendState.m_BlendAttachments[i].m_BlendEnable)
		{
			if (!blendEnabled)
			{
				glEnable(GL_BLEND);
				blendEnabled = true;
			}

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
			glSampleMaski(0, m_PipelineStateDesc.m_SampleMask);
		}
		else
		{
			glSampleMaski(0, kU32Max);
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
	}
}


BvComputePipelineStateGl::BvComputePipelineStateGl(const BvRenderDeviceGl& device, const ComputePipelineStateDesc& pipelineStateDesc)
	: BvComputePipelineState(pipelineStateDesc), m_Device(device)
{
}


BvComputePipelineStateGl::~BvComputePipelineStateGl()
{
}


void BvComputePipelineStateGl::SetStates()
{
}