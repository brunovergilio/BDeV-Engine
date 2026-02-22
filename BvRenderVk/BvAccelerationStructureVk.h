#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvAccelerationStructure.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvRenderDeviceVk;


class BvAccelerationStructureVk final : public IBvAccelerationStructure, public IBvResourceVk
{
public:
	BvAccelerationStructureVk(BvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& desc,
		VkAccelerationStructureKHR as, VkDeviceAddress deviceAddress, BvVector<VkAccelerationStructureGeometryKHR>& geometries,
		BvVector<VkAccelerationStructureBuildRangeInfoKHR>& ranges, VkBuffer buffer, VmaAllocation allocation,
		VkDeviceAddress bufferDeviceAddress, const RayTracingAccelerationStructureScratchSize& scratchSizes);
	~BvAccelerationStructureVk();

	BV_INLINE const RayTracingAccelerationStructureDesc& GetDesc() const override { return m_Desc; }
	u32 GetGeometryIndex(BvStringId id) const override;
	void WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const RayTracingAccelerationStructureInstanceDesc* pInstances, u32 firstInstance = 0) override;
	BV_INLINE u64 GetTopLevelInstanceSize() const override { return sizeof(VkAccelerationStructureInstanceKHR); }
	BV_INLINE RayTracingAccelerationStructureScratchSize GetBuildSizes() const override { return m_ScratchSizes; }
	BV_INLINE u64 GetDeviceAddress() const override { return m_DeviceAddress; }
	BV_INLINE VkBuffer GetBuffer() const { return m_Buffer; }

	BV_INLINE BvVector<VkAccelerationStructureGeometryKHR>& GetGeometries() { return m_Geometries; }
	BV_INLINE BvVector<VkAccelerationStructureBuildRangeInfoKHR>& GetRanges() { return m_Ranges; }
	BV_INLINE VkAccelerationStructureKHR GetHandle() const { return m_Handle; }
	BV_INLINE bool IsValid() const { return m_Handle != VK_NULL_HANDLE; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkAccelerationStructureKHR m_Handle = VK_NULL_HANDLE;
	VkDeviceAddress m_DeviceAddress = 0;
	VkBuffer m_Buffer = VK_NULL_HANDLE;
	VmaAllocation m_Allocation = VK_NULL_HANDLE;
	VkDeviceAddress m_BufferDeviceAddress = 0;
	RayTracingAccelerationStructureScratchSize m_ScratchSizes;
	BvVector<VkAccelerationStructureGeometryKHR> m_Geometries;
	BvVector<VkAccelerationStructureBuildRangeInfoKHR> m_Ranges;
	RayTracingAccelerationStructureDesc m_Desc;
	BvRobinMap<BvStringId, u32> m_GeometryMap;
};
BV_OBJECT_DEFINE_ID(BvAccelerationStructureVk, "70617509-5657-4c47-9a4e-dc318b535076");
BV_CREATE_CAST_TO_VK(BvAccelerationStructure)