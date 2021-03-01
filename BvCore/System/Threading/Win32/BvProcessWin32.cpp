#include "BvCore/System/Threading/Win32/BvProcessWin32.h"
#include "BvCore/System/Memory/BvMemory.h"
#include "BvCore/Container/BvVector.h"
#include <Windows.h>


BvSystemInfo g_SystemInfo = []()
{
	BvSystemInfo systemInfo{};

	BvVector<u8> bufferData;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pBuffer = nullptr;
	DWORD bufferSize = 0;
	DWORD result = GetLogicalProcessorInformation(pBuffer, &bufferSize);
	if (result == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		bufferData.Resize(bufferSize);
		pBuffer = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(bufferData.Data());
		result = GetLogicalProcessorInformation(pBuffer, &bufferSize);
		if (result == FALSE)
		{
			BV_WIN32_ERROR();
			return systemInfo;
		}
	}
	else
	{
		BV_WIN32_ERROR();
	}

	auto count = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

	for (auto i = 0u; i < count; i++)
	{
		if (pBuffer[i].Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore)
		{
			systemInfo.m_NumCores++;

			// A hyperthreaded core supplies more than one logical processor.
			systemInfo.m_NumLogicalProcessors += __popcnt(pBuffer[i].ProcessorMask);
		}
	}

	SYSTEM_INFO osInfo;
	GetSystemInfo(&osInfo);
	systemInfo.m_MemPageSize = osInfo.dwPageSize;

	return systemInfo;
}();


const BvSystemInfo & GetSystemInfo()
{
	return g_SystemInfo;
}