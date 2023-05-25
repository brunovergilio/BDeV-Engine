#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"


enum GPUVendorId : u32
{
	kUnknown = 0,
	kAMD = 0x1002,
	kImgTec = 0x1010,
	kNvidia = 0x10DE,
	kARM = 0x13B5,
	kQualcomm = 0x5143,
	kIntel = 0x8086,
};


struct BvGPUInfo
{
	const char* m_DeviceName = nullptr;
	u64 m_DeviceMemory = 0;
	u32 m_DeviceId = 0;
	u32 m_VendorId = 0;
	u32 m_GraphicsQueueCount = 0;
	u32 m_ComputeQueueCount = 0;
	u32 m_TransferQueueCount = 0;
	GPUVendorId m_Vendor = GPUVendorId::kUnknown;
};