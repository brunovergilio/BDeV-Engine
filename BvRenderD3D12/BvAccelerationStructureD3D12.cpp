#include "BvAccelerationStructureD3D12.h"
#include "BvBufferD3D12.h"


BvAccelerationStructureD3D12::BvAccelerationStructureD3D12(BvRenderDeviceD3D12* pDevice, const RayTracingAccelerationStructureDesc& desc, ComPtr<ID3D12Resource>& buffer,
	ComPtr<D3D12MA::Allocation>& allocation, BvVector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometries, const RayTracingAccelerationStructureScratchSize& scratchSizes)
	: m_pDevice(pDevice), m_Desc(desc), m_Buffer(std::move(buffer)), m_Allocation(std::move(allocation)), m_Geometries(std::move(geometries)), m_ScratchSizes(scratchSizes)
{
	if (m_Desc.m_Type == RayTracingAccelerationStructureType::kBottomLevel)
	{
		for (auto i = 0; i < m_Desc.m_Geometries.Size(); ++i)
		{
			auto id = m_Desc.m_Geometries[i].m_Id;
			if (id != BvStringId::Empty())
			{
				m_GeometryMap.Emplace(id, i);
			}
		}
	}
}


BvAccelerationStructureD3D12::~BvAccelerationStructureD3D12()
{
	Destroy();
}


u32 BvAccelerationStructureD3D12::GetGeometryIndex(BvStringId id) const
{
	auto it = m_GeometryMap.FindKey(id);
	if (it != m_GeometryMap.cend())
	{
		return it->second;
	}

	return kU32Max;
}


void BvAccelerationStructureD3D12::WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const RayTracingAccelerationStructureInstanceDesc* pInstances, u32 firstInstance /*= 0*/)
{
	BV_ASSERT(m_Desc.m_Type == RayTracingAccelerationStructureType::kTopLevel, "Acceleration structure is not a top level one");
	BV_ASSERT(m_Desc.m_CompactedSize == 0, "Acceleration structure can't be compact");

	auto pBuffer = TO_D3D12(pStagingBuffer);
	if (!pBuffer)
	{
		return;
	}

	auto pDst = reinterpret_cast<u8*>(pBuffer->Map(kU64Max, 0)) + sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * firstInstance;
	for (auto i = 0; i < instanceCount && i < m_Desc.m_Geometries[0].m_Instance.m_InstanceCount; ++i)
	{
		const RayTracingAccelerationStructureInstanceDesc& srcInstance = pInstances[i];
		D3D12_RAYTRACING_INSTANCE_DESC* pDstInstance = reinterpret_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(pDst) + i;
		memcpy(pDstInstance, &srcInstance, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	}
}


void BvAccelerationStructureD3D12::Destroy()
{
	m_Buffer.Reset();
}