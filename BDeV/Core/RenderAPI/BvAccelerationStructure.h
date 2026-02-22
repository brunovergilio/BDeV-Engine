#pragma once


#include "BvRenderCommon.h"


class IBvAccelerationStructure : public BvRCObj
{
public:
	virtual const RayTracingAccelerationStructureDesc& GetDesc() const = 0;
	virtual u32 GetGeometryIndex(BvStringId id) const = 0;
	virtual void WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const RayTracingAccelerationStructureInstanceDesc* pInstances, u32 firstInstance = 0) = 0;
	virtual u64 GetTopLevelInstanceSize() const = 0;
	virtual RayTracingAccelerationStructureScratchSize GetBuildSizes() const = 0;
	virtual u64 GetDeviceAddress() const = 0;

protected:
	IBvAccelerationStructure() {}
	~IBvAccelerationStructure() {}
};
BV_OBJECT_DEFINE_ID(IBvAccelerationStructure, "11ee119e-5ecb-4675-9b95-f13439e1a3aa");