#pragma once


#include "BvRenderCommon.h"



BV_OBJECT_DEFINE_ID(IBvShaderResourceLayout, "79547e0c-dc23-4354-9a51-c1af70d20b83");
class IBvShaderResourceLayout : public BvRCObj
{
	BV_NOCOPYMOVE(IBvShaderResourceLayout);

public:
	virtual const ShaderResourceLayoutDesc& GetDesc() const = 0;

protected:
	IBvShaderResourceLayout() {}
	~IBvShaderResourceLayout() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderResourceLayout);


class IBvShaderResourceParams
{
	BV_NOCOPYMOVE(IBvShaderResourceParams);

public:
	virtual void SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 binding, u32 startIndex = 0) = 0;

	BV_INLINE void SetConstantBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetConstantBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetStructuredBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetStructuredBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetRWStructuredBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetRWStructuredBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetFormattedBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetFormattedBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetRWFormattedBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetRWFormattedBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetTexture(const IBvTextureView* pResource, u32 binding, u32 startIndex = 0) { SetTextures(1, &pResource, binding, startIndex); }
	BV_INLINE void SetRWTexture(const IBvTextureView* pResource, u32 binding, u32 startIndex = 0) { SetRWTextures(1, &pResource, binding, startIndex); }
	BV_INLINE void SetSampler(const IBvSampler* pResource, u32 binding, u32 startIndex = 0) { SetSamplers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetAccelerationStructure(const IBvAccelerationStructure* pResource, u32 binding, u32 startIndex = 0) { SetAccelerationStructures(1, &pResource, binding, startIndex); }
	
	virtual void Bind() = 0;

protected:
	IBvShaderResourceParams() {}
	~IBvShaderResourceParams() {}
};