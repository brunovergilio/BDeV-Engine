#include "BvCommandContextD3D12.h"
#include "BvGPUFenceD3D12.h"
#include "BvQueryHeapD3D12.h"
#include "BvDescriptorHeapD3D12.h"
#include "BvShaderResourceD3D12.h"
#include "BvRenderDeviceD3D12.h"
#include "BvCommandListD3D12.h"
#include "BvShaderBindingTableD3D12.h"
#include "BvUtilsD3D12.h"


BvFrameDataD3D12::BvFrameDataD3D12(BvRenderDeviceD3D12* pDevice, ContextDataD3D12* pContextData, CommandType commandType, u32 frameIndex)
	: m_pDevice(pDevice), m_pContextData(pContextData), m_CommandAllocator(pDevice, commandType), m_FrameIndex(frameIndex)
{
	auto fenceObj = D3D12Utils::CreateGPUFence(pDevice, { 0 });
	BV_ASSERT(SUCCEEDED(fenceObj.first), "Error creating gpu fence");
	auto pFence = BV_RC_CREATE(BvGPUFenceD3D12, pDevice, fenceObj.second.m_Fence, fenceObj.second.m_Event);
	m_Fence.Attach(pFence);
}


BvFrameDataD3D12::~BvFrameDataD3D12()
{
}


void BvFrameDataD3D12::Reset(bool newFrame /*= true*/)
{
	// If any resource was destroyed, make sure its descriptors have also been removed
	for (u32 numDeletedResources = m_pContextData->m_DeletedResourceCounter.load(std::memory_order_relaxed); numDeletedResources > 0;
		numDeletedResources = m_pContextData->m_DeletedResourceCounter.load(std::memory_order_relaxed))
	{
		constexpr u32 kDeletedResourceCopyCount = 8;
		D3D12_CPU_DESCRIPTOR_HANDLE deletedResources[kDeletedResourceCopyCount];
		u32 copyCount = std::min(numDeletedResources, kDeletedResourceCopyCount);

		{
			BvScopedLock lock(m_pContextData->m_DeletedResourceLock);
			for (auto i = 0; i < copyCount; i++)
			{
				deletedResources[i] = m_pContextData->m_DeletedResourceHandles[m_pContextData->m_DeletedResourceHandles.Size() - 1 - i];
				m_pContextData->m_DeletedResourceHandles.PopBack();
			}
		}

		// Update counter
		m_pContextData->m_DeletedResourceCounter.fetch_sub(copyCount, std::memory_order_relaxed);

		// Remove all destroyed resources from the list of active resources
		for (auto resourceIndex = 0; resourceIndex < copyCount; resourceIndex++)
		{
			auto resourceIt = m_pContextData->m_ActiveResources.FindKey(deletedResources[resourceIndex].ptr);
			if (resourceIt != m_pContextData->m_ActiveResources.cend())
			{
				for (auto& descriptorData : resourceIt->second)
				{
					// Recycle the descriptors
					descriptorData.m_pDescriptorPool->RecycleDescriptor(descriptorData.m_Descriptor);
					
					// Also remove said descriptor from the list of active descriptors
					for (auto setIt = m_pContextData->m_Descriptors.cbegin(); setIt != m_pContextData->m_Descriptors.cend(); setIt++)
					{
						if (setIt->second.GetGPUHandle() == descriptorData.m_Descriptor)
						{
							m_pContextData->m_Descriptors.Erase(setIt);
							break;
						}
					}
				}

				m_pContextData->m_ActiveResources.Erase(resourceIt);
			}
		}
	}

	if (newFrame)
	{
		m_Fence->Wait(m_FenceValue);
	}

	m_CommandAllocator.Reset();
	m_CommandLists.Clear();
	m_pContextData->m_ResourceBindingState.Reset();

	m_ASPostBuildBuffers.Clear();
}


BvCommandListD3D12* BvFrameDataD3D12::RequestCommandList()
{
	if (auto pCommandList = m_CommandAllocator.GetCommandList(this))
	{
		m_CommandLists.EmplaceBack(pCommandList);
		return pCommandList;
	}

	return nullptr;
}


BvDescriptorHandle BvFrameDataD3D12::RequestDescriptorHandle(u32 rootIndex, const BvShaderResourceLayoutD3D12* pLayout,
	const BvVector<DescriptorData>& descriptors, bool bindless /*= false*/)
{
	auto srlHash = (u64)pLayout;
	HashCombine(srlHash, rootIndex);
	auto& pPool = m_pContextData->m_DescriptorPools[srlHash];
	if (!pPool)
	{
		pPool = BV_NEW(BvDescriptorPoolD3D12)(m_pDevice, pLayout, rootIndex, bindless ? 1 : 16);
	}

	auto& descriptorMap = !bindless ? m_pContextData->m_Descriptors : m_pContextData->m_BindlessDescriptors;
	u64 descriptorHash = 0;
	if (!bindless)
	{
		for (auto& descriptor : descriptors)
		{
			HashCombine(descriptorHash, descriptor.m_CPUHandle.ptr, descriptor.m_RangeOffset);
		}
	}
	else
	{
		descriptorHash = srlHash;
	}

	auto pDevice = m_pDevice->GetHandle();
	auto heapType = pPool->GetHeapType();

	auto result = descriptorMap.Emplace(descriptorHash, BvDescriptorHandle{});
	if (result.second)
	{
		result.first->second = pPool->Allocate();
	}

	u32 handleSize = pPool->GetHandleSize();
	if (result.second || bindless)
	{
		auto gpuHandle = result.first->second;

		if (!bindless)
		{
			BV_ASSERT_ONCE(pPool->GetHandleCount() == descriptors.Size(), "Missing descriptors");
		}

		for (auto& descriptor : descriptors)
		{
			pDevice->CopyDescriptorsSimple(1, gpuHandle.GetByIndex(descriptor.m_RangeOffset, handleSize), descriptor.m_CPUHandle, heapType);

			if (!bindless)
			{
				auto activeDescriptorData = m_pContextData->m_ActiveResources.Emplace(descriptor.m_CPUHandle.ptr, BvVector<ContextDataD3D12::ActiveDescriptorData>{});
				activeDescriptorData.first->second.PushBack({ BvDescriptorHandle(descriptor.m_CPUHandle), pPool });
			}
		}
	}

	return result.first->second;
}


void BvFrameDataD3D12::ClearActiveCommandLists()
{
	m_CommandLists.Clear();
}


ID3D12Resource* BvFrameDataD3D12::AddASPostBuildBuffer(u64 size)
{
	BufferDesc bd;
	bd.SetSize(size).SetUsageFlags(BufferUsage::kRWStructuredBuffer);
	auto obj = D3D12Utils::CreateBuffer(m_pDevice, bd, sizeof(u64));
	BV_ASSERT(SUCCEEDED(obj.first), "Failed to create Acceleration Structure Post Build Buffer");

	PostBuildBuffer pbBuffer{ std::move(obj.second.m_Buffer), std::move(obj.second.m_Allocation) };
	m_ASPostBuildBuffers.PushBack(std::move(pbBuffer));

	return m_ASPostBuildBuffers.Back().m_Buffer.Get();
}


BvCommandContextD3D12::BvCommandContextD3D12(BvRenderDeviceD3D12* pDevice, u32 frameCount, u32 contextIndex, u32 contextGroupIndex, ID3D12CommandQueue* pQueue)
	: m_pDevice(pDevice), m_CommandQueue(pDevice, pQueue), m_ContextGroupIndex(contextGroupIndex), m_ContextIndex(contextIndex), m_FrameCount(frameCount)
{
	constexpr u32 kDefaultQueryPoolSizes[kQueryTypeCount] = { 8, 4, 4, 2, 2 };

	m_pContextData = BV_NEW(ContextDataD3D12)();

	auto pFrameData = reinterpret_cast<u8*>(BV_ALLOC(sizeof(BvFrameDataD3D12) * m_FrameCount, alignof(BvFrameDataD3D12)));
	for (auto i = 0; i < m_FrameCount; ++i)
	{
		new(pFrameData + i * sizeof(BvFrameDataD3D12)) BvFrameDataD3D12(pDevice, m_pContextData, CommandType(contextGroupIndex + 1), i);
	}
	m_pFrames = reinterpret_cast<BvFrameDataD3D12*>(pFrameData);
}


BvCommandContextD3D12::~BvCommandContextD3D12()
{
	for (auto i = 0; i < m_FrameCount; ++i)
	{
		m_pFrames[i].~BvFrameDataD3D12();
	}
	BV_FREE(m_pFrames);

	for (auto& poolData : m_pContextData->m_DescriptorPools)
	{
		BV_DELETE(poolData.second);
	}

	BV_DELETE(m_pContextData);
}


void BvCommandContextD3D12::NewCommandList()
{
	if (m_pCurrCommandList)
	{
		m_pCurrCommandList->Close();
	}

	// Get command list
	m_pCurrCommandList = m_pFrames[m_ActiveFrameIndex].RequestCommandList();
}


void BvCommandContextD3D12::AddWaitFence(IBvGPUFence* pFence, u64 value)
{
	// Add a wait fence and its value
	m_CommandQueue.AddWaitFence(TO_D3D12(pFence)->GetHandle(), value);
}


void BvCommandContextD3D12::AddSignalFence(IBvGPUFence* pFence, u64 value)
{
	// Add a signal fence and its value
	m_CommandQueue.AddSignalFence(TO_D3D12(pFence)->GetHandle(), value);
}


void BvCommandContextD3D12::Execute()
{
	// Submit active command buffers
	if (m_pCurrCommandList)
	{
		m_pCurrCommandList->Close();
	}

	auto& commandLists = m_pFrames[m_ActiveFrameIndex].GetCommandLists();
	for (auto pCommandList : commandLists)
	{
		m_CommandLists.EmplaceBack(pCommandList->GetHandle());
	}

	auto pFence = m_pFrames[m_ActiveFrameIndex].GetGPUFence();
	auto semaphoreValue = m_pFrames[m_ActiveFrameIndex].UpdateFenceValue();
	m_CommandQueue.Submit(m_CommandLists, pFence->GetHandle(), semaphoreValue);
	m_CommandLists.Clear();

	m_pFrames[m_ActiveFrameIndex].ClearActiveCommandLists();

	m_pCurrCommandList = nullptr;
}


void BvCommandContextD3D12::ExecuteAndWait()
{
	Execute();

	m_CommandQueue.WaitIdle();

	m_pFrames[m_ActiveFrameIndex].Reset(false);
}


void BvCommandContextD3D12::FlushFrame()
{
	// Sanity check
	BV_ASSERT(m_pCurrCommandList == nullptr, "All command lists must have been submitted");

	// Get next frame
	m_ActiveFrameIndex = (m_ActiveFrameIndex + 1) % m_FrameCount;

	// Wait for frame's signal value
	m_pFrames[m_ActiveFrameIndex].Reset();
}


void BvCommandContextD3D12::BeginRenderPass(const IBvRenderPass* pRenderPass, u32 renderPassTargetCount, const RenderPassTargetDesc* pRenderPassTargets)
{
	m_pCurrCommandList->BeginRenderPass(pRenderPass, renderPassTargetCount, pRenderPassTargets);
}


void BvCommandContextD3D12::NextSubpass()
{
	m_pCurrCommandList->NextSubpass();
}


void BvCommandContextD3D12::EndRenderPass()
{
	m_pCurrCommandList->EndRenderPass();
}


void BvCommandContextD3D12::SetRenderTargets(u32 renderTargetCount, const RenderTargetDesc* pRenderTargets, u32 multiviewCount)
{
	m_pCurrCommandList->SetRenderTargets(renderTargetCount, pRenderTargets, multiviewCount);
}


void BvCommandContextD3D12::SetViewports(u32 viewportCount, const Viewport* pViewports)
{
	m_pCurrCommandList->SetViewports(viewportCount, pViewports);
}


void BvCommandContextD3D12::SetScissors(u32 scissorCount, const Rect* pScissors)
{
	m_pCurrCommandList->SetScissors(scissorCount, pScissors);
}


void BvCommandContextD3D12::SetGraphicsPipeline(const IBvGraphicsPipelineState* pPipeline)
{
	m_pCurrCommandList->SetGraphicsPipeline(pPipeline);
}


void BvCommandContextD3D12::SetComputePipeline(const IBvComputePipelineState* pPipeline)
{
	m_pCurrCommandList->SetComputePipeline(pPipeline);
}


void BvCommandContextD3D12::SetRayTracingPipeline(const IBvRayTracingPipelineState* pPipeline)
{
	m_pCurrCommandList->SetRayTracingPipeline(pPipeline);
}


void BvCommandContextD3D12::SetShaderResourceParams(u32 resourceParamsCount, IBvShaderResourceParams* const* ppResourceParams, u32 startIndex)
{
	m_pCurrCommandList->SetShaderResourceParams(resourceParamsCount, ppResourceParams, startIndex);
}

void BvCommandContextD3D12::SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetConstantBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetStructuredBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetRWStructuredBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetDynamicConstantBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	m_pCurrCommandList->SetDynamicConstantBuffer(pResource, offset, set, binding);
}


void BvCommandContextD3D12::SetDynamicStructuredBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	m_pCurrCommandList->SetDynamicStructuredBuffer(pResource, offset, set, binding);
}


void BvCommandContextD3D12::SetDynamicRWStructuredBuffer(IBvBufferView* pResource, u32 offset, u32 set, u32 binding)
{
	m_pCurrCommandList->SetDynamicRWStructuredBuffer(pResource, offset, set, binding);
}


void BvCommandContextD3D12::SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetFormattedBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetRWFormattedBuffers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetTextures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetRWTextures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetSamplers(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetInputAttachments(u32 count, const IBvTextureView* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetInputAttachments(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 set, u32 binding, u32 startIndex)
{
	m_pCurrCommandList->SetAccelerationStructures(count, ppResources, set, binding, startIndex);
}


void BvCommandContextD3D12::SetShaderConstants(u32 size, const void* pData, u32 binding, u32 set)
{
	m_pCurrCommandList->SetShaderConstants(size, pData, binding, set);
}


void BvCommandContextD3D12::SetVertexBufferViews(u32 vertexBufferCount, const VertexBufferView* pVertexBufferViews, u32 firstBinding /*= 0*/)
{
	m_pCurrCommandList->SetVertexBufferViews(vertexBufferCount, pVertexBufferViews, firstBinding);
}


void BvCommandContextD3D12::SetIndexBufferView(const IndexBufferView& indexBufferView)
{
	m_pCurrCommandList->SetIndexBufferView(indexBufferView);
}


void BvCommandContextD3D12::SetDepthBounds(f32 min, f32 max)
{
	m_pCurrCommandList->SetDepthBounds(min, max);
}


void BvCommandContextD3D12::SetStencilRef(u32 stencilRef)
{
	m_pCurrCommandList->SetStencilRef(stencilRef);
}


void BvCommandContextD3D12::SetBlendConstants(const float(&colors)[4])
{
	m_pCurrCommandList->SetBlendConstants(colors);
}


void BvCommandContextD3D12::SetShadingRate(ShadingRateDimensions dimensions, ShadingRateCombinerOp(pCombinerOps[2]))
{
	m_pCurrCommandList->SetShadingRate(dimensions, pCombinerOps);
}


void BvCommandContextD3D12::Draw(const DrawCommandArgs& args)
{
	m_pCurrCommandList->Draw(args);
}


void BvCommandContextD3D12::DrawIndexed(const DrawIndexedCommandArgs& args)
{
	m_pCurrCommandList->DrawIndexed(args);
}


void BvCommandContextD3D12::Dispatch(const DispatchCommandArgs& args)
{
	m_pCurrCommandList->Dispatch(args);
}


void BvCommandContextD3D12::DispatchMesh(const DispatchMeshCommandArgs& args)
{
	m_pCurrCommandList->DispatchMesh(args);
}


void BvCommandContextD3D12::DrawIndirect(const IBvBuffer* pBuffer, u32 drawCount /*= 1*/, u64 offset /*= 0*/)
{
	m_pCurrCommandList->DrawIndirect(pBuffer, drawCount, offset);
}


void BvCommandContextD3D12::DrawIndexedIndirect(const IBvBuffer* pBuffer, u32 drawCount /*= 1*/, u64 offset /*= 0*/)
{
	m_pCurrCommandList->DrawIndexedIndirect(pBuffer, drawCount, offset);
}


void BvCommandContextD3D12::DispatchIndirect(const IBvBuffer* pBuffer, u64 offset /*= 0*/)
{
	m_pCurrCommandList->DispatchIndirect(pBuffer, offset);
}


void BvCommandContextD3D12::DispatchMeshIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	m_pCurrCommandList->DispatchMeshIndirect(pBuffer, offset);
}


void BvCommandContextD3D12::DispatchMeshIndirectCount(const IBvBuffer* pBuffer, u64 offset, const IBvBuffer* pCountBuffer, u64 countOffset, u32 maxCount)
{
	m_pCurrCommandList->DispatchMeshIndirectCount(pBuffer, offset, pCountBuffer, countOffset, maxCount);
}


void BvCommandContextD3D12::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer)
{
	m_pCurrCommandList->CopyBuffer(pSrcBuffer, pDstBuffer);
}


void BvCommandContextD3D12::CopyBuffer(const IBvBuffer* pSrcBuffer, IBvBuffer* pDstBuffer, const BufferCopyDesc& copyDesc)
{
	m_pCurrCommandList->CopyBuffer(pSrcBuffer, pDstBuffer, copyDesc);
}


void BvCommandContextD3D12::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture)
{
	m_pCurrCommandList->CopyTexture(pSrcTexture, pDstTexture);
}


void BvCommandContextD3D12::CopyTexture(const IBvTexture* pSrcTexture, IBvTexture* pDstTexture, const TextureCopyDesc& copyDesc)
{
	m_pCurrCommandList->CopyTexture(pSrcTexture, pDstTexture, copyDesc);
}


void BvCommandContextD3D12::CopyBufferToTexture(const IBvBuffer* pSrcBuffer, IBvTexture* pDstTexture, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	m_pCurrCommandList->CopyBufferToTexture(pSrcBuffer, pDstTexture, copyCount, pCopyDescs);
}


void BvCommandContextD3D12::CopyTextureToBuffer(const IBvTexture* pSrcTexture, IBvBuffer* pDstBuffer, u32 copyCount, const BufferTextureCopyDesc* pCopyDescs)
{
	m_pCurrCommandList->CopyTextureToBuffer(pSrcTexture, pDstBuffer, copyCount, pCopyDescs);
}


void BvCommandContextD3D12::ResourceBarrier(u32 barrierCount, const ResourceBarrierDesc* pBarriers)
{
	m_pCurrCommandList->ResourceBarrier(barrierCount, pBarriers);
}


void BvCommandContextD3D12::SetPredication(const IBvBuffer* pBuffer, u64 offset, PredicationOp predicationOp)
{
	m_pCurrCommandList->SetPredication(pBuffer, offset, predicationOp);
}


bool BvCommandContextD3D12::SupportsQueryType(QueryType queryType) const
{
	auto pDeviceInfo = m_pDevice->GetDeviceInfo();
	auto& contextGroup = m_pDevice->GetGPUInfo().m_ContextGroups[m_ContextGroupIndex];
	if (queryType == QueryType::kTimestamp)
	{
		if (contextGroup.m_DedicatedCommandType == CommandType::kGraphics || contextGroup.m_DedicatedCommandType == CommandType::kCompute)
		{
			return true;
		}
		else
		{
			return contextGroup.m_DedicatedCommandType == CommandType::kTransfer && pDeviceInfo->m_Options3.CopyQueueTimestampQueriesSupported;
		}
	}
	else if (queryType == QueryType::kMeshPipelineStatistics)
	{
		return EHasFlag(m_pDevice->GetGPUInfo().m_DeviceCaps, RenderDeviceCapabilities::kMeshQuery) && contextGroup.SupportsCommandType(CommandType::kGraphics);
	}

	return contextGroup.SupportsCommandType(CommandType::kGraphics);
}


void BvCommandContextD3D12::BeginQuery(IBvQueryHeap* pQueryHeap, u32 index)
{
	m_pCurrCommandList->BeginQuery(pQueryHeap, index);
}


void BvCommandContextD3D12::EndQuery(IBvQueryHeap* pQueryHeap, u32 index)
{
	m_pCurrCommandList->EndQuery(pQueryHeap, index);
}


void BvCommandContextD3D12::ResolveQueryData(IBvQueryHeap* pQueryHeap, u32 startIndex, u32 queryCount, IBvBuffer* pDstBuffer, u64 offset)
{
	m_pCurrCommandList->ResolveQueryData(pQueryHeap, startIndex, queryCount, pDstBuffer, offset);
}


void BvCommandContextD3D12::BeginEvent(const char* pName, const BvColor& color)
{
	m_pCurrCommandList->BeginEvent(pName, color);
}


void BvCommandContextD3D12::EndEvent()
{
	m_pCurrCommandList->EndEvent();
}


void BvCommandContextD3D12::SetMarker(const char* pName, const BvColor& color)
{
	m_pCurrCommandList->SetMarker(pName, color);
}


void BvCommandContextD3D12::BuildRayTracingAccelerationStructures(u32 count, const RayTracingAccelerationStructureBuildDesc* pBuildDescs,
	const RayTracingAccelerationStructurePostBuildDesc* pPostBuildDesc)
{
	m_pCurrCommandList->BuildRayTracingAccelerationStructures(count, pBuildDescs, pPostBuildDesc);
}


void BvCommandContextD3D12::EmitRayTracingAccelerationStructurePostBuild(u32 count, IBvAccelerationStructure* const* ppAccelerationStructures,
	const RayTracingAccelerationStructurePostBuildDesc& postBuildDesc)
{
	m_pCurrCommandList->EmitASPostBuild(count, ppAccelerationStructures, postBuildDesc);
}


void BvCommandContextD3D12::CopyRayTracingAccelerationStructure(const RayTracingAccelerationStructureCopyDesc& copyDesc)
{
	m_pCurrCommandList->CopyRayTracingAccelerationStructure(copyDesc);
}


void BvCommandContextD3D12::DispatchRays(const DispatchRaysCommandArgs& args)
{
	m_pCurrCommandList->DispatchRays(args);
}


void BvCommandContextD3D12::DispatchRays(IBvShaderBindingTable* pSBT, u32 rayGenIndex, u32 missIndex, u32 hitIndex, u32 callableIndex,
	u32 width, u32 height, u32 depth)
{
	DispatchRaysCommandArgs args;
	pSBT->GetDeviceAddressRange(ShaderBindingTableGroupType::kRayGen, rayGenIndex, args.m_RayGenShader);
	pSBT->GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType::kMiss, missIndex, args.m_MissShader);
	pSBT->GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType::kHit, hitIndex, args.m_HitShader);
	pSBT->GetDeviceAddressRangeAndStride(ShaderBindingTableGroupType::kCallable, callableIndex, args.m_CallableShader);
	args.m_Width = width;
	args.m_Height = height;
	args.m_Depth = depth;

	DispatchRays(args);
}


void BvCommandContextD3D12::DispatchRaysIndirect(const IBvBuffer* pBuffer, u64 offset)
{
	m_pCurrCommandList->DispatchRaysIndirect(pBuffer, offset);
}


void BvCommandContextD3D12::OnResourceDeleted(u32 numHandles, const D3D12_CPU_DESCRIPTOR_HANDLE* pHandles)
{
	BvScopedLock lock(m_pContextData->m_DeletedResourceLock);
	for (auto i = 0; i < numHandles; i++)
	{
		m_pContextData->m_DeletedResourceHandles.PushBack(pHandles[i]);
	}
	m_pContextData->m_DeletedResourceCounter.fetch_add(numHandles, std::memory_order_relaxed);
}


void BvCommandContextD3D12::Destroy()
{
}