#include "BvPipelineStateVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvRenderPassVk.h"
#include "BvShaderResourceVk.h"
#include "BvUtilsVk.h"


VkShaderModule CreateShaderModule(VkDevice device, size_t size, const u8* pShaderCode, VkShaderStageFlagBits shaderStage);


BvGraphicsPipelineStateVk::BvGraphicsPipelineStateVk(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc, const VkPipelineCache pipelineCache)
	: BvGraphicsPipelineState(pipelineStateDesc), m_pDevice(pDevice), m_PipelineCache(pipelineCache)
{
	Create();
}


BvGraphicsPipelineStateVk::~BvGraphicsPipelineStateVk()
{
	Destroy();
}


BvRenderDevice* BvGraphicsPipelineStateVk::GetDevice()
{
	return m_pDevice;
}


void BvGraphicsPipelineStateVk::Create()
{
	BvFixedVector<VkVertexInputAttributeDescription, kMaxVertexBindings> attributeDescs(m_PipelineStateDesc.m_VertexInputDesc.Size(), {});
	BvFixedVector<VkVertexInputBindingDescription, kMaxVertexBindings> bindingDescs;
	BvFixedVector<VkVertexInputBindingDivisorDescriptionEXT, kMaxVertexBindings> divisorDescs;
	BvFixedVector<u32, kMaxVertexBindings> bindingIndices(kMaxVertexBindings, kU32Max);
	for (auto i = 0u; i < attributeDescs.Size(); i++)
	{
		attributeDescs[i].location = m_PipelineStateDesc.m_VertexInputDesc[i].m_Location;
		attributeDescs[i].binding = m_PipelineStateDesc.m_VertexInputDesc[i].m_Binding;
		attributeDescs[i].format = GetVkFormat(m_PipelineStateDesc.m_VertexInputDesc[i].m_Format);
		attributeDescs[i].offset = m_PipelineStateDesc.m_VertexInputDesc[i].m_Offset;

		// We keep track of each binding that hasn't already been set, and add the elements
		// when a new one is found (which means bindingIndex == kU32Max)
		auto& bindingIndex = bindingIndices[m_PipelineStateDesc.m_VertexInputDesc[i].m_Binding];
		if (bindingIndex == kU32Max)
		{
			bindingDescs.PushBack({ m_PipelineStateDesc.m_VertexInputDesc[i].m_Binding, m_PipelineStateDesc.m_VertexInputDesc[i].m_Stride,
				GetVkVertexInputRate(m_PipelineStateDesc.m_VertexInputDesc[i].m_InputRate) });
			bindingIndex = bindingDescs.Size() - 1;
		}

		if (m_PipelineStateDesc.m_VertexInputDesc[i].m_InputRate == InputRate::kPerInstance)
		{
			divisorDescs.PushBack({ m_PipelineStateDesc.m_VertexInputDesc[i].m_Binding, m_PipelineStateDesc.m_VertexInputDesc[i].m_InstanceRate});
		}
	}

	VkPipelineVertexInputDivisorStateCreateInfoEXT vertexDivisorStateCI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT };
	vertexDivisorStateCI.vertexBindingDivisorCount = divisorDescs.Size();
	vertexDivisorStateCI.pVertexBindingDivisors = divisorDescs.Data();

	VkPipelineVertexInputStateCreateInfo vertexCI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexCI.pNext = m_pDevice->GetDeviceInfo()->m_EnabledExtensions.Find(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME) != kU64Max && divisorDescs.Size() > 0 ? &vertexDivisorStateCI : nullptr;
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

	VkPipelineInputAssemblyStateCreateInfo iaCI{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	iaCI.primitiveRestartEnable = m_PipelineStateDesc.m_InputAssemblyStateDesc.m_PrimitiveRestart;
	iaCI.topology = GetVkPrimitiveTopology(m_PipelineStateDesc.m_InputAssemblyStateDesc.m_Topology);

	VkPipelineTessellationStateCreateInfo tessCI{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
	tessCI.patchControlPoints = m_PipelineStateDesc.m_TessellationStateDesc.m_PatchControlPoints;

	VkPipelineViewportStateCreateInfo viewportCI{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	//viewportCI.viewportCount = m_PipelineStateDesc.m_ViewportStateDesc.m_MaxViewportCount;
	//viewportCI.scissorCount = m_PipelineStateDesc.m_ViewportStateDesc.m_MaxScissorCount;

	VkPipelineRasterizationStateCreateInfo rasterizerCI{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizerCI.polygonMode = GetVkPolygonMode(m_PipelineStateDesc.m_RasterizerStateDesc.m_FillMode);
	rasterizerCI.cullMode = GetVkCullModeFlags(m_PipelineStateDesc.m_RasterizerStateDesc.m_CullMode);
	rasterizerCI.frontFace = GetVkFrontFace(m_PipelineStateDesc.m_RasterizerStateDesc.m_FrontFace);
	rasterizerCI.depthBiasEnable = m_PipelineStateDesc.m_RasterizerStateDesc.m_EnableDepthBias;
	rasterizerCI.depthBiasConstantFactor = m_PipelineStateDesc.m_RasterizerStateDesc.m_DepthBias;
	rasterizerCI.depthBiasClamp = m_PipelineStateDesc.m_RasterizerStateDesc.m_DepthBiasClamp;
	rasterizerCI.depthBiasSlopeFactor = m_PipelineStateDesc.m_RasterizerStateDesc.m_DepthBiasSlope;
	rasterizerCI.depthClampEnable = m_PipelineStateDesc.m_RasterizerStateDesc.m_EnableDepthClip;
	rasterizerCI.lineWidth = 1.0f;

	VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClip{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT };
	if (m_pDevice->GetDeviceInfo()->m_ExtendedFeatures.depthClibEnableFeature.depthClipEnable)
	{
		depthClip.depthClipEnable = VK_TRUE;
		rasterizerCI.pNext = &depthClip;
	}

	VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRaster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT };
	if (m_pDevice->GetDeviceInfo()->m_EnabledExtensions.Find(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME) != kU64Max && m_PipelineStateDesc.m_RasterizerStateDesc.m_EnableConservativeRasterization)
	{
		const auto & conservativeRasterProps = m_pDevice->GetDeviceInfo()->m_ExtendedProperties.convervativeRasterizationProps;

		if (conservativeRasterProps.maxExtraPrimitiveOverestimationSize > 0.0f)
		{
			conservativeRaster.conservativeRasterizationMode =
				VkConservativeRasterizationModeEXT::VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
			conservativeRaster.extraPrimitiveOverestimationSize = conservativeRasterProps.maxExtraPrimitiveOverestimationSize;

			rasterizerCI.pNext = &conservativeRaster;
		}
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilCI{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
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
	//depthStencilCI.front.reference = 0; // Set dynamically

	depthStencilCI.back.failOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilFailOp);
	depthStencilCI.back.depthFailOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilDepthFailOp);
	depthStencilCI.back.passOp = GetVkStencilOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilPassOp);
	depthStencilCI.back.compareOp = GetVkCompareOp(m_PipelineStateDesc.m_DepthStencilDesc.m_StencilBack.m_StencilFunc);
	depthStencilCI.back.writeMask = m_PipelineStateDesc.m_DepthStencilDesc.m_StencilWriteMask;
	depthStencilCI.back.compareMask = m_PipelineStateDesc.m_DepthStencilDesc.m_StencilReadMask;
	//depthStencilCI.back.reference = 0; // Set dynamically

	depthStencilCI.depthBoundsTestEnable = m_PipelineStateDesc.m_DepthStencilDesc.m_DepthBoundsTestEnable;
	depthStencilCI.minDepthBounds = 0.0f;
	depthStencilCI.maxDepthBounds = 1.0f;

	u32 sampleMask[] = { m_PipelineStateDesc.m_SampleMask, 0 };
	VkPipelineMultisampleStateCreateInfo msCI{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	msCI.rasterizationSamples = GetVkSampleCountFlagBits(m_PipelineStateDesc.m_SampleCount);
	msCI.pSampleMask = sampleMask;

	BvFixedVector<VkFormat, kMaxRenderTargets> rtvFormats(m_PipelineStateDesc.m_RenderTargetFormats.Size());
	for (auto i = 0; i < rtvFormats.Size(); i++)
	{
		rtvFormats[i] = GetVkFormat(m_PipelineStateDesc.m_RenderTargetFormats[i]);
	}

	VkPipelineRenderingCreateInfoKHR pipelineRenderingCI{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
	VkRenderPass renderPass = m_PipelineStateDesc.m_pRenderPass ? static_cast<BvRenderPassVk*>(m_PipelineStateDesc.m_pRenderPass)->GetHandle() : VK_NULL_HANDLE;
	if (renderPass == VK_NULL_HANDLE)
	{
		pipelineRenderingCI.colorAttachmentCount = m_PipelineStateDesc.m_RenderTargetFormats.Size();
		pipelineRenderingCI.pColorAttachmentFormats = rtvFormats.Data();
		pipelineRenderingCI.depthAttachmentFormat = GetVkFormat(m_PipelineStateDesc.m_DepthStencilFormat);
		pipelineRenderingCI.stencilAttachmentFormat = pipelineRenderingCI.depthAttachmentFormat;
	}

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
		blendAttachments[i].alphaBlendOp = GetVkBlendOp(m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_AlphaBlendOp);
		blendAttachments[i].colorWriteMask = m_PipelineStateDesc.m_BlendStateDesc.m_BlendAttachments[i].m_RenderTargetWriteMask;
	}

	VkPipelineColorBlendStateCreateInfo blendCI{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	if (blendAttachments.Size() > 0)
	{
		blendCI.attachmentCount = (u32)blendAttachments.Size();
		blendCI.pAttachments = blendAttachments.Data();
	}
	blendCI.logicOpEnable = m_PipelineStateDesc.m_BlendStateDesc.m_LogicEnable;
	blendCI.logicOp = GetVkLogicOp(m_PipelineStateDesc.m_BlendStateDesc.m_LogicOp);

	constexpr u32 kMaxDynamicStates = 8; // Change as needed
	BvFixedVector<VkDynamicState, kMaxDynamicStates> dynamicStates{};
	dynamicStates.PushBack(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
	dynamicStates.PushBack(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
	dynamicStates.PushBack(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
	dynamicStates.PushBack(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	if (m_pDevice->GetDeviceInfo()->m_DeviceFeatures.features.depthBounds)
	{
		dynamicStates.PushBack(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
	}
	if (m_pDevice->GetDeviceInfo()->m_EnabledExtensions.Find(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) != kU64Max)
	{
		dynamicStates.PushBack(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
	}

	VkPipelineDynamicStateCreateInfo dynamicStateCI{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateCI.dynamicStateCount = (u32)dynamicStates.Size();
	dynamicStateCI.pDynamicStates = dynamicStates.Data();

	BvFixedVector<VkPipelineShaderStageCreateInfo, kMaxShaderStages> shaderStages(m_PipelineStateDesc.m_Shaders.Size(), {});
	for (auto i = 0u; i < shaderStages.Size(); i++)
	{
		const auto& byteCode = m_PipelineStateDesc.m_Shaders[i]->GetShaderBlob();
		shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[i].pName = m_PipelineStateDesc.m_Shaders[i]->GetEntryPoint();
		shaderStages[i].stage = GetVkShaderStageFlagBits(m_PipelineStateDesc.m_Shaders[i]->GetShaderStage());
		shaderStages[i].module = CreateShaderModule(m_pDevice->GetHandle(), byteCode.Size(),
			byteCode.Data(), shaderStages[i].stage);
	}

	VkGraphicsPipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineCI.pNext = !m_PipelineStateDesc.m_pRenderPass ? &pipelineRenderingCI : nullptr;
	//pipelineCI.flags = 0;
	pipelineCI.stageCount = (u32)m_PipelineStateDesc.m_Shaders.Size();
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
	pipelineCI.renderPass = renderPass;
	pipelineCI.layout = static_cast<BvShaderResourceLayoutVk*>(m_PipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
	pipelineCI.subpass = m_PipelineStateDesc.m_SubpassIndex;
	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.basePipelineIndex = -1;

	auto result = vkCreateGraphicsPipelines(m_pDevice->GetHandle(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipeline);

	for (auto i = 0u; i < m_PipelineStateDesc.m_Shaders.Size(); i++)
	{
		vkDestroyShaderModule(m_pDevice->GetHandle(), shaderStages[i].module, nullptr);
	}
}


void BvGraphicsPipelineStateVk::Destroy()
{
	if (m_Pipeline)
	{
		vkDestroyPipeline(m_pDevice->GetHandle(), m_Pipeline, nullptr);
		m_Pipeline = nullptr;
		m_PipelineCache = nullptr;
	}
}


BvComputePipelineStateVk::BvComputePipelineStateVk(BvRenderDeviceVk* pDevice, const ComputePipelineStateDesc & pipelineStateDesc,
	const VkPipelineCache pipelineCache)
	: BvComputePipelineState(pipelineStateDesc), m_pDevice(pDevice), m_PipelineCache(pipelineCache)
{
	Create();
}


BvComputePipelineStateVk::~BvComputePipelineStateVk()
{
	Destroy();
}


BvRenderDevice* BvComputePipelineStateVk::GetDevice()
{
	return m_pDevice;
}


void BvComputePipelineStateVk::Create()
{
	VkComputePipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
	pipelineCI.layout = static_cast<BvShaderResourceLayoutVk *>(m_PipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
	pipelineCI.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineCI.stage.pName = m_PipelineStateDesc.m_pShader->GetEntryPoint();
	pipelineCI.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	auto& blob = m_PipelineStateDesc.m_pShader->GetShaderBlob();
	pipelineCI.stage.module = CreateShaderModule(m_pDevice->GetHandle(), blob.Size(), blob.Data(), pipelineCI.stage.stage);
	pipelineCI.basePipelineIndex = -1;

	auto result = vkCreateComputePipelines(m_pDevice->GetHandle(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipeline);

	vkDestroyShaderModule(m_pDevice->GetHandle(), pipelineCI.stage.module, nullptr);
}


void BvComputePipelineStateVk::Destroy()
{
	if (m_Pipeline)
	{
		vkDestroyPipeline(m_pDevice->GetHandle(), m_Pipeline, nullptr);
		m_Pipeline = nullptr;
		m_PipelineCache = nullptr;
	}
}


VkShaderModule CreateShaderModule(VkDevice device, size_t size, const u8* pShaderCode, VkShaderStageFlagBits shaderStage)
{
	VkShaderModuleCreateInfo shaderCI{};
	shaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCI.pCode = reinterpret_cast<const u32 *>(pShaderCode);
	shaderCI.codeSize = size;

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	vkCreateShaderModule(device, &shaderCI, nullptr, &shaderModule);

	return shaderModule;
}