#include "BvCore/System/Threading/BvProcess.h"
#include "BvCore/System/Memory/BvMemory.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <intrin.h>

BvSystemInfo g_SystemInfo = []()
{
	BvSystemInfo systemInfo{};

	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pBuffer = nullptr;
	DWORD bufferSize;
	DWORD result = GetLogicalProcessorInformation(pBuffer, &bufferSize);
	if (result == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		pBuffer = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(malloc(bufferSize));
		if (!pBuffer)
		{
			return systemInfo;
		}

		result = GetLogicalProcessorInformation(pBuffer, &bufferSize);
		if (result == FALSE)
		{
			return systemInfo;
		}
	}

	auto count = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

	for (auto i = 0u; i < count; i++)
	{
		if (pBuffer[i].Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore)
		{
			systemInfo.m_NumCores++;

			// A hyperthreaded core supplies more than one logical processor.
			systemInfo.m_NumLogicalProcessors += __popcnt(pBuffer[i].ProcessorMask);
			break;
		}
	}

	free(pBuffer);

	SYSTEM_INFO osInfo;
	GetSystemInfo(&osInfo);
	systemInfo.m_MemPageSize = osInfo.dwPageSize;

	return systemInfo;
}();


const BvSystemInfo & GetSystemInfo()
{
	return g_SystemInfo;
}


#endif