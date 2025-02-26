#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Utils/BvHash.h"
#include "BvRenderDeviceObject.h"


struct AttachmentRef
{
	u32 m_Index = 0;
	ResourceState m_ResourceState = ResourceState::kCommon;

	constexpr bool operator==(const AttachmentRef& rhs) const
	{
		return m_Index == rhs.m_Index && m_ResourceState == rhs.m_ResourceState;
	}

	constexpr bool operator!=(const AttachmentRef& rhs) const
	{
		return !(*this == rhs);
	}
};


struct ShadingRateAttachmentRef
{
	u32 m_Index = 0;
	ResourceState m_ResourceState = ResourceState::kCommon;
	u32 m_TexelSizes[2]{ 0,0 };

	constexpr bool operator==(const ShadingRateAttachmentRef& rhs) const
	{
		return m_Index == rhs.m_Index && m_ResourceState == rhs.m_ResourceState
			&& m_TexelSizes[0] == rhs.m_TexelSizes[0] && m_TexelSizes[1] == rhs.m_TexelSizes[1];
	}

	constexpr bool operator!=(const ShadingRateAttachmentRef& rhs) const
	{
		return !(*this == rhs);
	}
};


struct SubpassDesc
{
	u32 m_InputAttachmentCount = 0;
	u32 m_ColorAttachmentCount = 0;
	const AttachmentRef* m_pColorAttachments = nullptr;
	const AttachmentRef* m_pDepthStencilAttachment = nullptr;
	const AttachmentRef* m_pInputAttachments = nullptr;
	const AttachmentRef* m_pResolveAttachments = nullptr;
	const ShadingRateAttachmentRef* m_pShadingRateAttachment = nullptr;

	constexpr bool operator==(const SubpassDesc& rhs) const
	{
		if (m_ColorAttachmentCount != rhs.m_ColorAttachmentCount || m_InputAttachmentCount != rhs.m_InputAttachmentCount)
		{
			return false;
		}

		if ((m_pDepthStencilAttachment == nullptr && rhs.m_pDepthStencilAttachment != nullptr)
			|| (m_pDepthStencilAttachment != nullptr && rhs.m_pDepthStencilAttachment == nullptr))
		{
			return false;
		}

		if (m_pDepthStencilAttachment != nullptr && rhs.m_pDepthStencilAttachment != nullptr
			&& *m_pDepthStencilAttachment != *rhs.m_pDepthStencilAttachment)
		{
			return false;
		}

		if ((m_pResolveAttachments == nullptr && rhs.m_pResolveAttachments != nullptr)
			|| (m_pResolveAttachments != nullptr && rhs.m_pResolveAttachments == nullptr))
		{
			return false;
		}

		if (m_pResolveAttachments != nullptr && rhs.m_pResolveAttachments != nullptr
			&& *m_pResolveAttachments != *rhs.m_pResolveAttachments)
		{
			return false;
		}

		if ((m_pShadingRateAttachment == nullptr && rhs.m_pShadingRateAttachment != nullptr)
			|| (m_pShadingRateAttachment != nullptr && rhs.m_pShadingRateAttachment == nullptr))
		{
			return false;
		}

		if (m_pShadingRateAttachment != nullptr && rhs.m_pShadingRateAttachment != nullptr
			&& *m_pShadingRateAttachment != *rhs.m_pShadingRateAttachment)
		{
			return false;
		}

		for (auto i = 0u; i < m_ColorAttachmentCount; ++i)
		{
			if (m_pColorAttachments[i] != rhs.m_pColorAttachments[i])
			{
				return false;
			}
		}

		for (auto i = 0u; i < m_InputAttachmentCount; ++i)
		{
			if (m_pInputAttachments[i] != rhs.m_pInputAttachments[i])
			{
				return false;
			}
		}

		return true;
	}

	constexpr bool operator!=(const SubpassDesc& rhs) const
	{
		return !(*this == rhs);
	}
};


struct SubpassDependency
{
	static constexpr auto kExternalSubpassIndex = kU32Max;

	u32 m_SrcSubpass = 0;
	u32 m_DstSubpass = 0;
	ResourceAccess m_SrcAccess = ResourceAccess::kAuto;
	ResourceAccess m_DstAccess = ResourceAccess::kAuto;
	PipelineStage m_SrcStage = PipelineStage::kAuto;
	PipelineStage m_DstStage = PipelineStage::kAuto;

	constexpr bool operator==(const SubpassDependency& rhs) const
	{
		return m_SrcSubpass == rhs.m_SrcSubpass && (u32)m_SrcAccess == (u32)rhs.m_SrcAccess && (u32)m_SrcStage == (u32)rhs.m_SrcStage
			&& m_DstSubpass == rhs.m_DstSubpass && (u32)m_DstAccess == (u32)rhs.m_DstAccess && (u32)m_DstStage == (u32)rhs.m_DstStage;
	}

	constexpr bool operator!=(const SubpassDependency& rhs) const
	{
		return !(*this == rhs);
	}
};



struct RenderPassAttachment
{
	RenderPassAttachment()
		: m_Format(Format::kUnknown), m_StateBefore(ResourceState::kCommon), m_StateAfter(ResourceState::kCommon), m_SampleCount(1),
		m_LoadOp(LoadOp::kClear), m_StoreOp(StoreOp::kStore)
	{}

	Format m_Format;
	ResourceState m_StateBefore;
	ResourceState m_StateAfter;
	u8 m_SampleCount : 5;
	LoadOp m_LoadOp : 2;
	StoreOp m_StoreOp : 1;

	constexpr bool operator==(const RenderPassAttachment& rhs) const
	{
		return m_Format == rhs.m_Format &&
			m_StateBefore == rhs.m_StateBefore &&
			m_StateAfter == rhs.m_StateAfter &&
			m_SampleCount == rhs.m_SampleCount &&
			m_LoadOp == rhs.m_LoadOp &&
			m_StoreOp == rhs.m_StoreOp;
	}

	constexpr bool operator!=(const RenderPassAttachment& rhs) const
	{
		return !(*this == rhs);
	}
};


struct RenderPassDesc
{
	const RenderPassAttachment* m_pAttachments = nullptr;
	const SubpassDesc* m_pSubpasses = nullptr;
	const SubpassDependency* m_pSubpassDependencies = nullptr;
	u32 m_AttachmentCount = 0;
	u32 m_SubpassCount = 0;
	u32 m_SubpassDependencyCount = 0;

	bool operator==(const RenderPassDesc& rhs) const noexcept
	{
		return true;
	}

	bool operator!=(const RenderPassDesc& rhs) const noexcept
	{
		return !(*this == rhs);
	}
};


template<>
struct std::hash<RenderPassDesc>
{
	size_t operator()(const RenderPassDesc& renderPassDesc) const
	{
		u64 hash = 0;
		HashCombine(hash, renderPassDesc.m_AttachmentCount);
		for (auto i = 0u; i < renderPassDesc.m_AttachmentCount; ++i)
		{
			auto& attachment = renderPassDesc.m_pAttachments[i];
			HashCombine(hash, attachment.m_Format, attachment.m_StateBefore, attachment.m_StateAfter, attachment.m_SampleCount,
				attachment.m_LoadOp, attachment.m_StoreOp);
		}

		HashCombine(hash, renderPassDesc.m_SubpassCount);
		for (auto i = 0u; i < renderPassDesc.m_SubpassCount; ++i)
		{
			auto& subpass = renderPassDesc.m_pSubpasses[i];
			HashCombine(hash, subpass.m_ColorAttachmentCount);
			for (auto j = 0u; j < subpass.m_ColorAttachmentCount; ++j)
			{
				auto& attachment = subpass.m_pColorAttachments[j];
				HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
			}

			if (subpass.m_pDepthStencilAttachment)
			{
				HashCombine(hash, subpass.m_pDepthStencilAttachment->m_Index, subpass.m_pDepthStencilAttachment->m_ResourceState);
			}

			HashCombine(hash, subpass.m_InputAttachmentCount);
			for (auto j = 0u; j < subpass.m_InputAttachmentCount; ++j)
			{
				auto& attachment = subpass.m_pInputAttachments[j];
				HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
			}

			if (subpass.m_pResolveAttachments)
			{
				for (auto j = 0u; j < subpass.m_ColorAttachmentCount; ++j)
				{
					auto& attachment = subpass.m_pResolveAttachments[j];
					HashCombine(hash, attachment.m_Index, attachment.m_ResourceState);
				}
			}

			if (subpass.m_pShadingRateAttachment)
			{
				HashCombine(hash, subpass.m_pShadingRateAttachment->m_Index, subpass.m_pShadingRateAttachment->m_ResourceState,
					subpass.m_pShadingRateAttachment->m_TexelSizes[0], subpass.m_pShadingRateAttachment->m_TexelSizes[1]);
			}
		}

		HashCombine(hash, renderPassDesc.m_SubpassDependencyCount);
		for (auto i = 0u; i < renderPassDesc.m_SubpassDependencyCount; ++i)
		{
			auto& subpassDep = renderPassDesc.m_pSubpassDependencies[i];
			HashCombine(hash, subpassDep.m_SrcSubpass, subpassDep.m_DstSubpass,
				subpassDep.m_SrcAccess, subpassDep.m_DstAccess, subpassDep.m_SrcStage, subpassDep.m_DstStage);
		}

		return hash;
	}
};


BV_OBJECT_DEFINE_ID(BvRenderPass, "cdaf2dfb-3ab4-458f-846b-f560d415a023");
class BvRenderPass : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(BvRenderPass);

public:
	BV_INLINE const RenderPassDesc& GetDesc() const { return m_RenderPassDesc; }

protected:
	BvRenderPass(const RenderPassDesc& renderPassDesc)
		: m_RenderPassDesc(renderPassDesc) {}
	~BvRenderPass() {}

protected:
	RenderPassDesc m_RenderPassDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderPass);