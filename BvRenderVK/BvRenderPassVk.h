#pragma once


#include "BvRenderVK/BvCommonVk.h"
#include "BDeV/RenderAPI/BvRenderPass.h"
#include "BDeV/Utils/BvHash.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/Container/BvRobinMap.h"


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
};


BV_CREATE_CAST_TO_VK(BvRenderPass)


template<>
struct BvHash<RenderPassDesc>
{
	size_t operator()(const RenderPassDesc& renderPassDesc) const
	{
		u64 hash = 0;
		HashCombine(hash, renderPassDesc.m_RenderTargets.Size());
		for (const auto& rt : renderPassDesc.m_RenderTargets)
		{
			HashCombine(hash, rt.m_Format, rt.m_SampleCount, rt.m_StateAfter);
		}
		HashCombine(hash, (u32)renderPassDesc.m_HasDepth);
		if (renderPassDesc.m_HasDepth)
		{
			HashCombine(hash, renderPassDesc.m_DepthStencilTarget.m_Format, renderPassDesc.m_DepthStencilTarget.m_SampleCount, renderPassDesc.m_DepthStencilTarget.m_StateAfter);
		}
	}
};


class BvRenderPassManagerVk
{
public:
	BvRenderPassManagerVk();
	~BvRenderPassManagerVk();

	BvRenderPassVk* GetRenderPass(const BvRenderDeviceVk& device, const RenderPassDesc& desc);
	void Destroy();

private:
	BvRobinMap<RenderPassDesc, BvRenderPassVk*> m_RenderPasses;
	BvSpinlock m_Lock;
};