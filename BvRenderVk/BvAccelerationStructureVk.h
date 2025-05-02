#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvAccelerationStructure.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BvBufferVk.h"


class BvRenderDeviceVk;


//BV_OBJECT_DEFINE_ID(IBvAccelerationStructureVk, "70617509-5657-4c47-9a4e-dc318b535076");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvAccelerationStructureVk);


class BvAccelerationStructureVk final : public IBvAccelerationStructure, public IBvResourceVk
{
	BV_NOCOPYMOVE(BvAccelerationStructureVk);
	BV_VK_DEVICE_RES_DECL;

public:
	BvAccelerationStructureVk(BvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& desc);
	~BvAccelerationStructureVk();

	BV_INLINE const RayTracingAccelerationStructureDesc& GetDesc() const override { return m_Desc; }
	u32 GetGeometryIndex(BvStringId id) const override;
	void WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const TLASInstanceDesc* pInstances, u32 firstInstance = 0) override;
	BV_INLINE IBvBuffer* GetBuffer() const override { return m_Buffer; }
	BV_INLINE u64 GetSize() const override { return m_Buffer->GetDesc().m_Size; }
	BV_INLINE u64 GetTopLevelInstanceSize() const override { return sizeof(VkAccelerationStructureInstanceKHR); }
	IBvBuffer* GetTopLevelStagingInstanceBuffer() const override;
	BV_INLINE RayTracingAccelerationStructureScratchSize GetBuildSizes() const override { return m_ScratchSizes; }

	BV_INLINE BvVector<VkAccelerationStructureGeometryKHR>& GetGeometries() { return m_Geometries; }
	BV_INLINE BvVector<u32>& GetPrimitiveCounts() { return m_PrimitiveCounts; }
	BV_INLINE VkAccelerationStructureKHR GetHandle() const { return m_Handle; }
	BV_INLINE VkDeviceAddress GetDeviceAddress() const { return m_DeviceAddress; }
	BV_INLINE bool IsValid() const { return m_Handle != VK_NULL_HANDLE; }

	//BV_OBJECT_IMPL_INTERFACE(IBvAccelerationStructureVk, IBvAccelerationStructure, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

	void GetBuildSizes(const VkAccelerationStructureBuildGeometryInfoKHR& buildInfo, const u32* pPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR& sizeInfo);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkAccelerationStructureKHR m_Handle = VK_NULL_HANDLE;
	VkDeviceAddress m_DeviceAddress = 0;
	BvRCRef<BvBufferVk> m_Buffer = nullptr;
	BvRCRef<BvBufferVk> m_StagingBuffer = nullptr;
	RayTracingAccelerationStructureScratchSize m_ScratchSizes;
	BvVector<VkAccelerationStructureGeometryKHR> m_Geometries;
	BvVector<u32> m_PrimitiveCounts;
	RayTracingAccelerationStructureDesc m_Desc;
	BvRobinMap<BvStringId, u32> m_GeometryMap;
};


BV_CREATE_CAST_TO_VK(BvAccelerationStructure)