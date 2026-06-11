#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/RenderAPI/BvAccelerationStructure.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvRenderDeviceD3D12;


class BvAccelerationStructureD3D12 final : public IBvAccelerationStructure, public IBvResourceD3D12
{
public:
	BvAccelerationStructureD3D12(BvRenderDeviceD3D12* pDevice, const RayTracingAccelerationStructureDesc& desc, ComPtr<ID3D12Resource>& buffer,
		ComPtr<D3D12MA::Allocation>& allocation, BvVector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometries, const RayTracingAccelerationStructureScratchSize& scratchSizes);
	~BvAccelerationStructureD3D12();

	BV_INLINE const RayTracingAccelerationStructureDesc& GetDesc() const override { return m_Desc; }
	u32 GetGeometryIndex(BvStringId id) const override;
	void WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const RayTracingAccelerationStructureInstanceDesc* pInstances, u32 firstInstance = 0) override;
	BV_INLINE u64 GetTopLevelInstanceSize() const override { return sizeof(D3D12_RAYTRACING_INSTANCE_DESC); }
	BV_INLINE RayTracingAccelerationStructureScratchSize GetBuildSizes() const override { return m_ScratchSizes; }
	BV_INLINE u64 GetDeviceAddress() const override { return m_Buffer->GetGPUVirtualAddress(); }
	BV_INLINE BvVector<D3D12_RAYTRACING_GEOMETRY_DESC>& GetGeometries() { return m_Geometries; }
	BV_INLINE ID3D12Resource* GetBuffer() const { return m_Buffer.Get(); }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ComPtr<ID3D12Resource> m_Buffer;
	ComPtr<D3D12MA::Allocation> m_Allocation;
	BvVector<D3D12_RAYTRACING_GEOMETRY_DESC> m_Geometries;
	RayTracingAccelerationStructureScratchSize m_ScratchSizes;
	RayTracingAccelerationStructureDesc m_Desc;
	BvRobinMap<BvStringId, u32> m_GeometryMap;
};
BV_OBJECT_DEFINE_ID(BvAccelerationStructureD3D12, "0A2136AA-D56A-40F1-ACBD-781CC416FBB6");
BV_CREATE_CAST_TO_D3D12(BvAccelerationStructure)