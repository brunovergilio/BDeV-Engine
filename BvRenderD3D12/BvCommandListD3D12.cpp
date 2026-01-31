#include "BvCommandListD3D12.h"
#include "BvRenderEngineD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvTextureViewD3D12.h"
#include "BvRenderPassD3D12.h"
#include "BvPipelineStateD3D12.h"
#include "BvBufferD3D12.h"
#include "BvTypeConversionsD3D12.h"
#include "BvCommandAllocatorD3D12.h"
#include "BvShaderResourceD3D12.h"
#include "BvTextureD3D12.h"
#include "BvSwapChainD3D12.h"
#include "BvBufferViewD3D12.h"
#include "BvSamplerD3D12.h"
#include "BvQueryD3D12.h"
#include "BvCommandQueueD3D12.h"
#include "BvCommandContextD3D12.h"
//#include "BvAccelerationStructureD3D12.h"
//#include "BvShaderBindingTableD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderAPIUtils.h"


BvCommandListD3D12::BvCommandListD3D12(BvRenderDeviceD3D12* pDevice, ID3D12CommandAllocator* pCommandAllocator,
	ID3D12GraphicsCommandList* pCommandList, BvFrameDataD3D12* pFrameData)
	: m_pDevice(pDevice), m_pCommandAllocator(pCommandAllocator), m_CommandList(pCommandList), m_pFrameData(pFrameData),
	m_pDrawIndirectSig(pDevice->GetDrawIndirectSig()), m_pDrawIndexedIndirectSig(pDevice->GetDrawIndexedIndirectSig()),
	m_pDispatchIndirectSig(pDevice->GetDispatchIndirectSig()), m_pDispatchMeshIndirectSig(pDevice->GetDispatchMeshIndirectSig()),
	m_pDispatchRaysIndirectSig(pDevice->GetDispatchRaysIndirectSig())
{
	m_CommandList.As(&m_CommandListEx);
}


BvCommandListD3D12::~BvCommandListD3D12()
{
}


void BvCommandListD3D12::Reset()
{
	m_CommandList->Reset(m_pCommandAllocator, nullptr);
}


void BvCommandListD3D12::Begin()
{
	m_SwapChains.Clear();
	m_pGraphicsPipeline = nullptr;
	m_pComputePipeline = nullptr;
	m_pShaderResourceLayout = nullptr;

	ID3D12DescriptorHeap* pHeaps[] =
	{
		m_pDevice->GetGPUShaderHeap()->GetHandle(),
		m_pDevice->GetGPUSamplerHeap()->GetHandle()
	};

	m_CommandList->SetDescriptorHeaps(2, pHeaps);

	m_CurrentState = State::kRecording;
}


void BvCommandListD3D12::Close()
{
	ResetRenderTargets();

	m_CommandList->Close();
}


void BvCommandListD3D12::BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets)
{
	BV_ASSERT(false, "Not implemented");

	BV_ASSERT(pRenderPass != nullptr, "Invalid render pass");
	BV_ASSERT(pRenderPassTargets != nullptr, "No render / depth targets");
	BV_ASSERT(renderPassTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

	ResetRenderTargets();

	m_CurrentState = State::kRenderPass;
}


void BvCommandListD3D12::NextSubpass()
{
	BV_ASSERT(false, "Not implemented");
}


void BvCommandListD3D12::EndRenderPass()
{
	BV_ASSERT(false, "Not implemented");

	BV_ASSERT(m_CurrentState == State::kRenderPass, "Command buffer not in render pass");

	m_CurrentState = State::kRecording;
}


void BvCommandListD3D12::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets, u32 multiviewCount)
{
	ResetRenderTargets();

	if (!renderTargetCount)
	{
		return;
	}

	BV_ASSERT(renderTargetCount <= kMaxRenderTargetsWithDepth, "Too many render targets");

	u32 colorAttachmentCount = 0;
	u32 resolveCount = 0;
	bool hasDepth = false;
	bool hasStencil = false;
	bool hasShadingRate = false;

	BvFixedVector<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxRenderTargets> rtvs;
	BvFixedVector<const f32*, kMaxRenderTargets> rtvColors;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv{};
	f32 depthColor{};
	u8 stencil{};

	u32 layerCount = kU32Max;
	for (u32 i = 0; i < renderTargetCount; i++)
	{
		if (colorAttachmentCount >= 8)
		{
			continue;
		}

		const auto& renderTarget = pRenderTargets[i];
		auto pView = TO_D3D12(renderTarget.m_pView);
		auto pTexture = TO_D3D12(renderTarget.m_pView->GetDesc().m_pTexture);
		auto& desc = renderTarget.m_pView->GetDesc().m_pTexture->GetDesc();
		auto& viewDesc = renderTarget.m_pView->GetDesc();

		layerCount = std::min(layerCount, (viewDesc.m_SubresourceDesc.layerCount == kU32Max ? desc.m_ArraySize : viewDesc.m_SubresourceDesc.layerCount)
			- viewDesc.m_SubresourceDesc.firstLayer);

		auto fi = BvRenderUtils::GetFormatInfo(viewDesc.m_Format);
		bool isColorTarget = !fi.m_IsDepth && !fi.m_IsStencil;
		bool isDepthTarget = fi.m_IsDepth;
		bool isStencilTarget = fi.m_IsStencil;
		bool isShadingRate = renderTarget.m_State == ResourceState::kShadingRate
			|| renderTarget.m_ShadingRateTexelSizes[0] != 0 || renderTarget.m_ShadingRateTexelSizes[1] != 0;
		bool isResolveImage = renderTarget.m_ResolveMode != ResolveMode::kNone;

		if (isShadingRate)
		{
			BV_ASSERT(false, "Not Implemented");
		}
		else if (isColorTarget)
		{
			if (isResolveImage)
			{
				BV_ASSERT(false, "Not Implemented");
			}

			rtvs.EmplaceBack(pView->GetRTV());
			rtvColors.EmplaceBack(renderTarget.m_ClearValues.colors);
		}
		else if (isDepthTarget)
		{
			hasDepth = true;
			hasStencil = isStencilTarget;
			if (isResolveImage)
			{
				BV_ASSERT(false, "Not Implemented");
			}

			dsv = pView->GetDSV();
			depthColor = renderTarget.m_ClearValues.depth;
			stencil = renderTarget.m_ClearValues.stencil;
		}

		auto pResource = pTexture->GetHandle();
		if (renderTarget.m_StateBefore != renderTarget.m_State)
		{
			auto& barrier = m_PreRenderBarriers.PushBack({});
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.StateBefore = GetD3D12ResourceState(renderTarget.m_StateBefore);
			barrier.Transition.StateAfter = GetD3D12ResourceState(renderTarget.m_State);
			barrier.Transition.Subresource = kU32Max;
			barrier.Transition.pResource = pResource;
		}
		if (renderTarget.m_State != renderTarget.m_StateAfter)
		{
			auto& barrier = m_PostRenderBarriers.PushBack({});
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.StateBefore = GetD3D12ResourceState(renderTarget.m_State);
			barrier.Transition.StateAfter = GetD3D12ResourceState(renderTarget.m_StateAfter);
			barrier.Transition.Subresource = kU32Max;
			barrier.Transition.pResource = pResource;
		}
	}

	if (m_PreRenderBarriers.Size() > 0)
	{
		m_CommandList->ResourceBarrier(m_PreRenderBarriers.Size(), m_PreRenderBarriers.Data());

		m_PreRenderBarriers.Clear();
	}

	m_CommandList->OMSetRenderTargets(rtvs.Size(), rtvs.Size() ? rtvs.Data() : nullptr, FALSE, dsv.ptr ? &dsv : nullptr);
	for (auto i = 0; i < rtvs.Size(); i++)
	{
		m_CommandList->ClearRenderTargetView(rtvs[i], rtvColors[i], 0, nullptr);
	}
	if (hasDepth)
	{
		m_CommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | (hasStencil ? D3D12_CLEAR_FLAG_STENCIL : D3D12_CLEAR_FLAGS(0)),
			depthColor, stencil, 0, nullptr);
	}

	m_CurrentState = State::kRenderTarget;
}


void BvCommandListD3D12::SetViewports(u32 viewportCount, const Viewport* pViewports)
{
	constexpr u32 kMaxViewports = 16;
	BV_ASSERT(viewportCount <= kMaxViewports, "Viewport count greater than limit");
	D3D12_VIEWPORT vps[kMaxViewports];
	for (auto i = 0u; i < viewportCount; ++i)
	{
		vps[i].TopLeftX = pViewports[i].m_X;
		vps[i].TopLeftY = pViewports[i].m_Y;
		vps[i].Width = pViewports[i].m_Width;
		vps[i].Height = pViewports[i].m_Height;
		vps[i].MinDepth = pViewports[i].m_MinDepth;
		vps[i].MaxDepth = pViewports[i].m_MaxDepth;
	}

	m_CommandList->RSSetViewports(viewportCount, vps);
}


void BvCommandListD3D12::SetScissors(u32 scissorCount, const Rect* pScissors)
{
	constexpr u32 kMaxScissors = 16;
	BV_ASSERT(scissorCount <= kMaxScissors, "Scissor count greater than limit");
	D3D12_RECT scissors[kMaxScissors];
	for (auto i = 0u; i < scissorCount; ++i)
	{
		scissors[i].left = pScissors[i].m_Left;
		scissors[i].top = pScissors[i].m_Top;
		scissors[i].right = pScissors[i].m_Right;
		scissors[i].bottom = pScissors[i].m_Bottom;
	}

	m_CommandList->RSSetScissorRects(scissorCount, scissors);
}


void BvCommandListD3D12::SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline)
{
	auto pPSO = TO_D3D12(pPipeline);

	m_pComputePipeline = nullptr;
	m_pRayTracingPipeline = nullptr;
	m_pGraphicsPipeline = pPSO;
	m_pShaderResourceLayout = TO_D3D12(m_pGraphicsPipeline->GetDesc().m_pShaderResourceLayout);
	
	m_CommandList->SetPipelineState(m_pGraphicsPipeline->GetHandle());
	m_CommandList->SetGraphicsRootSignature(m_pGraphicsPipeline->GetRootSig());
	m_CommandList->IASetPrimitiveTopology(m_pGraphicsPipeline->GetPrimitiveTopology());
}


void BvCommandListD3D12::SetComputePipeline(const IBvComputePipelineState* pPipeline)
{
	BV_ASSERT(false, "Not Implemented");

	//m_pGraphicsPipeline = nullptr;
	//m_pRayTracingPipeline = nullptr;
	//m_pComputePipeline = TO_D3D12(pPipeline);
	//m_pShaderResourceLayout = TO_D3D12(m_pComputePipeline->GetDesc().m_pShaderResourceLayout);

	//m_CommandList->SetPipelineState(m_pComputePipeline->GetHandle());
	//m_CommandList->SetGraphicsRootSignature(m_pComputePipeline->GetRootSig());
}


void BvCommandListD3D12::SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline)
{
	BV_ASSERT(false, "Not Implemented");

	//m_pGraphicsPipeline = nullptr;
	//m_pComputePipeline = nullptr;
	//m_pRayTracingPipeline = TO_D3D12(pPipeline);
	//m_pShaderResourceLayout = TO_D3D12(m_pRayTracingPipeline->GetDesc().m_pShaderResourceLayout);
	
	//m_CommandList->SetPipelineState(m_pRayTracingPipeline->GetHandle());
	//m_CommandList->SetGraphicsRootSignature(m_pRayTracingPipeline->GetRootSig());
}


void BvCommandListD3D12::SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetCBV(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetSRV(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetUAV(), set, binding, startIndex + i);
	}
}

void BvCommandListD3D12::SetDynamicConstantBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::SetDynamicStructuredBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::SetDynamicRWStructuredBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetSRV(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetUAV(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetSRV(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetUAV(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetHandle(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	auto& bindingState = m_pFrameData->GetResourceBindingState();
	for (auto i = 0; i < count; ++i)
	{
		bindingState.SetResource(TO_D3D12(ppResources[i])->GetSRV(), set, binding, startIndex + i);
	}
}


void BvCommandListD3D12::SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set)
{
	auto rootIndex = m_pShaderResourceLayout->GetRootSignatureSlot(binding, set);
	if (m_pGraphicsPipeline)
	{
		m_CommandList->SetGraphicsRoot32BitConstants(rootIndex, size >> 2, pData, 0);
	}
	else if (m_pComputePipeline)
	{
		m_CommandList->SetComputeRoot32BitConstants(rootIndex, size >> 2, pData, 0);
	}
}


void BvCommandListD3D12::SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding)
{
	BV_ASSERT(vertexBufferCount <= kMaxVertexBuffers, "Vertex buffer view count greater than limit");

	D3D12_VERTEX_BUFFER_VIEW vertexBuffers[kMaxVertexBuffers];
	for (auto i = 0u; i < vertexBufferCount; i++)
	{
		if (pVertexBufferViews[i].m_pBuffer)
		{
			vertexBuffers[i].BufferLocation = TO_D3D12(pVertexBufferViews[i].m_pBuffer)->GetDeviceAddress() + pVertexBufferViews[i].m_Offset;
			vertexBuffers[i].SizeInBytes = pVertexBufferViews[i].m_pBuffer->GetDesc().m_Size - pVertexBufferViews[i].m_Offset;
			vertexBuffers[i].StrideInBytes = pVertexBufferViews[i].m_Stride;
		}
	}
	
	m_CommandList->IASetVertexBuffers(firstBinding, vertexBufferCount, vertexBuffers);
}


void BvCommandListD3D12::SetIndexBufferView(const IndexBufferView& indexBufferView)
{
	D3D12_INDEX_BUFFER_VIEW ibv{ indexBufferView.m_pBuffer->GetDeviceAddress() + indexBufferView.m_Offset,
		u32(indexBufferView.m_pBuffer->GetDesc().m_Size - indexBufferView.m_Offset), GetD3D12IndexFormat(indexBufferView.m_IndexFormat) };
	m_CommandList->IASetIndexBuffer(&ibv);
}


void BvCommandListD3D12::SetDepthBounds(f32 min, f32 max)
{
	m_CommandListEx->OMSetDepthBounds(min, max);
}


void BvCommandListD3D12::SetStencilRef(u32 stencilRef)
{
	m_CommandList->OMSetStencilRef(stencilRef);
}


void BvCommandListD3D12::SetBlendConstants(const f32(&colors)[4])
{
	m_CommandList->OMSetBlendFactor(colors);
}


void BvCommandListD3D12::SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp(pCombinerOps[2]))
{
	BV_ASSERT(false, "Not done yet");
	//m_CommandListEx->RSSetShadingRate();
}


void BvCommandListD3D12::Draw(const DrawCommandArgs& args)
{
	FlushDescriptorSets();

	m_CommandList->DrawInstanced(args.m_VertexCount, args.m_InstanceCount, args.m_FirstVertex, args.m_FirstInstance);
}


void BvCommandListD3D12::DrawIndexed(const DrawIndexedCommandArgs& args)
{
	FlushDescriptorSets();

	m_CommandList->DrawIndexedInstanced(args.m_IndexCount, args.m_InstanceCount, args.m_FirstIndex, args.m_VertexOffset, args.m_FirstInstance);
}


void BvCommandListD3D12::Dispatch(const DispatchCommandArgs& args)
{
	FlushDescriptorSets();

	m_CommandList->Dispatch(args.m_ThreadGroupCountX, args.m_ThreadGroupCountY, args.m_ThreadGroupCountZ);
}


void BvCommandListD3D12::DispatchMesh(const DispatchMeshCommandArgs& args)
{
	FlushDescriptorSets();

	m_CommandListEx->DispatchMesh(args.m_ThreadGroupCountX, args.m_ThreadGroupCountY, args.m_ThreadGroupCountZ);
}


void BvCommandListD3D12::DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	m_CommandList->ExecuteIndirect(m_pDrawIndirectSig, drawCount, TO_D3D12(pBuffer)->GetHandle(), offset, nullptr, 0);
}


void BvCommandListD3D12::DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount, u64 offset)
{
	FlushDescriptorSets();

	m_CommandList->ExecuteIndirect(m_pDrawIndexedIndirectSig, drawCount, TO_D3D12(pBuffer)->GetHandle(), offset, nullptr, 0);
}


void BvCommandListD3D12::DispatchIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	m_CommandList->ExecuteIndirect(m_pDrawIndexedIndirectSig, 1, TO_D3D12(pBuffer)->GetHandle(), offset, nullptr, 0);
}


void BvCommandListD3D12::DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	m_CommandList->ExecuteIndirect(m_pDrawIndexedIndirectSig, 1, TO_D3D12(pBuffer)->GetHandle(), offset, nullptr, 0);
}


void BvCommandListD3D12::DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount)
{
	FlushDescriptorSets();

	m_CommandList->ExecuteIndirect(m_pDrawIndexedIndirectSig, 1, TO_D3D12(pBuffer)->GetHandle(), offset, nullptr, 0);
}


void BvCommandListD3D12::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer)
{
	ResetRenderTargets();

	auto pSrc = TO_D3D12(pSrcBuffer);
	auto pDst = TO_D3D12(pDstBuffer);
	m_CommandList->CopyResource(pDst->GetHandle(), pSrc->GetHandle());
}


void BvCommandListD3D12::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc)
{
	ResetRenderTargets();

	auto pSrc = TO_D3D12(pSrcBuffer);
	auto pDst = TO_D3D12(pDstBuffer);
	m_CommandList->CopyBufferRegion(pDst->GetHandle(), copyDesc.m_DstOffset, pSrc->GetHandle(), copyDesc.m_SrcOffset, copyDesc.m_SrcSize);
}


void BvCommandListD3D12::CopyBuffer(ID3D12Resource* pSrc, ID3D12Resource* pDst, const BufferCopyDesc& copyDesc)
{
	m_CommandList->CopyBufferRegion(pDst, copyDesc.m_DstOffset, pSrc, copyDesc.m_SrcOffset, copyDesc.m_SrcSize);
}


void BvCommandListD3D12::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture)
{
	ResetRenderTargets();

	auto pSrc = TO_D3D12(pSrcTexture);
	auto pDst = TO_D3D12(pDstTexture);
	m_CommandList->CopyResource(pDst->GetHandle(), pSrc->GetHandle());
}


void BvCommandListD3D12::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc)
{
	ResetRenderTargets();

	auto pSrc = TO_D3D12(pSrcTexture);
	auto pDst = TO_D3D12(pDstTexture);

	auto& srcDesc = pSrc->GetDesc();
	auto& dstDesc = pDst->GetDesc();

	D3D12_TEXTURE_COPY_LOCATION src{}, dst{};
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = BvRenderUtils::CalcSubresourceIndex(copyDesc.m_SrcMip, copyDesc.m_SrcLayer, copyDesc.m_SrcPlane,
		srcDesc.m_MipLevels, srcDesc.m_ArraySize);
	src.pResource = pSrc->GetHandle();

	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = BvRenderUtils::CalcSubresourceIndex(copyDesc.m_DstMip, copyDesc.m_DstLayer, copyDesc.m_DstPlane,
		dstDesc.m_MipLevels, dstDesc.m_ArraySize);
	dst.pResource = pDst->GetHandle();

	D3D12_BOX box
	{
		(u32)copyDesc.m_SrcTextureOffset.m_X, (u32)copyDesc.m_SrcTextureOffset.m_Y, (u32)copyDesc.m_SrcTextureOffset.m_Z,
		copyDesc.m_Size.m_X, copyDesc.m_Size.m_Y, copyDesc.m_Size.m_Z
	};

	m_CommandList->CopyTextureRegion(&dst, copyDesc.m_DstTextureOffset.m_X, copyDesc.m_DstTextureOffset.m_Y, copyDesc.m_DstTextureOffset.m_Z,
		&src, &box);
}


void BvCommandListD3D12::CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = TO_D3D12(pSrcBuffer);
	auto pDst = TO_D3D12(pDstTexture);

	auto& srcDesc = pSrc->GetDesc();
	auto& dstDesc = pDst->GetDesc();

	for (auto i = 0; i < copyCount; i++)
	{
		D3D12_RESOURCE_DESC textureDesc = pDst->GetHandle()->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint{};

		auto& copyDesc = pCopyDescs[i];
		auto subresourceIndex = BvRenderUtils::CalcSubresourceIndex(copyDesc.m_Mip, copyDesc.m_Layer, copyDesc.m_Plane,
			dstDesc.m_MipLevels, dstDesc.m_ArraySize);

		auto& subresource = copyDesc.m_SubresourceFootprint.m_Subresource;
		placedFootprint.Offset = copyDesc.m_SubresourceFootprint.m_Offset;
		placedFootprint.Footprint.Format = (DXGI_FORMAT)subresource.m_Format;
		placedFootprint.Footprint.Width = subresource.m_Width;
		placedFootprint.Footprint.Height = subresource.m_Height;
		placedFootprint.Footprint.Depth = subresource.m_Detph;
		placedFootprint.Footprint.RowPitch = subresource.m_RowPitch;

		D3D12_TEXTURE_COPY_LOCATION src{}, dst{};
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = placedFootprint;
		src.pResource = pSrc->GetHandle();

		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = subresourceIndex;
		dst.pResource = pDst->GetHandle();
		
		m_CommandList->CopyTextureRegion(&dst, copyDesc.m_TextureOffset.m_X, copyDesc.m_TextureOffset.m_Y, copyDesc.m_TextureOffset.m_Z, &src, nullptr);
	}
}


void BvCommandListD3D12::CopyBufferToTexture(ID3D12Resource* pSrc, ID3D12Resource* pDst, u32 copyCount, const SubresourceFootprint* pCopyDescs)
{
	for (auto subresourceIndex = 0; subresourceIndex < copyCount; subresourceIndex++)
	{
		D3D12_RESOURCE_DESC textureDesc = pDst->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint{};

		auto& footprint = pCopyDescs[subresourceIndex];
		auto& subresource = footprint.m_Subresource;
		placedFootprint.Offset = footprint.m_Offset;
		placedFootprint.Footprint.Format = (DXGI_FORMAT)subresource.m_Format;
		placedFootprint.Footprint.Width = subresource.m_Width;
		placedFootprint.Footprint.Height = subresource.m_Height;
		placedFootprint.Footprint.Depth = subresource.m_Detph;
		placedFootprint.Footprint.RowPitch = subresource.m_RowPitch;

		D3D12_TEXTURE_COPY_LOCATION src{}, dst{};
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = placedFootprint;
		src.pResource = pSrc;

		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = subresourceIndex;
		dst.pResource = pDst;

		m_CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	}
}


void BvCommandListD3D12::CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	auto pSrc = TO_D3D12(pSrcTexture);
	auto pDst = TO_D3D12(pDstBuffer);

	auto& srcDesc = pSrc->GetDesc();
	auto& dstDesc = pDst->GetDesc();

	for (auto i = 0; i < copyCount; i++)
	{
		D3D12_RESOURCE_DESC textureDesc = pDst->GetHandle()->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint{};

		auto& copyDesc = pCopyDescs[i];
		auto subresourceIndex = BvRenderUtils::CalcSubresourceIndex(copyDesc.m_Mip, copyDesc.m_Layer, copyDesc.m_Plane,
			srcDesc.m_MipLevels, srcDesc.m_ArraySize);

		auto& subresource = copyDesc.m_SubresourceFootprint.m_Subresource;
		placedFootprint.Offset = copyDesc.m_SubresourceFootprint.m_Offset;
		placedFootprint.Footprint.Format = (DXGI_FORMAT)subresource.m_Format;
		placedFootprint.Footprint.Width = subresource.m_Width;
		placedFootprint.Footprint.Height = subresource.m_Height;
		placedFootprint.Footprint.Depth = subresource.m_Detph;
		placedFootprint.Footprint.RowPitch = subresource.m_RowPitch;

		D3D12_TEXTURE_COPY_LOCATION src{}, dst{};
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = subresourceIndex;
		src.pResource = pSrc->GetHandle();

		dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dst.PlacedFootprint = placedFootprint;
		dst.pResource = pDst->GetHandle();

		bool useBox = copyDesc.m_TextureOffset.m_X || copyDesc.m_TextureOffset.m_Y || copyDesc.m_TextureOffset.m_Z;
		D3D12_BOX box
		{
			(u32)copyDesc.m_TextureOffset.m_X, (u32)copyDesc.m_TextureOffset.m_Y, (u32)copyDesc.m_TextureOffset.m_Z,
			copyDesc.m_TextureSize.m_X - (u32)copyDesc.m_TextureOffset.m_X,
			copyDesc.m_TextureSize.m_Y - (u32)copyDesc.m_TextureOffset.m_Y,
			copyDesc.m_TextureSize.m_Z - (u32)copyDesc.m_TextureOffset.m_Z
		};

		m_CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, useBox ? &box : nullptr);
	}
}


void BvCommandListD3D12::ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers)
{
	ResetRenderTargets();

	BvVector<D3D12_RESOURCE_BARRIER> barriers;
	barriers.Reserve(barrierCount);

	for (auto i = 0; i < barrierCount; i++)
	{
		auto& barrier = barriers.PushBack({});
		auto pResource = pBarriers[i].m_pTexture ? TO_D3D12(pBarriers[i].m_pTexture)->GetHandle() :
			(pBarriers[i].m_pBuffer ? TO_D3D12(pBarriers[i].m_pBuffer)->GetHandle() : nullptr);
		if (pBarriers[i].m_Type == ResourceBarrierDesc::Type::kMemory)
		{
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.UAV.pResource = pResource;
		}
		else if (pBarriers[i].m_Type == ResourceBarrierDesc::Type::kStateTransition)
		{
			auto& subresource = pBarriers[i].m_Subresource;
			u32 subresourceIndex = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			if (subresource.mipCount == 1 && subresource.layerCount == 1 && pBarriers[i].m_pTexture)
			{
				auto& desc = pBarriers[i].m_pTexture->GetDesc();
				subresourceIndex = BvRenderUtils::CalcSubresourceIndex(subresource.firstMip, subresource.firstLayer, subresource.planeSlice,
					desc.m_MipLevels, desc.m_ArraySize);
			}

			barrier.Transition.pResource = pResource;
			barrier.Transition.Subresource = subresourceIndex;
			barrier.Transition.StateBefore = GetD3D12ResourceState(pBarriers[i].m_SrcState);
			barrier.Transition.StateAfter = GetD3D12ResourceState(pBarriers[i].m_DstState);

			if (pBarriers[i].m_pSrcContext && pBarriers[i].m_pDstContext)
			{
				BV_ASSERT(false, "Not Implemented");
			}
		}
	}
}


void BvCommandListD3D12::SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp)
{
	if (pBuffer)
	{
		auto pBufferD3D12 = TO_D3D12(pBuffer);
		m_CommandList->SetPredication(pBufferD3D12->GetHandle(), offset, GetD3D12PredicationOp(predicationOp));
	}
	else
	{
		m_CommandList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
	}
}


void BvCommandListD3D12::BeginQuery(IBvQuery* pQuery)
{
	auto pQueryD3D12 = TO_D3D12(pQuery);
	auto queryType = pQueryD3D12->GetQueryType();
	auto pData = pQueryD3D12->Allocate(m_pFrameData->GetQueryHeapManager(), m_pFrameData->GetFrameIndex());
	if (queryType != QueryType::kTimestamp)
	{
		m_CommandList->BeginQuery(pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex), GetD3D12QueryType(queryType), pData->m_QueryIndex);
	}

	m_pFrameData->AddQuery(pQueryD3D12);
}


void BvCommandListD3D12::EndQuery(IBvQuery* pQuery)
{
	ResetRenderTargets();

	auto pQueryD3D12 = TO_D3D12(pQuery);
	auto queryType = pQueryD3D12->GetQueryType();
	auto frameIndex = m_pFrameData->GetFrameIndex();
	auto pData = pQueryD3D12->GetQueryData(frameIndex);
	auto pHeap = pData->m_pQueryHeap->GetHandle(pData->m_HeapIndex);
	m_CommandList->EndQuery(pHeap, GetD3D12QueryType(queryType), pData->m_QueryIndex);

	u64 stride = 0, offset = 0;
	ID3D12Resource* pBuffer = nullptr;
	pData->m_pQueryHeap->GetBufferInformation(pData->m_HeapIndex, frameIndex, pData->m_QueryIndex, pBuffer, offset, stride);
	m_CommandList->ResolveQueryData(pHeap, GetD3D12QueryType(queryType), pData->m_QueryIndex, 1, pBuffer, offset);
}


void BvCommandListD3D12::BeginEvent(const char* pName, const BvColor& color)
{
	PIXBeginEvent(m_CommandList.Get(), PIX_COLOR(color.m_Red, color.m_Green, color.m_Blue), pName);
}


void BvCommandListD3D12::EndEvent()
{
	PIXEndEvent(m_CommandList.Get());
}


void BvCommandListD3D12::SetMarker(const char* pName, const BvColor& color)
{
	PIXSetMarker(m_CommandList.Get(), PIX_COLOR(color.m_Red, color.m_Green, color.m_Blue), pName);
}


void BvCommandListD3D12::BuildBLAS(const BLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::BuildTLAS(const TLASBuildDesc& desc, const ASPostBuildDesc* pPostBuildDesc)
{
	BV_ASSERT(false, "Not Implemented");
}


//void BvCommandListD3D12::EmitASPostBuild(IBvAccelerationStructure* pAS, const ASPostBuildDesc& postBuildDesc)
//{
//	BV_ASSERT(false, "Not Implemented");
//}


void BvCommandListD3D12::CopyBLAS(const AccelerationStructureCopyDesc& copyDesc)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::CopyTLAS(const AccelerationStructureCopyDesc& copyDesc)
{
	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::DispatchRays(const DispatchRaysCommandArgs& args)
{
	FlushDescriptorSets();

	D3D12_DISPATCH_RAYS_DESC drd{ { args.m_RayGenShader.m_Address, args.m_RayGenShader.m_Size }, // For RayGen, stride == size
		{ args.m_MissShader.m_Address, args.m_MissShader.m_Stride, args.m_MissShader.m_Size },
		{ args.m_HitShader.m_Address, args.m_HitShader.m_Stride, args.m_HitShader.m_Size },
		{ args.m_CallableShader.m_Address, args.m_CallableShader.m_Stride, args.m_CallableShader.m_Size },
		args.m_Width, args.m_Height, args.m_Depth };
	m_CommandListEx->DispatchRays(&drd);
}


void BvCommandListD3D12::DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	FlushDescriptorSets();

	BV_ASSERT(false, "Not Implemented");
}


void BvCommandListD3D12::FlushDescriptorSets()
{
	auto& rbs = m_pFrameData->GetResourceBindingState();
	if (rbs.IsEmpty())
	{
		return;
	}

	BvVector<DescriptorData> descriptors;

	u64 hashSeed = 0;
	auto& params = m_pShaderResourceLayout->GetRootParams();
	for (auto rootIndex = 0; rootIndex < params.Size(); rootIndex++)
	{
		auto& rootParam = params[rootIndex];
		if (rootParam.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			continue;
		}

		u32 rangeOffset = 0;
		for (auto randeIndex = 0; randeIndex < rootParam.DescriptorTable.NumDescriptorRanges; randeIndex++, rangeOffset++)
		{
			auto& range = rootParam.DescriptorTable.pDescriptorRanges[randeIndex];

			for (auto descriptorIndex = 0u; descriptorIndex < range.NumDescriptors; descriptorIndex++)
			{
				ResourceIdD3D12 resId{ range.RegisterSpace, range.BaseShaderRegister, descriptorIndex };
				if (auto pResourceData = rbs.GetResource(resId))
				{
					descriptors.PushBack({ pResourceData->GetCPUHandle(), rangeOffset });
				}
				else
				{
					// TODO: ignore for bindless
					BV_ASSERT(false, "Needs a valid cpu descriptor handle");
				}
			}
		}

		// TODO: Add bindless option / mapping from layout
		D3D12_GPU_DESCRIPTOR_HANDLE dstHandle = m_pFrameData->RequestDescriptorHandle(rootIndex, m_pShaderResourceLayout, descriptors, false);

		if (m_pGraphicsPipeline)
		{
			m_CommandList->SetGraphicsRootDescriptorTable(rootIndex, dstHandle);
		}
		else if (m_pComputePipeline)
		{
			m_CommandList->SetComputeRootDescriptorTable(rootIndex, dstHandle);
		}

		descriptors.Clear();
	}
}


void BvCommandListD3D12::ResetRenderTargets()
{
	if (m_CurrentState == State::kRenderPass)
	{
		EndRenderPass();
	}
	else if (m_CurrentState == State::kRenderTarget)
	{
		m_CommandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
	}

	if (m_PostRenderBarriers.Size() > 0)
	{
		m_CommandList->ResourceBarrier(m_PostRenderBarriers.Size(), m_PostRenderBarriers.Data());
		m_PostRenderBarriers.Clear();
	}

	m_CurrentState = State::kRecording;
}