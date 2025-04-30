#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BDeV/Core/Utils/BvObject.h"


class IBvBuffer;


//BV_OBJECT_DEFINE_ID(IBvAccelerationStructure, "11ee119e-5ecb-4675-9b95-f13439e1a3aa");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvAccelerationStructure);
class IBvAccelerationStructure : public BvRCObj
{
	BV_NOCOPYMOVE(IBvAccelerationStructure);

public:
	virtual const RayTracingAccelerationStructureDesc& GetDesc() const = 0;
	virtual u32 GetGeometryIndex(BvStringId id) const = 0;
	virtual void WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const TLASBuildInstanceDesc* pInstances, u32 firstInstance = 0) = 0;
	virtual u64 GetTopLevelInstanceSize() const = 0;
	virtual IBvBuffer* GetTopLevelStagingInstanceBuffer() const = 0;
	virtual RayTracingAccelerationStructureScratchSize GetBuildSizes() const = 0;
	virtual u64 GetRayTracingInstanceSize() const = 0;
	
	BV_INLINE void WriteTopLevelInstances(u32 instanceCount, const TLASBuildInstanceDesc* pInstances, u32 firstInstance = 0)
	{
		WriteTopLevelInstances(nullptr, instanceCount, pInstances, firstInstance);
	}

protected:
	IBvAccelerationStructure() {}
	~IBvAccelerationStructure() {}
};