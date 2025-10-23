#include "BDeV/Core/System/Process/BvProcess.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include <BDeV/Core/Utils/BvText.h>
#include <DbgHelp.h>
#include <bit>


void FillUpSystemInfo(BvSystemInfo& systemInfo)
{
	SYSTEM_INFO osInfo;
	::GetSystemInfo(&osInfo);

	systemInfo.m_PageSize = osInfo.dwPageSize;
	systemInfo.m_LargePageSize = (u32)GetLargePageMinimum();

	DWORD bufferSize = 0;
	DWORD result = GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize);
	if (result == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		//pBufferData = BV_NEW_ARRAY(u8, bufferSize);
		//pBuffer = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(pBufferData);
		//constexpr u32 kMaxProcessorDataSize = 2_kb;
		//BV_ASSERT(bufferSize <= kMaxProcessorDataSize, "Increase stack buffer size");
		//u8 processorData[kMaxProcessorDataSize];
		u8* processorData = reinterpret_cast<u8*>(BV_STACK_ALLOC(bufferSize));
		result = GetLogicalProcessorInformationEx(RelationProcessorCore, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(processorData), &bufferSize);
		if (result == TRUE)
		{
			constexpr u32 kMaxNumCores = 64;
			constexpr u32 kMaxNumLogicalProcessors = 128;

			static BvSystemInfo::CoreInfo cores[kMaxNumCores]{};
			static u32 logicalProcessorIndices[kMaxNumLogicalProcessors]{};

			systemInfo.m_pCoreInfos = cores;

			DWORD offset = 0;

			while (offset < bufferSize)
			{
				PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX pProcessorInfo = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(processorData + offset);
				const PROCESSOR_RELATIONSHIP& rel = pProcessorInfo->Processor;

				BV_ASSERT(systemInfo.m_NumCores < kMaxNumCores, "Increase kMaxNumCores");

				auto& coreInfo = cores[systemInfo.m_NumCores++];

				coreInfo.m_SMT = (rel.Flags & LTP_PC_SMT) != 0;
				coreInfo.m_pLogicalProcessorIndices = &logicalProcessorIndices[systemInfo.m_NumLogicalProcessors];
				for (WORD g = 0; g < rel.GroupCount; ++g)
				{
					const GROUP_AFFINITY& ga = rel.GroupMask[g];
					KAFFINITY mask = ga.Mask;

					for (u32 bit = 0; bit < 64; ++bit)
					{
						if (mask & (1ull << bit))
						{
							BV_ASSERT(systemInfo.m_NumLogicalProcessors < kMaxNumLogicalProcessors, "Increase kMaxNumLogicalProcessors");

							// Logical processor index = group offset + bit
							// You can also track group separately if needed
							u32 logicalProcessorIndex = bit + ga.Group * 64;
							logicalProcessorIndices[systemInfo.m_NumLogicalProcessors++] = logicalProcessorIndex;

							coreInfo.m_NumLogicalProcessors++;
						}
					}
				}

				offset += pProcessorInfo->Size;
			}
		}
		else
		{
			BV_SYS_FATAL();
		}

		//BV_DELETE_ARRAY(pBufferData);
	}
}


const BvSystemInfo& BvSystem::GetSystemInfo()
{
	static const BvSystemInfo& s_SystemInfo = []()
		{
			static BvSystemInfo systemInfo;
			FillUpSystemInfo(systemInfo);

			return systemInfo;
		}();

	//Console::Print(BvColorI::BrightGreen, "Num Cores: %d\n", sys.m_NumCores);
	//Console::Print(BvColorI::BrightGreen, "Num Logical Processors: %d\n", sys.m_NumLogicalProcessors);
	//Console::Print(BvColorI::BrightGreen, "Page Size: %d\n", sys.m_PageSize);
	//Console::Print(BvColorI::BrightGreen, "Large Page Size: %d\n", sys.m_LargePageSize);
	//Console::Print(BvColorI::BrightGreen, "Core Info:\n");
	//for (auto i = 0; i < sys.m_NumCores; ++i)
	//{
	//	auto& coreInfo = sys.m_pCoreInfos[i];
	//	Console::Print(BvColorI::BrightGreen, "\nCore %d:\n", i);
	//	Console::Print(BvColorI::BrightGreen, "\tIs SMT: %s\n", coreInfo.m_SMT ? "Yes" : "No");
	//	Console::Print(BvColorI::BrightGreen, "\tNum Logical Processors: %d [ ", coreInfo.m_NumLogicalProcessors);
	//	for (auto j = 0; j < coreInfo.m_NumLogicalProcessors; ++j)
	//	{
	//		Console::Print(BvColorI::BrightGreen, "%d ", coreInfo.m_pLogicalProcessorIndices[j]);
	//	}
	//	Console::Print(BvColorI::BrightGreen, "]\n");
	//}

	return s_SystemInfo;
}


#pragma warning(push)
#pragma warning(disable:4996)
//#pragma comment(lib, "dbghelp.lib")

#define BV_LOADPROC(proc) m_SymLib.GetProcAddressT(#proc, PFn##proc)
#define BV_CALLPROC(proc) s_StackTraceHandler.PFn##proc

class StackTraceHandler
{
public:
	StackTraceHandler()
		: m_SymLib("dbghelp.dll")
	{
		BV_LOADPROC(SymInitialize);
		BV_LOADPROC(SymCleanup);
		BV_LOADPROC(SymSetOptions);
		BV_LOADPROC(StackWalk64);
		BV_LOADPROC(SymFunctionTableAccess64);
		BV_LOADPROC(SymGetModuleBase64);
		BV_LOADPROC(SymGetSymFromAddr64);
		BV_LOADPROC(SymGetLineFromAddr64);

		if (!(PFnSymInitialize
			&& PFnSymCleanup
			&& PFnSymSetOptions
			&& PFnStackWalk64
			&& PFnSymFunctionTableAccess64
			&& PFnSymGetModuleBase64
			&& PFnSymGetSymFromAddr64
			&& PFnSymGetLineFromAddr64))
		{
			m_SymLib.Close();
			return;
		}

		if (PFnSymInitialize(GetCurrentProcess(), nullptr, TRUE))
		{
			PFnSymSetOptions(SYMOPT_LOAD_LINES);
			m_IsValid = true;
		}
	}
	~StackTraceHandler()
	{
		if (m_IsValid)
		{
			PFnSymCleanup(GetCurrentProcess());
		}
	}

	BV_INLINE bool IsValid() const { return m_IsValid; }

	BOOL(*PFnSymInitialize)(HANDLE, PCSTR, BOOL);
	BOOL(*PFnSymCleanup)(HANDLE);
	DWORD(*PFnSymSetOptions)(DWORD);
	BOOL(*PFnStackWalk64)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64,
		PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
	PVOID(*PFnSymFunctionTableAccess64)(HANDLE, DWORD64);
	DWORD64(*PFnSymGetModuleBase64)(HANDLE, DWORD64);
	BOOL(*PFnSymGetSymFromAddr64)(HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64);
	BOOL(*PFnSymGetLineFromAddr64)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);

private:
	BvSharedLib m_SymLib;
	bool m_IsValid = false;
};


bool BvSystem::GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip, const u32 numFramesToRecord)
{
	static StackTraceHandler s_StackTraceHandler;
	if (!s_StackTraceHandler.IsValid())
	{
		return false;
	}

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

	BOOL result = TRUE;
	for (auto i = 0u; i < numFramesToSkip; i++)
	{
		result = BV_CALLPROC(StackWalk64)(machine, process, GetCurrentThread(), &frame, &context,
			nullptr, BV_CALLPROC(SymFunctionTableAccess64), BV_CALLPROC(SymGetModuleBase64), nullptr);
		if (!result)
		{
			break;
		}
	}

	if (!result)
	{
		return false;
	}

	for (auto i = 0u; i < numFramesToRecord; i++)
	{
		result = BV_CALLPROC(StackWalk64)(machine, process, GetCurrentThread(), &frame, &context,
			nullptr, BV_CALLPROC(SymFunctionTableAccess64), BV_CALLPROC(SymGetModuleBase64), nullptr);
		if (!result)
		{
			break;
		}

		stackTrace.frames.PushBack({});
		auto& currFrame = stackTrace.frames.Back();

		currFrame.m_Address = frame.AddrPC.Offset;

		DWORD moduleBase = BV_CALLPROC(SymGetModuleBase64)(process, frame.AddrPC.Offset);
		wchar_t moduleNameW[kMaxNameSize]{};
		if (moduleBase && GetModuleFileNameW((HINSTANCE)moduleBase, moduleNameW, kMaxNameSize))
		{
			auto sizeNeeded = BvTextUtilities::ConvertWideCharToUTF8Char(moduleNameW, 0, nullptr, 0);
			if (sizeNeeded > 0)
			{
				currFrame.m_Module.SetAllocator(stackTrace.frames.GetAllocator());
				currFrame.m_Module.Resize(sizeNeeded - 1, ' ');
				BvTextUtilities::ConvertWideCharToUTF8Char(moduleNameW, 0, &currFrame.m_Module[0], sizeNeeded);
			}
		}

		auto pSymbol = reinterpret_cast<IMAGEHLP_SYMBOL64*>(dbgHelpSymbolMem);
		pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + kMaxNameSize;
		pSymbol->MaxNameLength = kMaxNameSize - 1;

		if (BV_CALLPROC(SymGetSymFromAddr64)(process, frame.AddrPC.Offset, nullptr, pSymbol))
		{
			currFrame.m_Function.SetAllocator(stackTrace.frames.GetAllocator());
			currFrame.m_Function = pSymbol->Name;
		}

		DWORD offset = 0;
		IMAGEHLP_LINE line;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

		if (BV_CALLPROC(SymGetLineFromAddr64)(process, frame.AddrPC.Offset, &offset, &line))
		{
			currFrame.m_File.SetAllocator(stackTrace.frames.GetAllocator());
			currFrame.m_File = line.FileName;
			currFrame.m_Line = line.LineNumber;
		}
	}

	return true;
}

#undef BV_LOADPROC
#undef BV_CALLPROC

#pragma warning(pop)


void BvCPU::Yield()
{
	YieldProcessor();
}