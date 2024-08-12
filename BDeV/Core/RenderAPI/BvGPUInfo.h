#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvString.h"


enum class GPUVendorId : u32
{
	kUnknown,
	kAMD,
	kImgTec,
	kNvidia,
	kARM,
	kQualcomm,
	kIntel,
};


enum GUPCapabilities : u64
{
	kWireframe,
	kTesselationShader,
	kGeometryShader,
	kShadingRate,
	kMeshShader,
	kRayTracing,
	kRayQuery,
	kCustomBorderColor,
	kConservativeRasterization,
	kIndirectDraw,
	kPredication,
	kTimestampQueries,
	kOcclusionQueries,
};

enum class GPUType : u8
{
	kUnknown,
	kDiscrete,
	kIntegrated
};


struct BvGPUInfo
{
	const char* m_DeviceName = nullptr;
	u64 m_DeviceMemory = 0;
	u32 m_DeviceId = 0;
	u32 m_VendorId = 0;
	GPUVendorId m_Vendor = GPUVendorId::kUnknown;
};