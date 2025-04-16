#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderPass.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvRenderDeviceVk;


//BV_OBJECT_DEFINE_ID(IBvRenderPassVk, "1be279e7-57e1-49a2-98f0-61bb54edac9b");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRenderPassVk);


class BvRenderPassVk final : public IBvRenderPass, public IBvResourceVk
{
	BV_VK_DEVICE_RES_DECL;

public:
	BvRenderPassVk(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc);
	~BvRenderPassVk();

	BV_INLINE const RenderPassDesc& GetDesc() const override { return m_RenderPassDesc; }
	BV_INLINE const VkRenderPass GetHandle() const { return m_RenderPass; }
	BV_INLINE bool IsValid() const { return m_RenderPass != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvRenderPassVk, IBvRenderPass, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

	void SetupAttachments(BvVector<VkAttachmentDescription2>& attachments);
	void SetupSubpasses(BvVector<VkSubpassDescription2>& subpasses, BvVector<VkAttachmentReference2>& attachmentRefs,
		BvVector<VkSubpassDescriptionDepthStencilResolve>& depthStencilResolves, BvVector<VkFragmentShadingRateAttachmentInfoKHR>& shadingRateRefs);
	void SetupDependencies(BvVector<VkSubpassDependency2>& dependencies, BvVector<VkMemoryBarrier2>& barriers);

	void GetVkFlags(u32 subpassIndex, VkAccessFlags2& accessFlags, VkPipelineStageFlags2& stageFlags);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	RenderPassDesc m_RenderPassDesc;
	RenderPassAttachment* m_pAttachments = nullptr;
	SubpassDependency* m_pDependencies = nullptr;
	SubpassDesc* m_pSubpasses = nullptr;
	AttachmentRef* m_pRefs = nullptr;
	ShadingRateAttachmentRef* m_pSRRefs = nullptr;
};


BV_CREATE_CAST_TO_VK(BvRenderPass)