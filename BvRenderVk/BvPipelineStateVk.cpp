#include "BvPipelineStateVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvRenderPassVk.h"
#include "BvShaderResourceVk.h"
#include "BvUtilsVk.h"
#include "BvShaderResourceVk.h"


VkShaderModule CreateShaderModule(VkDevice device, size_t size, const u8* pShaderCode, VkShaderStageFlagBits shaderStage);


BvGraphicsPipelineStateVk::BvGraphicsPipelineStateVk(BvRenderDeviceVk* pDevice, const GraphicsPipelineStateDesc& pipelineStateDesc, const VkPipelineCache pipelineCache)
	: BvGraphicsPipelineState(pipelineStateDesc), m_pDevice(pDevice), m_PipelineCache(pipelineCache)
{
	auto pVertexInputDescs = BV_NEW_ARRAY(VertexInputDesc, m_PipelineStateDesc.m_VertexInputDescCount);
	memcpy(pVertexInputDescs, m_PipelineStateDesc.m_pVertexInputDescs, sizeof(VertexInputDesc) * m_PipelineStateDesc.m_VertexInputDescCount);
	m_PipelineStateDesc.m_pVertexInputDescs = pVertexInputDescs;

	Create();
}


BvGraphicsPipelineStateVk::~BvGraphicsPipelineStateVk()
{
	Destroy();

	BV_DELETE_ARRAY(m_PipelineStateDesc.m_pVertexInputDescs);
}


BvRenderDevice* BvGraphicsPipelineStateVk::GetDevice()
{
	return m_pDevice;
}


void BvGraphicsPipelineStateVk::Create()
{
	BvFixedVector<VkVertexInputAttributeDescription, kMaxVertexBindings> attributeDescs(m_PipelineStateDesc.m_VertexInputDescCount, {});
	BvFixedVector<VkVertexInputBindingDescription, kMaxVertexBindings> bindingDescs;
	BvFixedVector<VkVertexInputBindingDivisorDescriptionEXT, kMaxVertexBindings> divisorDescs;
	
	{
		BvFixedVector<u32, kMaxVertexBindings> bindingIndices(kMaxVertexBindings, kU32Max);
		BvFixedVector<u32, kMaxVertexBindings> bindingElemLocations(kMaxVertexBindings, 0);
		for (auto i = 0u; i < attributeDescs.Size(); i++)
		{
			auto& viDesc = m_PipelineStateDesc.m_pVertexInputDescs[i];
			u32 stride = GetFormatInfo(viDesc.m_Format).m_BitsPerPixel >> 3;

			// We keep track of each binding that hasn't already been set, and add the elements
			// when a new one is found (which means bindingIndex == kU32Max)
			auto& bindingIndex = bindingIndices[viDesc.m_Binding];
			if (bindingIndex == kU32Max)
			{
				bindingDescs.PushBack({ viDesc.m_Binding, 0,
					GetVkVertexInputRate(viDesc.m_InputRate) });
				bindingIndex = bindingDescs.Size() - 1;
			}

			attributeDescs[i].format = GetVkFormat(viDesc.m_Format);
			attributeDescs[i].location = bindingElemLocations[viDesc.m_Binding]++;
			attributeDescs[i].binding = viDesc.m_Binding;
			attributeDescs[i].offset = viDesc.m_Offset;
			if (attributeDescs[i].offset == VertexInputDesc::kAutoOffset)
			{
				attributeDescs[i].offset = bindingDescs[bindingIndex].stride;
			}

			bindingDescs[bindingIndex].stride += stride;

			if (viDesc.m_InputRate == InputRate::kPerInstance)
			{
				divisorDescs.PushBack({ viDesc.m_Binding, viDesc.m_InstanceRate });
			}
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
	if (m_PipelineStateDesc.m_RasterizerStateDesc.m_EnableConservativeRasterization && m_pDevice->GetDeviceInfo()->m_EnabledExtensions.Find(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME) != kU64Max)
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

	BvFixedVector<VkFormat, kMaxRenderTargets> rtvFormats;
	for (auto i = 0; i < kMaxRenderTargets && m_PipelineStateDesc.m_RenderTargetFormats[i] != Format::kUnknown; i++)
	{
		rtvFormats.EmplaceBack(GetVkFormat(m_PipelineStateDesc.m_RenderTargetFormats[i]));
	}

	VkPipelineRenderingCreateInfoKHR pipelineRenderingCI{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
	VkRenderPass renderPass = m_PipelineStateDesc.m_pRenderPass ? static_cast<BvRenderPassVk*>(m_PipelineStateDesc.m_pRenderPass)->GetHandle() : VK_NULL_HANDLE;
	if (renderPass == VK_NULL_HANDLE)
	{
		pipelineRenderingCI.colorAttachmentCount = rtvFormats.Size();
		pipelineRenderingCI.pColorAttachmentFormats = rtvFormats.Data();
		pipelineRenderingCI.depthAttachmentFormat = GetVkFormat(m_PipelineStateDesc.m_DepthStencilFormat);
		pipelineRenderingCI.stencilAttachmentFormat = pipelineRenderingCI.depthAttachmentFormat;
	}

	BvFixedVector<VkPipelineColorBlendAttachmentState, kMaxRenderTargets> blendAttachments(rtvFormats.Size(), {});
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
	if (m_PipelineStateDesc.m_ShadingRateEnabled && m_pDevice->GetDeviceInfo()->m_EnabledExtensions.Find(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) != kU64Max)
	{
		dynamicStates.PushBack(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
	}

	VkPipelineDynamicStateCreateInfo dynamicStateCI{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateCI.dynamicStateCount = (u32)dynamicStates.Size();
	dynamicStateCI.pDynamicStates = dynamicStates.Data();

	BvFixedVector<VkPipelineShaderStageCreateInfo, kMaxShaderStages> shaderStages;
	for (auto i = 0u; i < kMaxShaderStages && m_PipelineStateDesc.m_Shaders[i] != nullptr; i++)
	{
		auto& shaderStage = shaderStages.EmplaceBack(VkPipelineShaderStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO });
		const auto& byteCode = m_PipelineStateDesc.m_Shaders[i]->GetShaderBlob();
		shaderStage.pName = m_PipelineStateDesc.m_Shaders[i]->GetEntryPoint();
		shaderStage.stage = GetVkShaderStageFlagBits(m_PipelineStateDesc.m_Shaders[i]->GetShaderStage());
		shaderStage.module = CreateShaderModule(m_pDevice->GetHandle(), byteCode.Size(), byteCode.Data(), shaderStages[i].stage);
	}

	VkGraphicsPipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineCI.pNext = !m_PipelineStateDesc.m_pRenderPass ? &pipelineRenderingCI : nullptr;
	//pipelineCI.flags = 0;
	pipelineCI.stageCount = shaderStages.Size();
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
	pipelineCI.layout = TO_VK(m_PipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
	pipelineCI.subpass = m_PipelineStateDesc.m_SubpassIndex;
	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.basePipelineIndex = -1;

	auto result = vkCreateGraphicsPipelines(m_pDevice->GetHandle(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipeline);

	for (auto i = 0u; i < shaderStages.Size(); i++)
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


BvRayTracingPipelineStateVk::BvRayTracingPipelineStateVk(BvRenderDeviceVk* pDevice, const RayTracingPipelineStateDesc& pipelineStateDesc,
	const VkPipelineCache pipelineCache)
	: BvRayTracingPipelineState(pipelineStateDesc), m_pDevice(pDevice), m_PipelineCache(pipelineCache)
{
	auto ppShaders = BV_NEW_ARRAY(BvShader*, m_PipelineStateDesc.m_ShaderCount);
	memcpy(ppShaders, m_PipelineStateDesc.m_ppShaders, sizeof(BvShader*) * m_PipelineStateDesc.m_ShaderCount);
	m_PipelineStateDesc.m_ppShaders = ppShaders;

	auto pGroupDescs = BV_NEW_ARRAY(ShaderGroupDesc, m_PipelineStateDesc.m_ShaderGroupCount);
	memcpy(pGroupDescs, m_PipelineStateDesc.m_pShaderGroupDescs, sizeof(ShaderGroupDesc) * m_PipelineStateDesc.m_ShaderGroupCount);
	m_PipelineStateDesc.m_pShaderGroupDescs = pGroupDescs;

	for (auto g = 0u; g < m_PipelineStateDesc.m_ShaderGroupCount; ++g)
	{
		auto& group = const_cast<ShaderGroupDesc&>(m_PipelineStateDesc.m_pShaderGroupDescs[g]);
		if (group.m_pName)
		{
			auto count = std::char_traits<char>::length(group.m_pName) + 1;
			auto pName = BV_NEW_ARRAY(char, count);
			strcpy(pName, group.m_pName);
			group.m_pName = pName;
		}
	}

	Create();
}


BvRayTracingPipelineStateVk::~BvRayTracingPipelineStateVk()
{
	Destroy();

	for (auto g = 0u; g < m_PipelineStateDesc.m_ShaderGroupCount; ++g)
	{
		auto& group = m_PipelineStateDesc.m_pShaderGroupDescs[g];
		if (group.m_pName)
		{
			BV_DELETE_ARRAY(group.m_pName);
		}
	}

	BV_DELETE_ARRAY(m_PipelineStateDesc.m_pShaderGroupDescs);
	BV_DELETE_ARRAY(m_PipelineStateDesc.m_ppShaders);
}


BvRenderDevice* BvRayTracingPipelineStateVk::GetDevice()
{
	return m_pDevice;
}


void BvRayTracingPipelineStateVk::Create()
{
	BvVector<VkPipelineShaderStageCreateInfo> shaderStages(m_PipelineStateDesc.m_ShaderCount, { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO });
	BvVector<VkRayTracingShaderGroupCreateInfoKHR> groups(m_PipelineStateDesc.m_ShaderGroupCount, { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR });

	for (auto i = 0u; i < m_PipelineStateDesc.m_ShaderCount; i++)
	{
		auto& shaderStage = shaderStages[i];
		const auto& byteCode = m_PipelineStateDesc.m_ppShaders[i]->GetShaderBlob();
		shaderStage.pName = m_PipelineStateDesc.m_ppShaders[i]->GetEntryPoint();
		shaderStage.stage = GetVkShaderStageFlagBits(m_PipelineStateDesc.m_ppShaders[i]->GetShaderStage());
		shaderStage.module = CreateShaderModule(m_pDevice->GetHandle(), byteCode.Size(), byteCode.Data(), shaderStages[i].stage);
	}

	for (auto i = 0u; i < m_PipelineStateDesc.m_ShaderGroupCount; ++i)
	{
		auto& groupDesc = m_PipelineStateDesc.m_pShaderGroupDescs[i];
		auto& group = groups[i];

		switch (groupDesc.m_Type)
		{
		case ShaderGroupType::kNone:
			BV_ASSERT(false, "Shader hit group type can't be kNone");
			Destroy();
			return;
		case ShaderGroupType::kGeneral:
			group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			group.generalShader = groupDesc.m_General;
			group.closestHitShader = VK_SHADER_UNUSED_KHR;
			group.anyHitShader = VK_SHADER_UNUSED_KHR;
			group.intersectionShader = VK_SHADER_UNUSED_KHR;
			break;
		case ShaderGroupType::kTriangles:
			group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			group.generalShader = VK_SHADER_UNUSED_KHR;
			group.closestHitShader = groupDesc.m_ClosestHit;
			group.anyHitShader = groupDesc.m_AnyHit;
			group.intersectionShader = VK_SHADER_UNUSED_KHR;
			break;
		case ShaderGroupType::kProcedural:
			group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
			group.generalShader = VK_SHADER_UNUSED_KHR;
			group.closestHitShader = groupDesc.m_ClosestHit;
			group.anyHitShader = groupDesc.m_AnyHit;
			group.intersectionShader = groupDesc.m_Intersection;
			break;
		}
	}

	VkRayTracingPipelineInterfaceCreateInfoKHR pici{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR, nullptr,
		m_PipelineStateDesc.m_MaxPayloadSize, m_PipelineStateDesc.m_MaxAttributeSize };

	VkRayTracingPipelineCreateInfoKHR ci{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
	ci.groupCount = u32(groups.Size());
	ci.pGroups = groups.Data();
	ci.stageCount = u32(shaderStages.Size());
	ci.pStages = shaderStages.Data();
	ci.maxPipelineRayRecursionDepth = m_PipelineStateDesc.m_MaxPipelineRayRecursionDepth;
	ci.layout = TO_VK(m_PipelineStateDesc.m_pShaderResourceLayout)->GetPipelineLayoutHandle();
	if (m_PipelineStateDesc.m_ForcePayloadAndAttributeSizes)
	{
		ci.pLibraryInterface = &pici;
	}

	vkCreateRayTracingPipelinesKHR(m_pDevice->GetHandle(), VK_NULL_HANDLE, m_PipelineCache, 1, &ci, nullptr, &m_Pipeline);
}


void BvRayTracingPipelineStateVk::Destroy()
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