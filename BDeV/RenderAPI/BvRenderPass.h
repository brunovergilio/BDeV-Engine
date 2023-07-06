#pragma once


#include "BvCommon.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Container/BvFixedVector.h"


struct RenderPassTargetDesc
{
	RenderPassTargetDesc()
		: m_Format(Format::kUnknown), m_StateAfter(ResourceState::kUnknown), m_SampleCount(1),
		m_LoadOp(LoadOp::kClear), m_StoreOp(StoreOp::kStore),
		m_StencilLoadOp(LoadOp::kDontCare), m_StencilStoreOp(StoreOp::kDontCare),
		m_IsReadOnlyDepth(false), m_IsReadOnlyStencil(false) {}

	Format m_Format;
	// Render Passes will perform a transition on the resource to the specified state.
	// If left as kUnknown, the Render Pass will choose the state automatically.
	ResourceState m_StateAfter;
	u8 m_SampleCount : 5;

	LoadOp m_LoadOp : 2;
	StoreOp m_StoreOp : 1;
	LoadOp m_StencilLoadOp: 2;
	StoreOp m_StencilStoreOp : 1;

	// Determines whether the depth part is read-only
	bool m_IsReadOnlyDepth : 1;

	// Determines whether the stencil part is read-only
	bool m_IsReadOnlyStencil : 1;

	bool operator==(const RenderPassTargetDesc& rhs) const noexcept
	{
		return m_Format == rhs.m_Format &&
			m_StateAfter == rhs.m_StateAfter &&
			m_SampleCount == rhs.m_SampleCount &&
			m_LoadOp == rhs.m_LoadOp &&
			m_StoreOp == rhs.m_StoreOp &&
			m_StencilLoadOp == rhs.m_StencilLoadOp &&
			m_StencilStoreOp == rhs.m_StencilStoreOp &&
			m_IsReadOnlyDepth == rhs.m_IsReadOnlyDepth &&
			m_IsReadOnlyStencil == rhs.m_IsReadOnlyStencil;
	}

	bool operator!=(const RenderPassTargetDesc& rhs) const noexcept
	{
		return !(*this == rhs);
	}
};


struct RenderPassDesc
{
	BvFixedVector<RenderPassTargetDesc, kMaxRenderTargets> m_RenderTargets;
	RenderPassTargetDesc m_DepthStencilTarget;
	bool m_HasDepth = false;

	bool operator==(const RenderPassDesc& rhs) const noexcept
	{
		if (m_RenderTargets.Size() != rhs.m_RenderTargets.Size())
		{
			return false;
		}

		for (auto i = 0u; i < m_RenderTargets.Size(); i++)
		{
			if (m_RenderTargets[i] != rhs.m_RenderTargets[i])
			{
				return false;
			}
		}

		if (m_HasDepth != rhs.m_HasDepth)
		{
			return false;
		}

		return m_DepthStencilTarget == rhs.m_DepthStencilTarget;
	}

	bool operator!=(const RenderPassDesc& rhs) const noexcept
	{
		return !(*this == rhs);
	}
};


class BvRenderPass
{
	BV_NOCOPYMOVE(BvRenderPass);

public:
	BV_INLINE const RenderPassDesc & GetRenderTargetDesc() const { return m_RenderPassDesc; }

protected:
	BvRenderPass(const RenderPassDesc & renderPassDesc)
		: m_RenderPassDesc(renderPassDesc) {}
	virtual ~BvRenderPass() = 0 {}

protected:
	RenderPassDesc m_RenderPassDesc;
};