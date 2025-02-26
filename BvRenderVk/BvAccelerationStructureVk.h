#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvAccelerationStructure.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvRenderDeviceVk;
class BvBufferVk;


BV_OBJECT_DEFINE_ID(BvAccelerationStructureVk, "70617509-5657-4c47-9a4e-dc318b535076");
class BvAccelerationStructureVk : public BvAccelerationStructure
{
	BV_NOCOPYMOVE(BvAccelerationStructureVk);

public:
	BvAccelerationStructureVk(BvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& desc);
	~BvAccelerationStructureVk();

	BvRenderDevice* GetDevice() override;
	void WriteTopLevelInstances(BvBuffer* pStagingBuffer, u32 instanceCount, const TLASBuildInstanceDesc* pInstances) override;
	BV_INLINE u64 GetTopLevelInstanceSize() const override { return sizeof(VkAccelerationStructureInstanceKHR); }
	BvBuffer* GetTopLevelStagingInstanceBuffer() const override;

	BV_INLINE RayTracingAccelerationStructureScratchSize GetBuildSizes() const override	{ return m_ScratchSizes; }
	BV_INLINE BvVector<VkAccelerationStructureGeometryKHR>& GetGeometries() { return m_Geometries; }
	BV_INLINE BvVector<u32>& GetPrimitiveCounts() { return m_PrimitiveCounts; }

	BV_INLINE VkAccelerationStructureKHR GetHandle() const { return m_Handle; }
	BV_INLINE BvBufferVk* GetBuffer() const { return m_pBuffer; }
	BV_INLINE VkDeviceAddress GetDeviceAddress() const { return m_DeviceAddress; }
	BV_INLINE bool IsValid() const { return m_Handle != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(BvAccelerationStructureVk, BvAccelerationStructure, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

	void GetBuildSizes(const VkAccelerationStructureBuildGeometryInfoKHR& buildInfo, const u32* pPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR& sizeInfo);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkAccelerationStructureKHR m_Handle = VK_NULL_HANDLE;
	VkDeviceAddress m_DeviceAddress = 0;
	BvBufferVk* m_pBuffer = nullptr;
	BvBufferVk* m_pStagingBuffer = nullptr;
	RayTracingAccelerationStructureScratchSize m_ScratchSizes;
	BvVector<VkAccelerationStructureGeometryKHR> m_Geometries;
	BvVector<u32> m_PrimitiveCounts;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvAccelerationStructureVk);


BV_CREATE_CAST_TO_VK(BvAccelerationStructure)