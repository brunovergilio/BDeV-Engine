#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"


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
	kShadingRate,
	kMeshShader,
	kRayTracing,
	kConservativeRasterization
};


struct BvGPUInfo
{
	const char* m_DeviceName = nullptr;
	u64 m_DeviceMemory = 0;
	u32 m_DeviceId = 0;
	u32 m_VendorId = 0;
	GPUVendorId m_Vendor = GPUVendorId::kUnknown;
};