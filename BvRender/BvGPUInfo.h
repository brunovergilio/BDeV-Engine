#pragma once


#include "BvCore/BvCore.h"
#include "BvCore/Container/BvString.h"


struct BvGPUInfo
{
	BvString m_DeviceName;
	u64 m_DeviceMemory = 0;
	u32 m_DeviceId = 0;
	u32 m_VendorId = 0;
	u32 m_GraphicsQueueCount = 0;
	u32 m_ComputeQueueCount = 0;
	u32 m_TransferQueueCount = 0;
};