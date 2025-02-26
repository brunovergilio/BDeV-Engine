#pragma once


#include "BvRenderCommon.h"
#include "BvRenderDeviceObject.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvBuffer;


BV_OBJECT_DEFINE_ID(BvAccelerationStructure, "11ee119e-5ecb-4675-9b95-f13439e1a3aa");
class BvAccelerationStructure : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(BvAccelerationStructure);

public:
	u32 GetGeometryIndex(BvStringId id)
	{
		auto it = m_GeometryMap.FindKey(id);
		if (it != m_GeometryMap.cend())
		{
			return it->second;
		}

		return kU32Max;
	}

	BV_INLINE const RayTracingAccelerationStructureDesc& GetDesc() const { return m_Desc; }

	virtual void WriteTopLevelInstances(BvBuffer* pStagingBuffer, u32 instanceCount, const TLASBuildInstanceDesc* pInstances) = 0;
	virtual u64 GetTopLevelInstanceSize() const = 0;
	virtual BvBuffer* GetTopLevelStagingInstanceBuffer() const = 0;
	virtual RayTracingAccelerationStructureScratchSize GetBuildSizes() const = 0;
	
	BV_INLINE void WriteTopLevelInstances(u32 instanceCount, const TLASBuildInstanceDesc* pInstances)
	{
		WriteTopLevelInstances(nullptr, instanceCount, pInstances);
	}

protected:
	BvAccelerationStructure(const RayTracingAccelerationStructureDesc& desc)
		: m_Desc(desc)
	{
		if (m_Desc.m_Type == RayTracingAccelerationStructureType::kBottomLevel)
		{
			for (auto i = 0; i < m_Desc.m_BLAS.m_GeometryCount; ++i)
			{
				auto id = m_Desc.m_BLAS.m_pGeometries[i].m_Id;
				if (id != BvStringId::Empty())
				{
					m_GeometryMap.Emplace(id, i);
				}
			}
		}
	}
	~BvAccelerationStructure() {}

protected:
	RayTracingAccelerationStructureDesc m_Desc;
	BvRobinMap<BvStringId, u32> m_GeometryMap;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvAccelerationStructure);