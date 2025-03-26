#include "BvRenderPassVk.h"
#include "BvRenderDeviceVk.h"
#include "BvUtilsVk.h"
#include "BvTypeConversionsVk.h"


BV_VK_DEVICE_RES_DEF(BvRenderPassVk)


BvRenderPassVk::BvRenderPassVk(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc)
	: m_RenderPassDesc(renderPassDesc), m_pDevice(pDevice)
{
	if (m_RenderPassDesc.m_AttachmentCount > 0)
	{
		m_pAttachments = BV_NEW_ARRAY(RenderPassAttachment, m_RenderPassDesc.m_AttachmentCount);
		memcpy(m_pAttachments, m_RenderPassDesc.m_pAttachments, sizeof(RenderPassAttachment) * m_RenderPassDesc.m_AttachmentCount);
		m_RenderPassDesc.m_pAttachments = m_pAttachments;
	}

	if (m_RenderPassDesc.m_SubpassDependencyCount > 0)
	{
		m_pDependencies = BV_NEW_ARRAY(SubpassDependency, m_RenderPassDesc.m_SubpassDependencyCount);
		memcpy(m_pDependencies, m_RenderPassDesc.m_pSubpassDependencies, sizeof(SubpassDependency) * m_RenderPassDesc.m_SubpassDependencyCount);
		m_RenderPassDesc.m_pSubpassDependencies = m_pDependencies;
	}

	if (m_RenderPassDesc.m_SubpassCount > 0)
	{
		m_pSubpasses = BV_NEW_ARRAY(SubpassDesc, m_RenderPassDesc.m_SubpassCount);
		memcpy(m_pSubpasses, m_RenderPassDesc.m_pSubpasses, sizeof(SubpassDesc) * m_RenderPassDesc.m_SubpassCount);
		m_RenderPassDesc.m_pSubpasses = m_pSubpasses;
	}

	u32 attachmentRefCount = 0;
	u32 shadingRateRefCount = 0;
	for (auto i = 0u; i < m_RenderPassDesc.m_SubpassCount; ++i)
	{
		auto& subpass = m_RenderPassDesc.m_pSubpasses[i];
		attachmentRefCount += subpass.m_ColorAttachmentCount + subpass.m_InputAttachmentCount + (subpass.m_pDepthStencilAttachment ? 1 : 0)
			+ (subpass.m_pResolveAttachments ? subpass.m_ColorAttachmentCount : 0);
		shadingRateRefCount += subpass.m_pShadingRateAttachment ? 1 : 0;
	}

	u32 currRefIndex = 0;
	u32 currShadingRefIndex = 0;
	if (attachmentRefCount > 0)
	{
		m_pRefs = BV_NEW_ARRAY(AttachmentRef, attachmentRefCount);
	}
	if (shadingRateRefCount > 0)
	{
		m_pSRRefs = BV_NEW_ARRAY(ShadingRateAttachmentRef, shadingRateRefCount);
	}

	for (auto i = 0u; i < m_RenderPassDesc.m_SubpassCount; ++i)
	{
		auto& subpass = m_pSubpasses[i];
		if (subpass.m_ColorAttachmentCount > 0)
		{
			memcpy(m_pRefs + currRefIndex, subpass.m_pColorAttachments, sizeof(AttachmentRef) * subpass.m_ColorAttachmentCount);
			subpass.m_pColorAttachments = m_pRefs + currRefIndex;
			currRefIndex += subpass.m_ColorAttachmentCount;
		}
		if (subpass.m_InputAttachmentCount > 0)
		{
			memcpy(m_pRefs + currRefIndex, subpass.m_pInputAttachments, sizeof(AttachmentRef) * subpass.m_InputAttachmentCount);
			subpass.m_pInputAttachments = m_pRefs + currRefIndex;
			currRefIndex += subpass.m_InputAttachmentCount;
		}
		if (subpass.m_pResolveAttachments)
		{
			memcpy(m_pRefs + currRefIndex, subpass.m_pResolveAttachments, sizeof(AttachmentRef) * subpass.m_ColorAttachmentCount);
			subpass.m_pResolveAttachments = m_pRefs + currRefIndex;
			currRefIndex += subpass.m_ColorAttachmentCount;
		}
		if (subpass.m_pDepthStencilAttachment)
		{
			m_pRefs[currRefIndex] = *subpass.m_pDepthStencilAttachment;
			subpass.m_pDepthStencilAttachment = m_pRefs + currRefIndex;
			currRefIndex += 1;
		}
		if (subpass.m_pShadingRateAttachment)
		{
			m_pSRRefs[currShadingRefIndex] = *subpass.m_pShadingRateAttachment;
			subpass.m_pShadingRateAttachment = m_pSRRefs + currShadingRefIndex;
			currShadingRefIndex += 1;
		}
	}

	Create();
}


BvRenderPassVk::~BvRenderPassVk()
{
	if (m_pAttachments)
	{
		BV_DELETE_ARRAY(m_pAttachments);
	}
	if (m_pSubpasses)
	{
		BV_DELETE_ARRAY(m_pSubpasses);
	}
	if (m_pRefs)
	{
		BV_DELETE_ARRAY(m_pRefs);
	}
	if (m_pSRRefs)
	{
		BV_DELETE_ARRAY(m_pSRRefs);
	}
	if (m_pDependencies)
	{
		BV_DELETE_ARRAY(m_pDependencies);
	}

	Destroy();
}


void BvRenderPassVk::Create()
{
	BvVector<VkAttachmentDescription2> attachments;
	SetupAttachments(attachments);

	BvVector<VkSubpassDescription2> subpasses;
	BvVector<VkAttachmentReference2> attachmentReferences;
	BvVector<VkFragmentShadingRateAttachmentInfoKHR> shadingRateRefs{ VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR };
	SetupSubpasses(subpasses, attachmentReferences, shadingRateRefs);

	BvVector<VkSubpassDependency2> dependencies;
	BvVector<VkMemoryBarrier2> barriers;
	SetupDependencies(dependencies, barriers);

	// Create render pass
	VkRenderPassCreateInfo2 createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };
	createInfo.attachmentCount = (u32)attachments.Size();
	createInfo.pAttachments = attachments.Data();
	createInfo.subpassCount = (u32)subpasses.Size();
	createInfo.pSubpasses = subpasses.Data();
	createInfo.dependencyCount = (u32)dependencies.Size();
	createInfo.pDependencies = dependencies.Data();

	auto result = vkCreateRenderPass2(m_pDevice->GetHandle(), &createInfo, nullptr, &m_RenderPass);
}


void BvRenderPassVk::Destroy()
{
	if (m_RenderPass)
	{
		vkDestroyRenderPass(m_pDevice->GetHandle(), m_RenderPass, nullptr);
		m_RenderPass = VK_NULL_HANDLE;
	}
}


void BvRenderPassVk::SetupAttachments(BvVector<VkAttachmentDescription2>& attachments)
{
	attachments.Resize(m_RenderPassDesc.m_AttachmentCount, VkAttachmentDescription2{ VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 });
	for (auto i = 0u; i < m_RenderPassDesc.m_AttachmentCount; ++i)
	{
		auto& attachment = m_RenderPassDesc.m_pAttachments[i];

		auto& attachmentVk = attachments[i];
		//attachmentVk.pNext = nullptr;
		//attachmentVk.flags;
		attachmentVk.format = GetVkFormat(attachment.m_Format);
		attachmentVk.samples = GetVkSampleCountFlagBits(attachment.m_SampleCount);
		attachmentVk.loadOp = GetVkAttachmentLoadOp(attachment.m_LoadOp);
		attachmentVk.storeOp = GetVkAttachmentStoreOp(attachment.m_StoreOp);
		if (IsDepthStencilFormat(attachment.m_Format))
		{
			attachmentVk.stencilLoadOp = attachmentVk.loadOp;
			attachmentVk.stencilStoreOp = attachmentVk.storeOp;
		}
		else
		{
			attachmentVk.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentVk.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		attachmentVk.initialLayout = GetVkImageLayout(attachment.m_StateBefore, IsDepthOrStencilFormat(attachment.m_Format));
		attachmentVk.finalLayout = GetVkImageLayout(attachment.m_StateAfter, IsDepthOrStencilFormat(attachment.m_Format));
	}
}


void BvRenderPassVk::SetupSubpasses(BvVector<VkSubpassDescription2>& subpasses,
	BvVector<VkAttachmentReference2>& attachmentRefs, BvVector<VkFragmentShadingRateAttachmentInfoKHR>& shadingRateRefs)
{
	subpasses.Resize(m_RenderPassDesc.m_SubpassCount, VkSubpassDescription2{ VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2 });
	attachmentRefs.Reserve(m_RenderPassDesc.m_SubpassCount * m_RenderPassDesc.m_AttachmentCount);
	u32 srRefCount = 0;
	for (auto i = 0; i < m_RenderPassDesc.m_SubpassCount; ++i)
	{
		auto& subpass = m_RenderPassDesc.m_pSubpasses[i];
		if (subpass.m_pShadingRateAttachment)
		{
			++srRefCount;
		}
	}
	shadingRateRefs.Reserve(srRefCount);

	for (auto i = 0u; i < m_RenderPassDesc.m_SubpassCount; ++i)
	{
		auto& subpass = m_RenderPassDesc.m_pSubpasses[i];
		
		auto& subpassVk = subpasses[i];
		subpassVk.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		if (subpass.m_ColorAttachmentCount > 0)
		{
			subpassVk.colorAttachmentCount = subpass.m_ColorAttachmentCount;
			auto currIndex = (u32)attachmentRefs.Size();
			for (auto j = 0u; j < subpass.m_ColorAttachmentCount; ++j)
			{
				auto& ref = subpass.m_pColorAttachments[j];

				auto& refVk = attachmentRefs.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
				refVk.aspectMask = GetVkFormatMap(m_RenderPassDesc.m_pAttachments[ref.m_Index].m_Format).aspectFlags;
				refVk.attachment = ref.m_Index;
				refVk.layout = GetVkImageLayout(ref.m_ResourceState);
			}

			subpassVk.pColorAttachments = &attachmentRefs[currIndex];
		}

		if (subpass.m_pDepthStencilAttachment)
		{
			auto& ref = *subpass.m_pDepthStencilAttachment;

			auto& refVk = attachmentRefs.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
			refVk.aspectMask = GetVkFormatMap(m_RenderPassDesc.m_pAttachments[ref.m_Index].m_Format).aspectFlags;
			refVk.attachment = ref.m_Index;
			refVk.layout = GetVkImageLayout(ref.m_ResourceState);

			subpassVk.pDepthStencilAttachment = &refVk;
		}

		if (subpass.m_InputAttachmentCount > 0)
		{
			auto currIndex = (u32)attachmentRefs.Size();
			for (auto j = 0u; j < subpass.m_InputAttachmentCount; ++j)
			{
				auto& ref = subpass.m_pInputAttachments[j];

				auto& refVk = attachmentRefs.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
				refVk.aspectMask = GetVkFormatMap(m_RenderPassDesc.m_pAttachments[ref.m_Index].m_Format).aspectFlags;
				refVk.attachment = ref.m_Index;
				refVk.layout = GetVkImageLayout(ref.m_ResourceState);
			}
			subpassVk.pInputAttachments = &attachmentRefs[currIndex];
		}

		if (subpass.m_pResolveAttachments)
		{
			auto currIndex = (u32)attachmentRefs.Size();
			for (auto j = 0u; j < subpass.m_ColorAttachmentCount; ++j)
			{
				auto& ref = subpass.m_pResolveAttachments[j];

				auto& refVk = attachmentRefs.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
				refVk.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				refVk.attachment = ref.m_Index;
				refVk.layout = GetVkImageLayout(ref.m_ResourceState);
			}

			subpassVk.pResolveAttachments = &attachmentRefs[currIndex];
		}

		if (subpass.m_pShadingRateAttachment)
		{
			auto& ref = *subpass.m_pShadingRateAttachment;

			auto& refVk = attachmentRefs.EmplaceBack(VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
			refVk.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			refVk.attachment = ref.m_Index;
			refVk.layout = GetVkImageLayout(ref.m_ResourceState);

			auto& shadingRateRef = shadingRateRefs.EmplaceBack();
			shadingRateRef.shadingRateAttachmentTexelSize = VkExtent2D{ subpass.m_pShadingRateAttachment->m_TexelSizes[0],
				subpass.m_pShadingRateAttachment->m_TexelSizes[1] };
			shadingRateRef.pFragmentShadingRateAttachment = &refVk;

			subpassVk.pNext = &shadingRateRef;
		}
	}
}


void BvRenderPassVk::SetupDependencies(BvVector<VkSubpassDependency2>& dependencies, BvVector<VkMemoryBarrier2>& barriers)
{
	if (m_RenderPassDesc.m_SubpassDependencyCount > 0)
	{
		dependencies.Resize(m_RenderPassDesc.m_SubpassDependencyCount, VkSubpassDependency2{ VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 });
		barriers.Resize(dependencies.Size(), VkMemoryBarrier2{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 });
		for (auto i = 0u; i < m_RenderPassDesc.m_SubpassDependencyCount; ++i)
		{
			auto& dep = m_RenderPassDesc.m_pSubpassDependencies[i];

			auto& depVk = dependencies[i];
			auto& barrierVk = barriers[i];
			depVk.srcSubpass = dep.m_SrcSubpass;
			depVk.dstSubpass = dep.m_DstSubpass;
			barrierVk.srcAccessMask = GetVkAccessFlags(dep.m_SrcAccess);
			barrierVk.dstAccessMask = GetVkAccessFlags(dep.m_DstAccess);
			barrierVk.srcStageMask = GetVkPipelineStageFlags(dep.m_SrcStage);
			barrierVk.dstStageMask = GetVkPipelineStageFlags(dep.m_DstStage);
			depVk.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
	}
	else
	{
		dependencies.Resize(m_RenderPassDesc.m_SubpassCount + 1, VkSubpassDependency2{ VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 });
		barriers.Resize(dependencies.Size(), VkMemoryBarrier2{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 });

		auto lastPassIndex = m_RenderPassDesc.m_SubpassCount - 1;
		for (auto i = 0u; i < lastPassIndex; ++i)
		{
			auto& srcPass = m_RenderPassDesc.m_pSubpasses[i];
			auto& dstPass = m_RenderPassDesc.m_pSubpasses[i + 1];

			auto& dep = dependencies[i + 1];
			auto& barrier = barriers[i + 1];
			dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dep.srcSubpass = i;
			dep.dstSubpass = i + 1;
			dep.pNext = &barrier;
			GetVkFlags(i, barrier.srcAccessMask, barrier.srcStageMask);
			GetVkFlags(i + 1, barrier.dstAccessMask, barrier.dstStageMask);
		}

		auto lastDependencyIndex = (u32)dependencies.Size() - 1;

		auto& extSrc = dependencies[0];
		extSrc.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		extSrc.srcSubpass = VK_SUBPASS_EXTERNAL;
		extSrc.dstSubpass = 0;
		extSrc.pNext = &barriers[0];
		GetVkFlags(0, barriers[0].dstAccessMask, barriers[0].dstStageMask);

		auto& extDst = dependencies[lastDependencyIndex];
		extDst.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		extDst.srcSubpass = lastPassIndex;
		extDst.dstSubpass = VK_SUBPASS_EXTERNAL;
		extDst.pNext = &barriers[lastDependencyIndex];
		GetVkFlags(lastPassIndex,
			barriers[lastDependencyIndex].srcAccessMask, barriers[lastDependencyIndex].srcStageMask);

		auto& lastSubpass = m_RenderPassDesc.m_pSubpasses[lastPassIndex];
		VkAccessFlags2 accessFlags = 0;
		VkPipelineStageFlags2 stageFlags = 0;
		for (auto i = 0u; i < lastSubpass.m_ColorAttachmentCount; ++i)
		{
			auto& attachment = lastSubpass.m_pColorAttachments[i];
			if (m_RenderPassDesc.m_pAttachments[attachment.m_Index].m_StateAfter == ResourceState::kPresent)
			{
				accessFlags = VK_ACCESS_2_MEMORY_READ_BIT;
				stageFlags = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
				
				break;
			}
		}

		if (accessFlags != 0 && stageFlags != 0)
		{
			barriers[0].srcAccessMask = accessFlags;
			barriers[0].srcStageMask = stageFlags;

			barriers[lastDependencyIndex].dstAccessMask = accessFlags;
			barriers[lastDependencyIndex].dstStageMask = stageFlags;
		}
		else
		{
			barriers[0].srcAccessMask = barriers[0].dstAccessMask;
			barriers[0].srcStageMask = barriers[0].dstStageMask;

			barriers[lastDependencyIndex].dstAccessMask = barriers[lastDependencyIndex].srcAccessMask;
			barriers[lastDependencyIndex].dstStageMask = barriers[lastDependencyIndex].srcStageMask;
		}
	}
}


void BvRenderPassVk::GetVkFlags(u32 subpassIndex, VkAccessFlags2& accessFlags, VkPipelineStageFlags2& stageFlags)
{
	auto& subpass = m_RenderPassDesc.m_pSubpasses[subpassIndex];
	if (subpass.m_ColorAttachmentCount > 0)
	{
		accessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (subpass.m_InputAttachmentCount > 0)
	{
		accessFlags |= VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
	}
	if (subpass.m_pDepthStencilAttachment)
	{
		accessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (subpass.m_pShadingRateAttachment)
	{
		accessFlags |= VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
	}
	stageFlags |= GetVkPipelineStageFlags(accessFlags);
	if (accessFlags & VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT)
	{
		stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	}
}