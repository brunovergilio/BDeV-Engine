#include "BDeV/Core/System/Threading/BvProcess.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Windows/BvWindowsHeader.h"
#include <DbgHelp.h>
#include <bit>


const BvSystemInfo& BvProcess::GetSystemInfo()
{
	static BvSystemInfo s_SystemInfo = []()
	{
		BvSystemInfo systemInfo{};

		u8* pBufferData = nullptr;
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pBuffer = nullptr;
		DWORD bufferSize = 0;
		DWORD result = GetLogicalProcessorInformation(pBuffer, &bufferSize);
		if (result == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			pBufferData = BV_NEW_ARRAY(u8, bufferSize);
			pBuffer = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(pBufferData);
			result = GetLogicalProcessorInformation(pBuffer, &bufferSize);
			if (result == FALSE)
			{
				BV_DELETE_ARRAY(pBufferData);
				BvOSCrashIfFailed(result);
				return systemInfo;
			}
		}
		else
		{
			BvOSCrashIfFailed(result);
		}

		auto count = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

		for (auto i = 0u; i < count; i++)
		{
			if (pBuffer[i].Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore)
			{
				systemInfo.m_NumCores++;

				// A hyperthreaded core supplies more than one logical processor.
				systemInfo.m_NumLogicalProcessors += std::popcount(pBuffer[i].ProcessorMask);
			}
			else if (pBuffer[i].Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationCache)
			{
				if (pBuffer->Cache.Level == 1) { systemInfo.m_L1CacheCount++; }
				else if (pBuffer->Cache.Level == 2) { systemInfo.m_L2CacheCount++; }
				else if (pBuffer->Cache.Level == 3) { systemInfo.m_L3CacheCount++; }
			}
		}

		BV_DELETE_ARRAY(pBufferData);

		SYSTEM_INFO osInfo;
		::GetSystemInfo(&osInfo);
		systemInfo.m_PageSize = osInfo.dwPageSize;
		systemInfo.m_LargePageSize = osInfo.dwPageSize;

		if (auto largePageSize = (u32)GetLargePageMinimum())
		{
			systemInfo.m_LargePageSize = largePageSize;
		}

		return systemInfo;
	}();

	return s_SystemInfo;
}


#pragma warning(push)
#pragma warning(disable:4996)
#pragma comment(lib, "dbghelp.lib")

class StackTraceHandler
{
public:
	StackTraceHandler()
	{
		SymInitialize(GetCurrentProcess(), nullptr, TRUE);
		SymSetOptions(SYMOPT_LOAD_LINES);
	}
	~StackTraceHandler()
	{
		SymCleanup(GetCurrentProcess());
	}
};


void BvProcess::GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip, const u32 numFramesToRecord)
{
	static StackTraceHandler s_StackTraceHandler;

	stackTrace.frames.Reserve(numFramesToRecord);

	constexpr auto kMaxNameSize = MAX_PATH;
	u8 dbgHelpSymbolMem[sizeof(IMAGEHLP_SYMBOL64) + kMaxNameSize];

	CONTEXT context;
	RtlCaptureContext(&context);

	STACKFRAME64 frame{};
	frame.AddrPC.Offset = context.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;

	DWORD machine = 0;
#if defined(_M_X64)
	machine = IMAGE_FILE_MACHINE_AMD64;
//#elif defined(_M_IX86)
//	machine = IMAGE_FILE_MACHINE_I386;
//#elif defined(_M_IA64)
//	machine = IMAGE_FILE_MACHINE_IA64;
#else
#error "platform not supported!"
#endif

	HANDLE process = GetCurrentProcess();

	// Since we don't care about this function, we do an initial StackWalk64 call
	// and ditch the first result
	BOOL result = TRUE;
	for (auto i = 0u; i < numFramesToSkip; i++)
	{
		result = StackWalk64(machine, process, GetCurrentThread(), &frame, &context,
			nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr);
		if (!result)
		{
			break;
		}
	}

	if (!result)
	{
		return;
	}

	for (auto i = 0u; i < numFramesToRecord; i++)
	{
		result = StackWalk64(machine, process, GetCurrentThread(), &frame, &context,
			nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr);
		if (!result)
		{
			break;
		}

		stackTrace.frames.PushBack({});
		auto& currFrame = stackTrace.frames.Back();

		currFrame.m_Address = frame.AddrPC.Offset;

		DWORD moduleBase = SymGetModuleBase64(process, frame.AddrPC.Offset);
		char moduleBuff[kMaxNameSize]{};
		if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduleBuff, kMaxNameSize))
		{
			currFrame.m_Module = moduleBuff;
		}

		auto pSymbol = reinterpret_cast<IMAGEHLP_SYMBOL64*>(dbgHelpSymbolMem);
		pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + kMaxNameSize;
		pSymbol->MaxNameLength = kMaxNameSize - 1;

		if (SymGetSymFromAddr64(process, frame.AddrPC.Offset, nullptr, pSymbol))
		{
			currFrame.m_Function = pSymbol->Name;
		}

		DWORD offset = 0;
		IMAGEHLP_LINE line;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

		if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &offset, &line))
		{
			currFrame.m_File = line.FileName;
			currFrame.m_Line = line.LineNumber;
		}
	}
}
#pragma warning( pop )


void BvProcess::YieldExecution()
{
	YieldProcessor();
}