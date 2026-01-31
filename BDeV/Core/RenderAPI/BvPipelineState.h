#pragma once


#include "BvRenderCommon.h"


class IBvGraphicsPipelineState : public BvRCObj
{
	BV_NOCOPYMOVE(IBvGraphicsPipelineState);

public:
	virtual const GraphicsPipelineStateDesc& GetDesc() const = 0;

protected:
	IBvGraphicsPipelineState() {}
	~IBvGraphicsPipelineState() {}
};
BV_OBJECT_DEFINE_ID(IBvGraphicsPipelineState, "863fd051-1ea0-4913-bd1c-2dd02f69589f");


class IBvComputePipelineState : public BvRCObj
{
	BV_NOCOPYMOVE(IBvComputePipelineState);

public:
	virtual const ComputePipelineStateDesc& GetDesc() const = 0;

protected:
	IBvComputePipelineState() {}
	~IBvComputePipelineState() {}
};
BV_OBJECT_DEFINE_ID(IBvComputePipelineState, "3d4b6d6a-9cb4-42ff-bc14-ec6a328b2801");


class IBvRayTracingPipelineState : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRayTracingPipelineState);

public:
	virtual const RayTracingPipelineStateDesc& GetDesc() const = 0;

protected:
	IBvRayTracingPipelineState() {}
	~IBvRayTracingPipelineState() {}
};
BV_OBJECT_DEFINE_ID(IBvRayTracingPipelineState, "e04f4570-bb75-4ff2-b9e2-02ba4affb9de");


class IBvPipelineCache : public BvRCObj
{
	BV_NOCOPYMOVE(IBvPipelineCache);

public:
	virtual void GetCachedData(u64& size, void* pData) = 0;

protected:
	IBvPipelineCache() {}
	~IBvPipelineCache() {}
};
BV_OBJECT_DEFINE_ID(IBvPipelineCache, "A777C429-8E00-4763-BA44-1A255733965E");