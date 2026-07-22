#include "BvRenderPassD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTypeConversionsD3D12.h"


BvRenderPassD3D12::BvRenderPassD3D12(BvRenderDeviceD3D12* pDevice, const RenderPassDesc& renderPassDesc)
	: m_RenderPassDesc(renderPassDesc), m_pDevice(pDevice)
{
	m_pBarrierData = BV_NEW(BarrierData)();
	auto& states = m_pBarrierData->m_States;
	auto& subpassBarrierIndices = m_pBarrierData->m_SubpassBarrierIndices;

	states.Reserve(m_RenderPassDesc.m_Attachments.Size() * (m_RenderPassDesc.m_Subpasses.Size() + 1));
	subpassBarrierIndices.Resize(m_RenderPassDesc.m_Subpasses.Size() + 1, {});

	BvVector<D3D12_RESOURCE_STATES> currStates(m_RenderPassDesc.m_Attachments.Size());
	for (auto i = 0u; i < m_RenderPassDesc.m_Attachments.Size(); i++)
	{
		currStates[i] = GetD3D12ResourceState(m_RenderPassDesc.m_Attachments[i].m_StateBefore);
	}

	u32 barrierStartIndex = 0;
	for (auto spIndex = 0u; spIndex < m_RenderPassDesc.m_Subpasses.Size(); spIndex++)
	{
		auto& sp = m_RenderPassDesc.m_Subpasses[spIndex];
		for (auto& ref : sp.m_ColorAttachments)
		{
			if (currStates[ref.m_Index] == D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				continue;
			}

			states.PushBack({ ref.m_Index, currStates[ref.m_Index], D3D12_RESOURCE_STATE_RENDER_TARGET });
			currStates[ref.m_Index] = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}

		for (auto& ref : sp.m_ResolveAttachments)
		{
			if (currStates[ref.m_Index] == D3D12_RESOURCE_STATE_RESOLVE_DEST)
			{
				continue;
			}

			states.PushBack({ ref.m_Index, currStates[ref.m_Index], D3D12_RESOURCE_STATE_RESOLVE_DEST });
			currStates[ref.m_Index] = D3D12_RESOURCE_STATE_RESOLVE_DEST;
		}

		if (sp.m_DepthStencilAttachment.IsValid())
		{
			auto& ref = sp.m_DepthStencilAttachment;
			auto newState = GetD3D12ResourceState(ref.m_ResourceState);
			if (currStates[ref.m_Index] != newState)
			{
				states.PushBack({ ref.m_Index, currStates[ref.m_Index], newState });
				currStates[ref.m_Index] = newState;
			}
		}

		if (sp.m_DepthStencilResolveAttachment.IsValid())
		{
			auto& ref = sp.m_DepthStencilResolveAttachment;
			if (currStates[ref.m_Index] != D3D12_RESOURCE_STATE_RESOLVE_DEST)
			{
				states.PushBack({ ref.m_Index, currStates[ref.m_Index], D3D12_RESOURCE_STATE_RESOLVE_DEST });
				currStates[ref.m_Index] = D3D12_RESOURCE_STATE_RESOLVE_DEST;
			}
		}

		for (auto& ref : sp.m_InputAttachments)
		{
			if (currStates[ref.m_Index] & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			{
				continue;
			}

			states.PushBack({ ref.m_Index, currStates[ref.m_Index], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE });
			currStates[ref.m_Index] = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}

		if (sp.m_ShadingRateAttachment.IsValid())
		{
			auto& ref = sp.m_ShadingRateAttachment;
			if (currStates[ref.m_Index] != D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE)
			{
				states.PushBack({ ref.m_Index, currStates[ref.m_Index], D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE });
				currStates[ref.m_Index] = D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
			}
		}

		subpassBarrierIndices[spIndex] = { barrierStartIndex, u32(states.Size()) - barrierStartIndex };
		barrierStartIndex = states.Size();
	}

	for (auto i = 0u; i < m_RenderPassDesc.m_Attachments.Size(); i++)
	{
		auto& attachment = m_RenderPassDesc.m_Attachments[i];
		auto finalState = GetD3D12ResourceState(attachment.m_StateAfter);
		if (currStates[i] == finalState)
		{
			continue;
		}

		states.PushBack({ i, currStates[i], finalState });

		currStates[i] = finalState;
	}

	subpassBarrierIndices.Back() = { barrierStartIndex, u32(states.Size()) - barrierStartIndex };

	for (auto i = 0u; i < m_RenderPassDesc.m_Attachments.Size(); i++)
	{
		if (m_RenderPassDesc.m_Attachments[i].m_LoadOp == LoadOp::kClear && currStates[i] != D3D12_RESOURCE_STATE_COMMON)
		{
			states.PushBack({ i, currStates[i], D3D12_RESOURCE_STATE_COMMON });
		}
	}
}


BvRenderPassD3D12::~BvRenderPassD3D12()
{
	Destroy();

	BV_DELETE(m_pBarrierData);
}


void BvRenderPassD3D12::Destroy()
{
}