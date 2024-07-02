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
	BvFixedVector<VkImageView, kMaxRenderTargetsWithDepth> m_Views;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	u32 m_Width = 0;
	u32 m_Height = 0;
	u32 m_LayerCount = 0;

	bool operator==(const FramebufferDesc& rhs) const
	{
		if (m_Views.Size() != rhs.m_Views.Size())
		{
			return false;
		}

		for (auto i = 0u; i < m_Views.Size(); i++)
		{
			if (m_Views[i] != rhs.m_Views[i])
			{
				return false;
			}
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
	u64 operator()(const FramebufferDesc& val) const
	{
		u64 hash = 0;
		HashCombine(hash, val.m_Views.Size());
		for (auto view : val.m_Views)
		{
			HashCombine(hash, view);
		}

		HashCombine(hash, val.m_RenderPass);
		HashCombine(hash, val.m_Width);
		HashCombine(hash, val.m_Height);
		HashCombine(hash, val.m_LayerCount);

		return hash;
	}
};


class BvFramebufferManagerVk
{
public:
	BvFramebufferManagerVk();
	~BvFramebufferManagerVk();

	VkFramebuffer GetFramebuffer(VkDevice device, const FramebufferDesc& framebufferDesc);
	void RemoveFramebuffersWithView(VkImageView view);

private:
	BvRobinMap<FramebufferDesc, VkFramebuffer> m_Framebuffers;
};