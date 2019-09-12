#include "BvCore/System/Threading/BvProcess.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


BvSystemInfo::SystemInfoData BvSystemInfo::s_SystemInfoData;


BvSystemInfo::SystemInfoData::SystemInfoData()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	m_NumCores = sysInfo.dwNumberOfProcessors;
	m_MemPageSize = sysInfo.dwPageSize;
}


#endif