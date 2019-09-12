#pragma once


#include "BvCore/BvDefines.h"


struct BvSystemInfo
{
	BV_INLINE static const u32 GetNumCores() { return s_SystemInfoData.m_NumCores; }
	BV_INLINE static const u32 GetMemPageSize() { return s_SystemInfoData.m_MemPageSize; }

private:
	struct SystemInfoData
	{
		SystemInfoData();

		u32 m_NumCores = 0;
		u32 m_MemPageSize = 0;
	};
	static SystemInfoData s_SystemInfoData;
};