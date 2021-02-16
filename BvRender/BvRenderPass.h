#pragma once


#include "BvRender/BvCommon.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Container/BvFixedVector.h"



enum class LoadOp : u8
{
	kDontCare = 0x0,
	kLoad = 0x1,
	kClear = 0x2,
};


enum class StoreOp : u8
{
	kDontCare = 0x0,
	kStore = 0x1,
};


struct RenderPassTargetDesc
{
	RenderPassTargetDesc()
		: m_Format(Format::kUndefined), m_StateAfter(ResourceState::kUndefined), m_SampleCount(1),
		m_LoadOp(LoadOp::kClear), m_StoreOp(StoreOp::kStore),
		m_StencilLoadOp(LoadOp::kDontCare), m_StencilStoreOp(StoreOp::kDontCare), m_IsOffscreen(true),
		m_IsReadOnlyDepth(false), m_IsReadOnlyStencil(false) {}

	Format m_Format;
	// Render Passes will perform a transition on the resource to the specified state.
	// If left as kUndefined, the Render Pass will choose the state automatically.
	ResourceState m_StateAfter;
	u8 m_SampleCount;

	LoadOp m_LoadOp : 2;
	StoreOp m_StoreOp : 1;
	LoadOp m_StencilLoadOp: 2;
	StoreOp m_StencilStoreOp : 1;

	// Determines whether it's an offscreen or a presentation render target
	bool m_IsOffscreen : 1;

	// Determines whether the depth part is read-only
	bool m_IsReadOnlyDepth : 1;

	// Determines whether the stencil part is read-only
	bool m_IsReadOnlyStencil : 1;
};


struct RenderPassDesc
{
	BvFixedVector<RenderPassTargetDesc, kMaxRenderTargets> m_RenderTargets;
	RenderPassTargetDesc m_DepthStencilTarget;
	bool m_HasDepth = false;
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