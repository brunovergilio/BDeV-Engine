#include "BvAccelerationStructureVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"
#include "BvBufferVk.h"


BvAccelerationStructureVk::BvAccelerationStructureVk(BvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& desc,
	VkAccelerationStructureKHR as, VkDeviceAddress deviceAddress, BvVector<VkAccelerationStructureGeometryKHR>& geometries,
	BvVector<VkAccelerationStructureBuildRangeInfoKHR>& ranges, VkBuffer buffer, VmaAllocation allocation,
	VkDeviceAddress bufferDeviceAddress, const RayTracingAccelerationStructureScratchSize& scratchSizes)
	: m_Desc(desc), m_pDevice(pDevice), m_Handle(as), m_DeviceAddress(deviceAddress), m_Geometries(std::move(geometries)),
	m_Ranges(std::move(ranges)), m_Buffer(buffer), m_Allocation(allocation),
	m_BufferDeviceAddress(bufferDeviceAddress), m_ScratchSizes(scratchSizes)
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


BvAccelerationStructureVk::~BvAccelerationStructureVk()
{
	Destroy();
}


u32 BvAccelerationStructureVk::GetGeometryIndex(BvStringId id) const
{
	auto it = m_GeometryMap.FindKey(id);
	if (it != m_GeometryMap.cend())
	{
		return it->second;
	}

	return kU32Max;
}


void BvAccelerationStructureVk::WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const RayTracingAccelerationStructureInstanceDesc* pInstances, u32 firstInstance)
{
	BV_ASSERT(m_Desc.m_Type == RayTracingAccelerationStructureType::kTopLevel, "Acceleration structure is not a top level one");
	BV_ASSERT(m_Desc.m_CompactedSize == 0, "Acceleration structure can't be compact");

	auto pVkBuffer = TO_VK(pStagingBuffer);
	if (!pVkBuffer)
	{
		return;
	}

	auto pDst = reinterpret_cast<u8*>(pVkBuffer->Map()) + sizeof(VkAccelerationStructureInstanceKHR) * firstInstance;
	for (auto i = 0; i < instanceCount && i < m_Ranges[0].primitiveCount; ++i)
	{
		const RayTracingAccelerationStructureInstanceDesc& srcInstance = pInstances[i];
		VkAccelerationStructureInstanceKHR* pDstInstance = reinterpret_cast<VkAccelerationStructureInstanceKHR*>(pDst) + i;
		memcpy(pDstInstance, &srcInstance, sizeof(VkAccelerationStructureInstanceKHR));
	}
	pVkBuffer->Flush();
}


void BvAccelerationStructureVk::Destroy()
{
	if (m_Handle)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Handle);
		m_pDevice->OnVkHandleDestroyed(u64(m_Handle), false);
		m_Handle = VK_NULL_HANDLE;
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Buffer, m_pDevice->GetAllocator(), m_Allocation);
	}
}