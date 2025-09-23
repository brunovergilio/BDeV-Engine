#include "BvRenderPassD3D12.h"
#include "BvRenderDeviceD3D12.h"


BV_D3D12_DEVICE_RES_DEF(BvRenderPassD3D12)


BvRenderPassD3D12::BvRenderPassD3D12(BvRenderDeviceD3D12* pDevice, const RenderPassDesc& renderPassDesc)
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
}


BvRenderPassD3D12::~BvRenderPassD3D12()
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
}