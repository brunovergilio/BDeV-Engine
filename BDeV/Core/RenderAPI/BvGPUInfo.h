#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvString.h"


#if defined(DeviceCapabilities)
#undef DeviceCapabilities
#endif


enum class GPUType : u8
{
	kUnknown,
	kDiscrete,
	kIntegrated
};


enum class GPUVendor : u8
{
	kUnknown,
	kAMD,
	kImgTec,
	kNvidia,
	kARM,
	kQualcomm,
	kIntel,
};


struct BvGPUInfo
{
	static constexpr u32 kMaxDeviceNameSize = 256;

	char m_DeviceName[kMaxDeviceNameSize];
	u32 m_DeviceId;
	u32 m_VendorId;
	u64 m_DeviceMemory;
	u32 m_GraphicsContextCount;
	u32 m_ComputeContextCount;
	u32 m_TransferContextCount;
	GPUType m_Type;
	GPUVendor m_Vendor;
	void* m_pPhysicalDevice;
};