#pragma once


#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Container/BvFixedVector.h"
#include "BvCommandPool.h"
#include "BvCommon.h"


class BvTextureView;
class BvRenderPass;
class BvGraphicsPipelineState;
class BvComputePipelineState;
class BvBuffer;
class BvBufferView;
class BvTexture;
class BvShaderResourceParams;
class BvCommandQueue;


struct ClearColorValue
{
	ClearColorValue()
		: r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	ClearColorValue(float r, float g, float b, float a = 1.0f)
		: r(r), g(g), b(b), a(a) {}
	ClearColorValue(float depth, u8 stencil = 0)
		: depth(depth), stencil(stencil) {}
	union
	{
		struct 
		{
			float r;
			float g;
			float b;
			float a;
		};
		struct 
		{
			float depth;
			u8 stencil;
		};
	};
};


enum class ClearFlags : u8
{
	kNone = 0,
	kClearDepth = BvBit(0),
	kClearStencil = BvBit(1),
	kClearDepthAndStencil = kClearDepth | kClearStencil
};
BV_USE_ENUM_CLASS_OPERATORS(ClearFlags);


struct Viewport
{
	float x;
	float y;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};


struct Rect
{
	i32 x;
	i32 y;
	u32 width;
	u32 height;
};


struct CopyRegion
{
	union
	{
		struct Buffer
		{
			u64 srcSize;
			u64 srcOffset;
			u64 dstOffset;
		} buffer;
		struct BufferTexture
		{
			u64 bufferOffset;
			Extent textureOffset;
			Extent textureSize;
			u32 mipLevel;
		} bufferTexture;
		struct Texture
		{
			Extent srcTextureOffset;
			u32 srcMipLevel;
			Extent dstTextureOffset;
			u32 dstMipLevel;
			Extent size;
		} texture;
	};
};


struct ResourceBarrierDesc
{
	BvTexture * pTexture = nullptr;
	BvBuffer * pBuffer = nullptr;

	BvCommandQueue * pSrcQueue = nullptr;
	BvCommandQueue * pDstQueue = nullptr;

	ResourceState srcLayout = ResourceState::kCommon;
	ResourceState dstLayout = ResourceState::kCommon;

	// These fields can be used for more detailed barriers (in Vulkan)
	ResourceAccess srcAccess = ResourceAccess::kAuto;
	ResourceAccess dstAccess = ResourceAccess::kAuto;

	PipelineStage srcPipelineStage = PipelineStage::kAuto;
	PipelineStage dstPipelineStage = PipelineStage::kAuto;

	SubresourceDesc subresource;
};


class BvCommandBuffer
{
	BV_NOCOPYMOVE(BvCommandBuffer);

public:
	enum class CurrentState
	{
		kReset,
		kRecording,
		kInRenderPass,
		kInRender,
		kRecorded
	};

	virtual void Reset() = 0;
	virtual void Begin() = 0;
	virtual void End() = 0;

	virtual void BeginRenderPass(const BvRenderPass * const pRenderPass, BvTextureView * const * const pRenderTargets,
		const ClearColorValue * const pClearColors = nullptr, BvTextureView * const pDepthStencilTarget = nullptr,
		const ClearColorValue & depthClear = ClearColorValue(1.0f, 0)) = 0;
	virtual void EndRenderPass() = 0;

	virtual void SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets, const ClearColorValue* const pClearColors,
		BvTextureView* const pDepthStencilTarget = nullptr, const ClearColorValue& depthClear = ClearColorValue(1.0f, 0),
		const ClearFlags clearFlags = ClearFlags::kClearDepthAndStencil) = 0;
	void SetRenderTargets(const u32 renderTargetCount, BvTextureView* const* const pRenderTargets, BvTextureView* const pDepthStencilTarget = nullptr)
	{
		SetRenderTargets(renderTargetCount, pRenderTargets, nullptr, pDepthStencilTarget);
	}
	virtual void ClearRenderTargets(u32 renderTargetCount, const ClearColorValue* const pClearValues, u32 firstRenderTargetIndex = 0) = 0;
	void ClearRenderTarget(u32 renderTargetIndex, const ClearColorValue& clearValue = ClearColorValue())
	{
		ClearRenderTargets(1, &clearValue);
	}

	virtual void SetViewports(const u32 viewportCount, const Viewport * const pViewports) = 0;
	void SetViewport(const Viewport & viewport) { SetViewports(1, &viewport); }
	void SetViewport(const f32 width, const f32 height, const f32 minDepth = 0.0f, const f32 maxDepth = 1.0f)
	{
		Viewport viewport{ 0.0f, 0.0f, width, height, minDepth, maxDepth };
		SetViewports(1, &viewport);
	}

	virtual void SetScissors(const u32 scissorCount, const Rect * const pScissors) = 0;
	void SetScissor(const Rect & scissor) { SetScissors(1, &scissor); }
	void SetScissor(const u32 width, const u32 height)
	{
		Rect scissor{ 0, 0, width, height };
		SetScissors(1, &scissor);
	}

	virtual void SetPipeline(const BvGraphicsPipelineState * const pPipeline) = 0;
	virtual void SetPipeline(const BvComputePipelineState * const pPipeline) = 0;

	virtual void SetShaderResourceParams(const u32 setCount, BvShaderResourceParams * const * const ppSets, const u32 firstSet = 0) = 0;

	virtual void SetVertexBufferViews(const u32 vertexBufferCount, const BvBufferView * const * const pVertexBufferViews,
		const u32 firstBinding = 0) = 0;
	void SetVertexBufferView(const BvBufferView * const pVertexBufferView, const u32 firstBinding = 0) { SetVertexBufferViews(1, &pVertexBufferView, firstBinding); }

	virtual void SetIndexBufferView(const BvBufferView * const pIndexBufferView, const IndexFormat indexFormat) = 0;

	virtual void Draw(const u32 vertexCount, const u32 instanceCount = 1,
		const u32 firstVertex = 0, const u32 firstInstance = 0) = 0;
	virtual void DrawIndexed(const u32 indexCount, const u32 instanceCount = 1, const u32 firstIndex = 0,
		const i32 vertexOffset = 0, const u32 firstInstance = 0) = 0;
	virtual void Dispatch(const u32 x, const u32 y = 1, const u32 z = 1) = 0;

	virtual void DrawIndirect(const BvBuffer * const pBuffer, const u32 drawCount = 1, const u64 offset = 0) = 0;
	virtual void DrawIndexedIndirect(const BvBuffer * const pBuffer, const u32 drawCount = 1, const u64 offset = 0) = 0;
	virtual void DispatchIndirect(const BvBuffer * const pBuffer, const u64 offset = 0) = 0;

	virtual void CopyBuffer(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer) = 0;
	virtual void CopyBufferRegion(const BvBuffer * const pSrcBuffer, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion) = 0;

	virtual void CopyTexture(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture) = 0;
	virtual void CopyTextureRegion(const BvTexture * const pSrcTexture, BvTexture * const pDstTexture, const CopyRegion & copyRegion) = 0;

	virtual void CopyTextureRegion(const BvBuffer * const pSrcBuffer, BvTexture * const pDstTexture, const CopyRegion & copyRegion) = 0;
	virtual void CopyTextureRegion(const BvTexture * const pSrcTexture, BvBuffer * const pDstBuffer, const CopyRegion & copyRegion) = 0;

	virtual void ResourceBarrier(const u32 barrierCount, const ResourceBarrierDesc * const pBarriers) = 0;

	QueueFamilyType GetQueueFamilyType() const { return m_pCommandPool->GetDesc().m_QueueFamilyType; }
	CommandType GetCommandLevel() const { return m_pCommandPool->GetDesc().m_CommandType; }

protected:
	BvCommandBuffer(BvCommandPool * pCommandPool)
		: m_pCommandPool(pCommandPool) {}
	virtual ~BvCommandBuffer() = 0 {}

protected:
	BvCommandPool * m_pCommandPool = nullptr;
	CurrentState m_CurrentState = CurrentState::kReset;
};