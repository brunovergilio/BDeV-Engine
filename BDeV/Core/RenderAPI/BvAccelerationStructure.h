#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class IBvBuffer;


BV_OBJECT_DEFINE_ID(IBvAccelerationStructure, "11ee119e-5ecb-4675-9b95-f13439e1a3aa");
class IBvAccelerationStructure : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(IBvAccelerationStructure);

public:
	virtual const RayTracingAccelerationStructureDesc& GetDesc() const = 0;
	virtual u32 GetGeometryIndex(BvStringId id) const = 0;
	virtual void WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const TLASBuildInstanceDesc* pInstances) = 0;
	virtual u64 GetTopLevelInstanceSize() const = 0;
	virtual IBvBuffer* GetTopLevelStagingInstanceBuffer() const = 0;
	virtual RayTracingAccelerationStructureScratchSize GetBuildSizes() const = 0;
	
	BV_INLINE void WriteTopLevelInstances(u32 instanceCount, const TLASBuildInstanceDesc* pInstances)
	{
		WriteTopLevelInstances(nullptr, instanceCount, pInstances);
	}

protected:
	IBvAccelerationStructure() {}
	~IBvAccelerationStructure() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvAccelerationStructure);