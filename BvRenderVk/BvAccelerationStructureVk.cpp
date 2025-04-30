#include "BvAccelerationStructureVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"


BV_VK_DEVICE_RES_DEF(BvAccelerationStructureVk)


BvAccelerationStructureVk::BvAccelerationStructureVk(BvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& desc)
	: m_Desc(desc), m_pDevice(pDevice)
{
	if (m_Desc.m_Type == RayTracingAccelerationStructureType::kBottomLevel)
	{
		auto pGeometries = BV_NEW_ARRAY(BLASGeometryDesc, m_Desc.m_BLAS.m_GeometryCount);
		memcpy(pGeometries, m_Desc.m_BLAS.m_pGeometries, sizeof(BLASGeometryDesc) * m_Desc.m_BLAS.m_GeometryCount);
		m_Desc.m_BLAS.m_pGeometries = pGeometries;

		for (auto i = 0; i < m_Desc.m_BLAS.m_GeometryCount; ++i)
		{
			auto id = m_Desc.m_BLAS.m_pGeometries[i].m_Id;
			if (id != BvStringId::Empty())
			{
				m_GeometryMap.Emplace(id, i);
			}
		}
	}

	Create();
}


BvAccelerationStructureVk::~BvAccelerationStructureVk()
{
	Destroy();

	if (m_Desc.m_Type == RayTracingAccelerationStructureType::kBottomLevel)
	{
		if (auto pGeometries = m_Desc.m_BLAS.m_pGeometries)
		{
			BV_DELETE_ARRAY(m_Desc.m_BLAS.m_pGeometries);
		}
	}
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


void BvAccelerationStructureVk::WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const TLASBuildInstanceDesc* pInstances, u32 firstInstance = 0)
{
	BV_ASSERT(m_Desc.m_Type == RayTracingAccelerationStructureType::kTopLevel, "Acceleration structure is not a top level one");
	if (!pStagingBuffer)
	{
		if (!m_StagingBuffer)
		{
			BufferDesc bufferDesc;
			bufferDesc.m_CreateFlags = BufferCreateFlags::kCreateMapped;
			bufferDesc.m_MemoryType = MemoryType::kUpload;
			bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
			bufferDesc.m_Size = std::max(m_PrimitiveCounts[0] * sizeof(VkAccelerationStructureInstanceKHR), sizeof(VkAccelerationStructureInstanceKHR));
			m_StagingBuffer = BvRCRaw(BV_NEW(BvBufferVk)(m_pDevice, bufferDesc, nullptr));
		}
		pStagingBuffer = m_StagingBuffer;
	}

	auto pDst = reinterpret_cast<u8>(pStagingBuffer->Map()) + sizeof(VkAccelerationStructureInstanceKHR) * firstInstance;
	for (auto i = 0; i < instanceCount && i < m_PrimitiveCounts[0]; ++i)
	{
		const TLASBuildInstanceDesc& srcInstance = pInstances[i];
		VkAccelerationStructureInstanceKHR& dstInstance = reinterpret_cast<VkAccelerationStructureInstanceKHR*>(pDst)[i];

		dstInstance.accelerationStructureReference = TO_VK(srcInstance.m_pBLAS)->GetDeviceAddress();
		dstInstance.instanceCustomIndex = srcInstance.m_InstanceId;
		dstInstance.mask = srcInstance.m_InstanceMask;
		dstInstance.instanceShaderBindingTableRecordOffset = srcInstance.m_ShaderBindingTableIndex;
		dstInstance.flags = GetVkGeometryInstanceFlags(srcInstance.m_Flags);
		memcpy(dstInstance.transform.matrix, srcInstance.m_Transform.m, sizeof(VkTransformMatrixKHR));
	}
	pStagingBuffer->Flush();
}


IBvBuffer* BvAccelerationStructureVk::GetTopLevelStagingInstanceBuffer() const
{
	return m_StagingBuffer;
}


//VkDeviceAddress BvAccelerationStructureVk::GetDeviceAddress() const
//{
//	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, nullptr, m_Handle };
//	return vkGetAccelerationStructureDeviceAddressKHR(m_pDevice->GetHandle(), &addressInfo);
//}


void BvAccelerationStructureVk::Create()
{
	VkAccelerationStructureTypeKHR asType{};
	if (m_Desc.m_Type == RayTracingAccelerationStructureType::kBottomLevel)
	{
		asType = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

		m_Geometries.Resize(m_Desc.m_BLAS.m_GeometryCount, { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR });
		m_PrimitiveCounts.Resize(m_Geometries.Size());

		for (auto i = 0u; i < m_Desc.m_BLAS.m_GeometryCount; ++i)
		{
			if (m_Desc.m_BLAS.m_pGeometries[i].m_Type == RayTracingGeometryType::kTriangles)
			{
				auto& srcGeometry = m_Desc.m_BLAS.m_pGeometries[i].m_Triangle;

				VkAccelerationStructureGeometryKHR& dstGeometry = m_Geometries[i];
				//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				//dstGeometry.pNext = nullptr;
				dstGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				dstGeometry.flags = GetVkGeometryFlags(m_Desc.m_BLAS.m_pGeometries[i].m_Flags);

				VkAccelerationStructureGeometryTrianglesDataKHR& triangle = dstGeometry.geometry.triangles;
				triangle.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				triangle.pNext = nullptr;
				triangle.vertexFormat = GetVkFormat(srcGeometry.m_VertexFormat);
				triangle.vertexStride = srcGeometry.m_VertexStride;
				triangle.maxVertex = srcGeometry.m_VertexCount - 1;
				triangle.indexType = GetVkIndexType(srcGeometry.m_IndexFormat);

				u32 primitiveCount = srcGeometry.m_IndexCount > 0 ? srcGeometry.m_IndexCount / 3 : srcGeometry.m_VertexCount / 3;
				m_PrimitiveCounts[i] = primitiveCount;
			}
			else if (m_Desc.m_BLAS.m_pGeometries[i].m_Type == RayTracingGeometryType::kAABB)
			{
				auto& srcGeometry = m_Desc.m_BLAS.m_pGeometries[i].m_AABB;

				VkAccelerationStructureGeometryKHR& dstGeometry = m_Geometries[i];
				//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				//dstGeometry.pNext = nullptr;
				dstGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
				dstGeometry.flags = GetVkGeometryFlags(m_Desc.m_BLAS.m_pGeometries[i].m_Flags);

				VkAccelerationStructureGeometryAabbsDataKHR& aabb = dstGeometry.geometry.aabbs;
				aabb.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
				aabb.pNext = nullptr;
				aabb.stride = srcGeometry.m_Stride;

				m_PrimitiveCounts[i] = srcGeometry.m_Count;
			}
		}
	}
	else if (m_Desc.m_Type == RayTracingAccelerationStructureType::kTopLevel)
	{
		asType = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

		m_Geometries.Resize(1, { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR });
		m_PrimitiveCounts.Resize(1, m_Desc.m_TLAS.m_InstanceCount);

		VkAccelerationStructureGeometryKHR &dstGeometry = m_Geometries.Back();
		//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		//dstGeometry.pNext = nullptr;
		dstGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		dstGeometry.flags = GetVkGeometryFlags(m_Desc.m_TLAS.m_Flags);

		VkAccelerationStructureGeometryInstancesDataKHR& instances = dstGeometry.geometry.instances;
		instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		instances.arrayOfPointers = VK_FALSE;
	}

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	//buildInfo.pNext = nullptr;
	buildInfo.type = asType;
	buildInfo.flags = GetVkBuildAccelerationStructureFlags(m_Desc.m_Flags);
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.geometryCount = u32(m_Geometries.Size());
	buildInfo.pGeometries = m_Geometries.Data();

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };

	GetBuildSizes(buildInfo, m_PrimitiveCounts.Data(), sizeInfo);

	m_ScratchSizes.m_Build = sizeInfo.buildScratchSize;
	m_ScratchSizes.m_Update = sizeInfo.updateScratchSize;

	BufferDesc bufferDesc;
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	bufferDesc.m_MemoryType = MemoryType::kDevice;
	bufferDesc.m_Size = sizeInfo.accelerationStructureSize;
	m_Buffer = BvRCRaw(BV_NEW(BvBufferVk)(m_pDevice, bufferDesc, nullptr));

	auto device = m_pDevice->GetHandle();

	VkAccelerationStructureCreateInfoKHR accelerationStructureCI{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	//accelerationStructureCI.createFlags = 0;
	accelerationStructureCI.buffer = m_Buffer->GetHandle();
	//accelerationStructureCI.offset = 0;
	accelerationStructureCI.size = sizeInfo.accelerationStructureSize;
	accelerationStructureCI.type = asType;
	if (vkCreateAccelerationStructureKHR(device, &accelerationStructureCI, nullptr, &m_Handle) != VK_SUCCESS)
	{
		Destroy();
		return;
	}

	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, nullptr, m_Handle };
	m_DeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
}


void BvAccelerationStructureVk::Destroy()
{
	auto device = m_pDevice->GetHandle();
	if (m_Handle)
	{
		vkDestroyAccelerationStructureKHR(device, m_Handle, nullptr);
		m_Handle = nullptr;
		
		m_Buffer.Reset();
		m_StagingBuffer.Reset();
	}
}


void BvAccelerationStructureVk::GetBuildSizes(const VkAccelerationStructureBuildGeometryInfoKHR& buildInfo, const u32* pPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR& sizeInfo)
{
	vkGetAccelerationStructureBuildSizesKHR(m_pDevice->GetHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, pPrimitiveCounts, &sizeInfo);
}