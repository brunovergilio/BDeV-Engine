#pragma once


#include "BvRender/BvCommon.h"
#include "BvRenderVk/BvCommonVk.h"
#include "BvCore/Container/BvFixedVector.h"
#include "BvTextureViewVk.h"
#include "BvCore/Utils/Hash.h"
#include "BvCore/Container/BvRobinMap.h"
#include "BvCore/System/Threading/BvSync.h"


class BvRenderDeviceVk;


struct FramebufferDesc
{
	BvFixedVector<BvTextureViewVk *, kMaxRenderTargets> m_RenderTargetViews{};
	BvTextureViewVk * m_pDepthStencilView = nullptr;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
};


template<>
struct std::hash<FramebufferDesc>
{
	u64 operator()(const FramebufferDesc & val)
	{
		u64 hash = 0;
		for (decltype(auto) pTarget : val.m_RenderTargetViews)
		{
			decltype(auto) desc = pTarget->GetDesc().m_pTexture->GetDesc();
			HashCombine(hash,
				((u32)desc.m_UsageFlags),
				(desc.m_UseAsCubeMap ? 1 : 0),
				((u32)desc.m_Size.width),
				((u32)desc.m_Size.height),
				((u32)desc.m_Size.depthOrLayerCount));
		}

		if (auto pTarget = val.m_pDepthStencilView)
		{
			decltype(auto) desc = pTarget->GetDesc().m_pTexture->GetDesc();
			HashCombine(hash,
				((u32)desc.m_UsageFlags),
				(desc.m_UseAsCubeMap ? 1u : 0u),
				((u32)desc.m_Size.width),
				((u32)desc.m_Size.height),
				((u32)desc.m_Size.depthOrLayerCount));
		}

		return hash;
	}
};


class BvFramebufferVk
{
public:
	BvFramebufferVk(const BvRenderDeviceVk & device, const FramebufferDesc & framebufferDesc);
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


class BvFramebufferManager
{
public:
	BvFramebufferManager();
	~BvFramebufferManager();

	BvFramebufferVk * GetFramebuffer(const BvRenderDeviceVk & device, const FramebufferDesc & desc);
	void RemoveFramebuffer(const BvTextureViewVk * const pTextureView);

private:
	BvRobinMap<u64, BvFramebufferVk *> m_Framebuffers;
	BvSpinlock m_Lock;
};