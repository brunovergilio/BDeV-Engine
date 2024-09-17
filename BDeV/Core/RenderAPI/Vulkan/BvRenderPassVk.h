#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderPass.h"
#include "BDeV/Core/Utils/BvHash.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvRenderDeviceVk;


class BvRenderPassVk final : public BvRenderPass
{
public:
	BvRenderPassVk(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc);
	~BvRenderPassVk();

	BvRenderDevice* GetDevice() override;
	BV_INLINE const VkRenderPass GetHandle() const { return m_RenderPass; }

private:
	void Create();
	void Destroy();

	void SetupAttachments(BvVector<VkAttachmentDescription2>& attachments);
	void SetupSubpasses(BvVector<VkSubpassDescription2>& subpasses, BvVector<VkAttachmentReference2>& attachmentRefs,
		VkFragmentShadingRateAttachmentInfoKHR& shadingRateRef);
	void SetupDependencies(BvVector<VkSubpassDependency2>& dependencies, BvVector<VkMemoryBarrier2>& barriers);

	void GetVkFlags(u32 subpassIndex, VkAccessFlags2& accessFlags, VkPipelineStageFlags2& stageFlags);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
};


BV_CREATE_CAST_TO_VK(BvRenderPass)