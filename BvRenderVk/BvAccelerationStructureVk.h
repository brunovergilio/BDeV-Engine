#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvAccelerationStructure.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class IBvRenderDeviceVk;
class IBvBufferVk;


BV_OBJECT_DEFINE_ID(IBvAccelerationStructureVk, "70617509-5657-4c47-9a4e-dc318b535076");
class IBvAccelerationStructureVk : public IBvAccelerationStructure
{
	BV_NOCOPYMOVE(IBvAccelerationStructureVk);

public:
	virtual BvVector<VkAccelerationStructureGeometryKHR>& GetGeometries() = 0;
	virtual BvVector<u32>& GetPrimitiveCounts() = 0;
	virtual VkAccelerationStructureKHR GetHandle() const = 0;
	virtual IBvBufferVk* GetBuffer() const = 0;
	virtual VkDeviceAddress GetDeviceAddress() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvAccelerationStructureVk() {}
	~IBvAccelerationStructureVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvAccelerationStructureVk);


class BvAccelerationStructureVk final : public IBvAccelerationStructureVk
{
	BV_NOCOPYMOVE(BvAccelerationStructureVk);

public:
	BvAccelerationStructureVk(IBvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& desc);
	~BvAccelerationStructureVk();

	IBvRenderDevice* GetDevice() override;

	BV_INLINE const RayTracingAccelerationStructureDesc& GetDesc() const override { return m_Desc; }
	u32 GetGeometryIndex(BvStringId id) const override;
	void WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const TLASBuildInstanceDesc* pInstances) override;
	BV_INLINE u64 GetTopLevelInstanceSize() const override { return sizeof(VkAccelerationStructureInstanceKHR); }
	IBvBuffer* GetTopLevelStagingInstanceBuffer() const override;
	BV_INLINE RayTracingAccelerationStructureScratchSize GetBuildSizes() const override { return m_ScratchSizes; }

	BV_INLINE BvVector<VkAccelerationStructureGeometryKHR>& GetGeometries() override { return m_Geometries; }
	BV_INLINE BvVector<u32>& GetPrimitiveCounts() override { return m_PrimitiveCounts; }
	BV_INLINE VkAccelerationStructureKHR GetHandle() const override { return m_Handle; }
	BV_INLINE IBvBufferVk* GetBuffer() const override { return m_pBuffer; }
	BV_INLINE VkDeviceAddress GetDeviceAddress() const override { return m_DeviceAddress; }
	BV_INLINE bool IsValid() const override { return m_Handle != VK_NULL_HANDLE; }

	BV_OBJECT_IMPL_INTERFACE(IBvAccelerationStructureVk, IBvAccelerationStructure, IBvRenderDeviceObject);

private:
	void Create();
	void Destroy();

	void GetBuildSizes(const VkAccelerationStructureBuildGeometryInfoKHR& buildInfo, const u32* pPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR& sizeInfo);

private:
	IBvRenderDeviceVk* m_pDevice = nullptr;
	VkAccelerationStructureKHR m_Handle = VK_NULL_HANDLE;
	VkDeviceAddress m_DeviceAddress = 0;
	IBvBufferVk* m_pBuffer = nullptr;
	IBvBufferVk* m_pStagingBuffer = nullptr;
	RayTracingAccelerationStructureScratchSize m_ScratchSizes;
	BvVector<VkAccelerationStructureGeometryKHR> m_Geometries;
	BvVector<u32> m_PrimitiveCounts;
	RayTracingAccelerationStructureDesc m_Desc;
	BvRobinMap<BvStringId, u32> m_GeometryMap;
};


BV_CREATE_CAST_TO_VK(IBvAccelerationStructure)