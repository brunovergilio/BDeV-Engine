#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvFixedVector.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/Utils/BvObject.h"


class IBvShaderResourceLayout;
class IBvShader;
class IBvRenderPass;


//BV_OBJECT_DEFINE_ID(IBvGraphicsPipelineState, "863fd051-1ea0-4913-bd1c-2dd02f69589f");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvGraphicsPipelineState);
class IBvGraphicsPipelineState : public BvRCObj
{
	BV_NOCOPYMOVE(IBvGraphicsPipelineState);

public:
	virtual const GraphicsPipelineStateDesc& GetDesc() const = 0;

protected:
	IBvGraphicsPipelineState() {}
	~IBvGraphicsPipelineState() {}
};


//BV_OBJECT_DEFINE_ID(IBvComputePipelineState, "3d4b6d6a-9cb4-42ff-bc14-ec6a328b2801");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvComputePipelineState);
class IBvComputePipelineState : public BvRCObj
{
	BV_NOCOPYMOVE(IBvComputePipelineState);

public:
	virtual const ComputePipelineStateDesc& GetDesc() const = 0;

protected:
	IBvComputePipelineState() {}
	~IBvComputePipelineState() {}
};


//BV_OBJECT_DEFINE_ID(IBvRayTracingPipelineState, "e04f4570-bb75-4ff2-b9e2-02ba4affb9de");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvRayTracingPipelineState);
class IBvRayTracingPipelineState : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRayTracingPipelineState);

public:
	virtual const RayTracingPipelineStateDesc& GetDesc() const = 0;

protected:
	IBvRayTracingPipelineState() {}
	~IBvRayTracingPipelineState() {}
};