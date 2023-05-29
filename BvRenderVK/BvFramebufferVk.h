#pragma once


#include "BDeV/RenderAPI/BvCommon.h"
#include "BvRenderVk/BvCommonVk.h"
#include "BDeV/Container/BvFixedVector.h"
#include "BvTextureViewVk.h"
#include "BDeV/Utils/BvHash.h"
#include "BDeV/Container/BvRobinMap.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BvRenderPassVk.h"


class BvRenderDeviceVk;


struct FramebufferDesc
{
	BvFixedVector<BvTextureViewVk*, kMaxRenderTargets> m_RenderTargetViews{};
	BvTextureViewVk* m_pDepthStencilView = VK_NULL_HANDLE;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;

	bool operator==(const FramebufferDesc& rhs) const
	{
		if (m_RenderTargetViews.Size() != rhs.m_RenderTargetViews.Size())
		{
			return false;
		}

		for (auto i = 0u; i < m_RenderTargetViews.Size(); i++)
		{
			if (m_RenderTargetViews[i] != rhs.m_RenderTargetViews[i])
			{
				return false;
			}
		}

		if (m_pDepthStencilView != rhs.m_pDepthStencilView)
		{
			return false;
		}

		if (m_RenderPass != rhs.m_RenderPass)
		{
			return false;
		}

		return true;
	}
};


template<>
struct BvHash<FramebufferDesc>
{
	u64 operator()(const FramebufferDesc & val)
	{
		u64 hash = 0;
		HashCombine(hash, val.m_RenderTargetViews.Size());
		for (auto pTarget : val.m_RenderTargetViews)
		{
			HashCombine(hash, pTarget);
		}

		HashCombine(hash, val.m_pDepthStencilView != nullptr ? 1 : 0);
		if (val.m_pDepthStencilView)
		{
			HashCombine(hash, val.m_pDepthStencilView);
		}

		return hash;
	}
};


class BvFramebufferVk
{
public:
	BvFramebufferVk(const BvRenderDeviceVk & device, const FramebufferDesc& framebufferDesc);
	~BvFramebufferVk();

	void Create();
	void Destroy();

	BV_INLINE const FramebufferDesc & GetDesc() const { return m_FramebufferDesc; }
	BV_INLINE VkFramebuffer GetHandle() const { return m_Framebuffer; }

private:
	FramebufferDesc m_FramebufferDesc;
	const BvRenderDeviceVk & m_Device;
	VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
};


class BvFramebufferManagerVk
{
public:
	BvFramebufferManagerVk();
	~BvFramebufferManagerVk();

	BvFramebufferVk * GetFramebuffer(const BvRenderDeviceVk & device, const FramebufferDesc& desc);
	void RemoveFramebuffer(const BvTextureViewVk * const pTextureView);
	void Destroy();

private:
	BvRobinMap<FramebufferDesc, BvFramebufferVk *> m_Framebuffers;
	BvSpinlock m_Lock;
};