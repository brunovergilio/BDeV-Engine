#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BvRender/BvRenderPass.h"


class BvRenderDeviceVk;


class BvRenderPassVk final : public BvRenderPass
{
public:
	BvRenderPassVk(const BvRenderDeviceVk & device, const RenderPassDesc & renderPassDesc);
	~BvRenderPassVk();

	void Create();
	void Destroy();

	BV_INLINE const VkRenderPass GetHandle() const { return m_RenderPass; }

private:
	void SetupAttachments(BvFixedVector<VkAttachmentDescription, kMaxRenderTargetsWithDepth> & attachmentDescs,
		BvFixedVector<VkAttachmentReference, kMaxRenderTargetsWithDepth> & attachmentReferences);
	void SetupDependencies(BvFixedVector<VkSubpassDependency, 2> & dependencies);

private:
	const BvRenderDeviceVk & m_Device;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	RenderPassDesc m_RenderPassDesc;
};