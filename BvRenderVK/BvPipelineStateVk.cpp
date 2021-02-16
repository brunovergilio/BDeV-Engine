#include "BvPipelineStateVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvRenderPassVk.h"
#include "BvShaderResourceVk.h"


VkShaderModule CreateShaderModule(const BvRenderDeviceVk & device, size_t size, const u8 * pShaderCode, const VkShaderStageFlagBits shaderStage);


BvGraphicsPipelineStateVk::BvGraphicsPipelineStateVk(const BvRenderDeviceVk & device, const GraphicsPipelineStateDesc & pipelineStateDesc, const VkPipelineCache pipelineCache)
	: BvGraphicsPipelineState(pipelineStateDesc), m_Device(device), m_PipelineCache(pipelineCache)
{
}


BvGraphicsPipelineStateVk::~BvGraphicsPipelineStateVk()
{
	Destroy();
}


void BvGraphicsPipelineStateVk::Create()
{
	BvFixedVector<VkVertexInputBindingDescription, kMaxVertexBindings> bindingDescs(m_PipelineStateDesc.m_VertexBindingDesc.Size(), {});
	for (auto i = 0u; i < bindingDescs.Size(); i++)
	{
		bindingDescs[i].binding = m_PipelineStateDesc.m_VertexBindingDesc[i].m_Binding;
		bindingDescs[i].stride = m_PipelineStateDesc.m_VertexBindingDesc[i].m_Stride;
		bindingDescs[i].inputRate = GetVkVertexInputRate(m_PipelineStateDesc.m_VertexBindingDesc[i].m_InputRate);
	}

	BvFixedVector<VkVertexInputAttributeDescription, kMaxVertexBindings> attributeDescs(m_PipelineStateDesc.m_VertexAttributeDesc.Size(), {});
	for (auto i = 0u; i < attributeDescs.Size(); i++)
	{
		attributeDescs[i].binding = m_PipelineStateDesc.m_VertexAttributeDesc[i].m_Binding;
		attributeDescs[i].format = GetVkFormat(m_PipelineStateDesc.m_VertexAttributeDesc[i].m_Format);
		attributeDescs[i].location = m_PipelineStateDesc.m_VertexAttributeDesc[i].m_Location;
		attributeDescs[i].offset = m_PipelineStateDesc.m_VertexAttributeDesc[i].m_Offset;
	}

	VkPipelineVertexInputStateCreateInfo vertexCI{};
	vertexCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (bindingDescs.Size() > 0)
	{
		vertexCI.vertexBindingDescriptionCount = (u32)bindingDescs.Size();
		vertexCI.pVertexBindingDescriptions = bindingDescs.Data();
	}
	if (attributeDescs.Size() > 0)
	{
		vertexCI.vertexAttributeDescriptionCount = (u32)attributeDescs.Size();
		vertexCI.pVertexAttributeDescriptions = attributeDescs.Data();
	}

	VkPipelineInputAssemblyStateCreateInfo iaCI{};
	iaCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaCI.primitiveRestartEnable = m_PipelineStateDesc.m_InputAssemblyStateDesc.m_PrimitiveRestart;
	iaCI.topology = GetVkPrimitiveTopology(m_PipelineStateDesc.m_InputAssemblyStateDesc.m_Topology);

	VkPipelineTessellationStateCreateInfo tessCI{};
	tessCI.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessCI.patchControlPoints = m_PipelineStateDesc.m_TessellationStateDesc.m_PatchControlPoints;

	VkPipelineViewportStateCreateInfo viewportCI{};
	viewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCI.viewportCount = m_PipelineStateDesc.m_ViewportStateDesc.m_MaxViewportCount;
	viewportCI.scissorCount = m_PipelineStateDesc.m_ViewportStateDesc.m_MaxScissorCount;

	VkPipelineRasterizationStateCreateInfo rasterizerCI{};
	rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCI.depthBiasConstantFactor = m_PipelineStateDesc.m_RasterizerStateDesc.m_DepthBias;
	rasterizerCI.depthBiasClamp = m_PipelineStateDesc.m_RasterizerStateDesc.m_DepthBiasClamp;
	rasterizerCI.depthBiasSlopeFactor = m_PipelineStateDesc.m_RasterizerStateDesc.m_DepthBiasSlope;
	rasterizerCI.depthBiasEnable = m_PipelineStateDesc.m_RasterizerStateDesc.m_EnableDepthClip;
	rasterizerCI.depthClampEnable = m_PipelineStateDesc.m_RasterizerStateDesc.m_EnableDepthClamp;
	rasterizerCI.polygonMode = GetVkPolygonMode(m_PipelineStateDesc.m_RasterizerStateDesc.m_FillMode);
	rasterizerCI.cullMode = GetVkCullModeFlags(m_PipelineStateDesc.m_RasterizerStateDesc.m_CullMode);
	rasterizerCI.frontFace = GetVkFrontFace(m_PipelineStateDesc.m_RasterizerStateDesc.m_FrontFace);
	rasterizerCI.lineWidth = 1.0f;

	VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRaster{};
	if (m_PipelineStateDesc.m_RasterizerStateDesc.m_EnableConservativeRasterization)
	{
		const auto & conservativeRasterProps = m_Device.GetGPUInfo().m_Extensions.convervativeRasterizationProps;

		if (conservativeRasterProps.maxExtraPrimitiveOverestimationSize > 0.0f)
		{
			conservativeRaster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
			conservativeRaster.conservativeRasterizationMode =
				VkConservativeRasterizationModeEXT::VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
			conservativeRaster.extraPrimitiveOverestimationSize = conservativeRasterProps.maxExtraPrimitiveOverestimationSize;

			rasterizerCI.pNext = &conservativeRaster;
		}
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilCI{};
	depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCI.depthTestEnable = m_PipelineStateDesc.m_DepthStencilDesc.m_DepthTestEnable;
	depthStencilCI.depthWriteEnable = m_PipelineStateDesc.m_DepthStencilDesc.m_DepthWriteEnable;
	depthStencilCI.depthCompareOp = GetVkCompareOp(m_PipelineStateDesc.m_DepthStencilDesc.m_DepthOp);
	depthStencilCI.stencilTestEnable = m_PipelineStateDesc.m_DepthStencilDesc.m_StencilTestEnable;

	depthStencilCI.front.failOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilFailOp);
	depthStencilCI.front.depthFailOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilDepthFailOp);
	depthStencilCI.front.passOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilPassOp);
	depthStencilCI.front.compareOp = GetVkCompareOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilFront.m_StencilFunc);
	depthStencilCI.front.writeMask = m_PipelineStateDesc.m_DepthStencilDesc.m_StencilWriteMask;
	depthStencilCI.front.compareMask = m_PipelineStateDesc.m_DepthStencilDesc.m_StencilReadMask;

	depthStencilCI.back.failOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilFailOp);
	depthStencilCI.back.depthFailOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilDepthFailOp);
	depthStencilCI.back.passOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilPassOp);
	depthStencilCI.back.compareOp = GetVkCompareOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilFunc);
	depthStencilCI.back.writeMask = m_PipelineStateDesc.m_DepthStencilDesc.m_StencilWriteMask;
	depthStencilCI.back.compareMask = m_PipelineStateDesc.m_DepthStencilDesc.m_StencilReadMask;

	depthStencilCI.depthBoundsTestEnable = m_PipelineStateDesc.m_DepthStencilDesc.m_DepthBoundsTestEnable;
	depthStencilCI.minDepthBounds = 0.0f;
	depthStencilCI.maxDepthBounds = 1.0f;

	VkPipelineMultisampleStateCreateInfo msCI{};
	msCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msCI.rasterizationSamples = GetVkSampleCountFlagBits(m_PipelineStateDesc.m_MultisampleStateDesc.m_SampleCount);

	BvFixedVector<VkPipelineColorBlendAttachmentState, kMaxRenderTargets>
		blendAttachments(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments.Size(), {});
	for (auto i = 0u; i < blendAttachments.Size(); i++)
	{
		blendAttachments[i].blendEnable = m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_BlendEnable;
		blendAttachments[i].srcColorBlendFactor = GetVkBlendFactor(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_SrcBlend);
		blendAttachments[i].srcAlphaBlendFactor = GetVkBlendFactor(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_SrcBlendAlpha);
		blendAttachments[i].colorBlendOp = GetVkBlendOp(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_BlendOp);
		blendAttachments[i].dstColorBlendFactor = GetVkBlendFactor(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_DstBlend);
		blendAttachments[i].dstAlphaBlendFactor = GetVkBlendFactor(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_DstBlendAlpha);
		blendAttachments[i].alphaBlendOp = GetVkBlendOp(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_BlendOpAlpha);
		blendAttachments[i].colorWriteMask = m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_RenderTargetWriteMask;
	}

	VkPipelineColorBlendStateCreateInfo blendCI{};
	blendCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	if (blendAttachments.Size() > 0)
	{
		blendCI.attachmentCount = (u32)blendAttachments.Size();
		blendCI.pAttachments = blendAttachments.Data();
	}
	blendCI.logicOpEnable = m_PipelineStateDesc.m_BlendStateDesc.m_LogicEnable;
	blendCI.logicOp = GetVkLogicOp(m_PipelineStateDesc.m_BlendStateDesc.m_LogicOp);

	constexpr u32 kMaxDynamicStates = 3; // Change as needed
	BvFixedVector<VkDynamicState, kMaxDynamicStates> dynamicStates{};
	dynamicStates.PushBack(VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.PushBack(VkDynamicState::VK_DYNAMIC_STATE_SCISSOR);
	if (m_PipelineStateDesc.m_DepthStencilDesc.m_StencilTestEnable)
	{
		dynamicStates.PushBack(VkDynamicState::VK_DYNAMIC_STATE_STENCIL_REFERENCE);
	}

	VkPipelineDynamicStateCreateInfo dynamicStateCI{};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.dynamicStateCount = (u32)dynamicStates.Size();
	dynamicStateCI.pDynamicStates = dynamicStates.Data();

	BvFixedVector<VkPipelineShaderStageCreateInfo, kMaxShaderStages> shaderStages(m_PipelineStateDesc.m_ShaderStages.Size(), {});
	for (auto i = 0u; i < shaderStages.Size(); i++)
	{
		shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[i].pName = m_PipelineStateDesc.m_ShaderStages[i].m_EntryPoint.CStr();
		shaderStages[i].stage = GetVkShaderStageFlagBits(m_PipelineStateDesc.m_ShaderStages[i].m_ShaderStage);
		shaderStages[i].module = CreateShaderModule(m_Device, m_PipelineStateDesc.m_ShaderStages[i].m_ByteCodeSize,
			m_PipelineStateDesc.m_ShaderStages[i].m_pByteCode, shaderStages[i].stage);
	}

	VkGraphicsPipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	//pipelineCI.pNext = nullptr;
	//pipelineCI.flags = 0;
	pipelineCI.stageCount = (u32)m_PipelineStateDesc.m_ShaderStages.Size();
	pipelineCI.pStages = shaderStages.Data();
	pipelineCI.pVertexInputState = &vertexCI;
	pipelineCI.pInputAssemblyState = &iaCI;
	pipelineCI.pTessellationState = m_PipelineStateDesc.m_TessellationStateDesc.m_PatchControlPoints > 0 ? &tessCI : nullptr;
	pipelineCI.pViewportState = &viewportCI;
	pipelineCI.pRasterizationState = &rasterizerCI;
	pipelineCI.pMultisampleState = &msCI;
	pipelineCI.pDepthStencilState = &depthStencilCI;
	pipelineCI.pColorBlendState = &blendCI;
	pipelineCI.pDynamicState = &dynamicStateCI;
	pipelineCI.renderPass = static_cast<BvRenderPassVk *>(m_PipelineStateDesc.m_pRenderPass)->GetHandle();
	pipelineCI.layout = static_cast<BvShaderResourceLayoutVk *>(m_PipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
	pipelineCI.subpass = 0;
	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.basePipelineIndex = -1;

	auto result = m_Device.GetDeviceFunctions().vkCreateGraphicsPipelines(m_Device.GetHandle(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipeline);

	for (auto i = 0u; i < m_PipelineStateDesc.m_ShaderStages.Size(); i++)
	{
		m_Device.GetDeviceFunctions().vkDestroyShaderModule(m_Device.GetHandle(), shaderStages[i].module, nullptr);
	}
}


void BvGraphicsPipelineStateVk::Destroy()
{
	if (m_Pipeline)
	{
		m_Device.GetDeviceFunctions().vkDestroyPipeline(m_Device.GetHandle(), m_Pipeline, nullptr);
		m_Pipeline = nullptr;
		m_PipelineCache = nullptr;
	}
}


BvComputePipelineStateVk::BvComputePipelineStateVk(const BvRenderDeviceVk & device, const ComputePipelineStateDesc & pipelineStateDesc,
	const VkPipelineCache pipelineCache)
	: BvComputePipelineState(pipelineStateDesc), m_Device(device), m_PipelineCache(pipelineCache)
{
}


BvComputePipelineStateVk::~BvComputePipelineStateVk()
{
	Destroy();
}


void BvComputePipelineStateVk::Create()
{
	VkComputePipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineCI.layout = static_cast<BvShaderResourceLayoutVk *>(m_PipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
	pipelineCI.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineCI.stage.pName = m_PipelineStateDesc.m_ShaderByteCodeDesc.m_EntryPoint.CStr();
	pipelineCI.stage.stage = GetVkShaderStageFlagBits(m_PipelineStateDesc.m_ShaderByteCodeDesc.m_ShaderStage);
	pipelineCI.stage.module = CreateShaderModule(m_Device, m_PipelineStateDesc.m_ShaderByteCodeDesc.m_ByteCodeSize,
	m_PipelineStateDesc.m_ShaderByteCodeDesc.m_pByteCode, pipelineCI.stage.stage);
	pipelineCI.basePipelineIndex = -1;

	auto result = m_Device.GetDeviceFunctions().vkCreateComputePipelines(m_Device.GetHandle(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipeline);

	m_Device.GetDeviceFunctions().vkDestroyShaderModule(m_Device.GetHandle(), pipelineCI.stage.module, nullptr);
}


void BvComputePipelineStateVk::Destroy()
{
	if (m_Pipeline)
	{
		m_Device.GetDeviceFunctions().vkDestroyPipeline(m_Device.GetHandle(), m_Pipeline, nullptr);
		m_Pipeline = nullptr;
		m_PipelineCache = nullptr;
	}
}


VkShaderModule CreateShaderModule(const BvRenderDeviceVk & device, size_t size, const u8 * pShaderCode,
	const VkShaderStageFlagBits shaderStage)
{
	VkShaderModuleCreateInfo shaderCI{};
	shaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCI.pCode = reinterpret_cast<const u32 *>(pShaderCode);
	shaderCI.codeSize = size;

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	device.GetDeviceFunctions().vkCreateShaderModule(device.GetHandle(), &shaderCI, nullptr, &shaderModule);

	return shaderModule;
}