#pragma once


#include "BvCommonGl.h"
#include "BDeV/Container/BvRobinMap.h"
#include "BDeV/Container/BvFixedVector.h"
#include "BDeV/System/Threading/BvSync.h"


class BvTextureViewGl;


struct FramebufferDesc
{
	BvFixedVector<BvTextureViewGl*, kMaxRenderTargets> m_RenderTargetViews{};
	BvTextureViewGl* m_pDepthStencilView = nullptr;

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

		return true;
	}
};


template<>
struct BvHash<FramebufferDesc>
{
	u64 operator()(const FramebufferDesc& val) const
	{
		u64 hash = 0;
		HashCombine(hash, val.m_RenderTargetViews.Size());
		for (auto pTarget : val.m_RenderTargetViews)
		{
			HashCombine(hash, pTarget);
		}

		if (val.m_pDepthStencilView)
		{
			HashCombine(hash, val.m_pDepthStencilView);
		}

		return hash;
	}
};


class BvFramebufferGl
{
	BV_NOCOPYMOVE(BvFramebufferGl);

public:
	BvFramebufferGl(const FramebufferDesc& framebufferDesc);
	~BvFramebufferGl();

private:
	void Create();
	void Destroy();

	void AttachToFramebuffer(GLenum attachment, BvTextureViewGl* pView);
	BV_INLINE GLuint GetHandle() const { return m_Framebuffer; }

private:
	FramebufferDesc m_FramebufferDesc;
	GLuint m_Framebuffer = 0;
};


class BvFramebufferManagerGl
{
	BV_NOCOPYMOVE(BvFramebufferManagerGl);

public:
	BvFramebufferManagerGl();
	~BvFramebufferManagerGl();

	BvFramebufferGl* GetFramebuffer(const FramebufferDesc& desc);
	void RemoveFramebuffer(const BvTextureViewGl* const pTextureView);
	void Destroy();

private:
	BvRobinMap<FramebufferDesc, BvFramebufferGl*> m_Framebuffers;
	BvSpinlock m_Lock;
};