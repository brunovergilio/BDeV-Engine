#include "BvAccelerationStructureVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvBufferVk.h"


BvAccelerationStructureVk::BvAccelerationStructureVk(IBvRenderDeviceVk* pDevice, const RayTracingAccelerationStructureDesc& desc)
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


IBvRenderDevice* BvAccelerationStructureVk::GetDevice()
{
	return m_pDevice;
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


void BvAccelerationStructureVk::WriteTopLevelInstances(IBvBuffer* pStagingBuffer, u32 instanceCount, const TLASBuildInstanceDesc* pInstances)
{
	BV_ASSERT(m_Desc.m_Type == RayTracingAccelerationStructureType::kTopLevel, "Acceleration structure is not a top level one");
	if (!pStagingBuffer)
	{
		if (!m_pStagingBuffer)
		{
			BufferDesc buff;
			buff.m_CreateFlags = BufferCreateFlags::kCreateMapped;
			buff.m_MemoryType = MemoryType::kUpload;
			buff.m_Size = m_PrimitiveCounts[0] * sizeof(VkAccelerationStructureInstanceKHR);
		}
		pStagingBuffer = m_pStagingBuffer;
	}

	auto pDst = pStagingBuffer->Map();
	for (auto i = 0; i < instanceCount && i < m_PrimitiveCounts[0]; ++i)
	{
		auto& srcInstance = pInstances[i];
		auto& dstInstance = reinterpret_cast<VkAccelerationStructureInstanceKHR*>(pDst)[i];
		dstInstance.accelerationStructureReference = TO_VK(srcInstance.m_pBLAS)->GetDeviceAddress();
		dstInstance.instanceCustomIndex = srcInstance.m_InstanceId;
		dstInstance.mask = srcInstance.m_InstanceMask;
		dstInstance.instanceShaderBindingTableRecordOffset = srcInstance.m_ShaderBindingTableIndex;
		dstInstance.flags = GetVkGeometryInstanceFlags(srcInstance.m_Flags);
		memcpy(dstInstance.transform.matrix, srcInstance.m_Transform, sizeof(VkTransformMatrixKHR));
	}
}


IBvBuffer* BvAccelerationStructureVk::GetTopLevelStagingInstanceBuffer() const
{
	return m_pBuffer;
}


void BvAccelerationStructureVk::Create()
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	if (m_Desc.m_Type == RayTracingAccelerationStructureType::kBottomLevel)
	{
		m_Geometries.Resize(m_Desc.m_BLAS.m_GeometryCount, { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR });
		m_PrimitiveCounts.Resize(m_Geometries.Size());

		for (auto i = 0u; i < m_Desc.m_BLAS.m_GeometryCount; ++i)
		{
			if (m_Desc.m_BLAS.m_pGeometries[i].m_Type == RayTracingGeometryType::kTriangles)
			{
				auto& srcGeometry = m_Desc.m_BLAS.m_pGeometries[i].m_Triangle;
				auto& dstGeometry = m_Geometries[i];
				auto& triangle = dstGeometry.geometry.triangles;

				//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				//dstGeometry.pNext = nullptr;
				dstGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				dstGeometry.flags = 0;

				triangle.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				triangle.pNext = nullptr;
				triangle.vertexFormat = GetVkFormat(srcGeometry.m_VertexFormat);
				triangle.vertexStride = srcGeometry.m_VertexStride;
				triangle.maxVertex = srcGeometry.m_VertexCount - 1;
				triangle.indexType = GetVkIndexType(srcGeometry.m_IndexFormat);

				auto primitiveCount = srcGeometry.m_IndexCount / 3;
				m_PrimitiveCounts[i] = primitiveCount;
			}
			else if (m_Desc.m_BLAS.m_pGeometries[i].m_Type == RayTracingGeometryType::kAABB)
			{
				auto& srcGeometry = m_Desc.m_BLAS.m_pGeometries[i].m_AABB;
				auto& dstGeometry = m_Geometries[i];
				auto& aabb = dstGeometry.geometry.aabbs;

				//dstGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				//dstGeometry.pNext = nullptr;
				dstGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
				dstGeometry.flags = 0;

				aabb.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
				aabb.pNext = nullptr;
				aabb.stride = srcGeometry.m_Stride;

				m_PrimitiveCounts[i] = srcGeometry.m_Count;
			}
		}
	}
	else if (m_Desc.m_Type == RayTracingAccelerationStructureType::kTopLevel)
	{
		m_Geometries.Resize(1, { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR });
		m_PrimitiveCounts.Resize(1, m_Desc.m_TLAS.m_InstanceCount);

		VkAccelerationStructureGeometryKHR &geometry = m_Geometries.Back();
		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

		auto& instances = geometry.geometry.instances;
		instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		instances.arrayOfPointers = VK_FALSE;
	}

	buildInfo.flags = GetVkBuildAccelerationStructureFlags(m_Desc.m_Flags);
	//buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.pGeometries = m_Geometries.Data();
	buildInfo.geometryCount = u32(m_Geometries.Size());

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };

	GetBuildSizes(buildInfo, m_PrimitiveCounts.Data(), sizeInfo);

	m_ScratchSizes.m_Build = sizeInfo.buildScratchSize;
	m_ScratchSizes.m_Update = sizeInfo.updateScratchSize;

	BufferDesc bufferDesc;
	bufferDesc.m_UsageFlags = BufferUsage::kRayTracing;
	bufferDesc.m_MemoryType = MemoryType::kDevice;
	bufferDesc.m_Size = sizeInfo.accelerationStructureSize;
	if (!m_pDevice->CreateBufferVk(bufferDesc, nullptr, &m_pBuffer))
	{
		return;
	}

	auto device = m_pDevice->GetHandle();

	VkAccelerationStructureCreateInfoKHR accelerationStructureCI{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	//accelerationStructureCI.createFlags = 0;
	accelerationStructureCI.buffer = m_pBuffer->GetHandle();
	//accelerationStructureCI.offset = 0;
	accelerationStructureCI.size = sizeInfo.accelerationStructureSize;
	accelerationStructureCI.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	if (vkCreateAccelerationStructureKHR(device, &accelerationStructureCI, nullptr, &m_Handle) != VK_SUCCESS)
	{
		Destroy();
		return;
	}

	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	addressInfo.accelerationStructure = m_Handle;
	m_DeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
}


void BvAccelerationStructureVk::Destroy()
{
	auto device = m_pDevice->GetHandle();
	if (m_Handle)
	{
		vkDestroyAccelerationStructureKHR(device, m_Handle, nullptr);
		m_Handle = nullptr;
	}

	if (m_pBuffer)
	{
		m_pBuffer->Release();
		m_pBuffer = nullptr;
	}
}


void BvAccelerationStructureVk::GetBuildSizes(const VkAccelerationStructureBuildGeometryInfoKHR& buildInfo, const u32* pPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR& sizeInfo)
{
	vkGetAccelerationStructureBuildSizesKHR(m_pDevice->GetHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, pPrimitiveCounts, &sizeInfo);
}