#include "BvCore/System/Threading/Win32/BvProcessWin32.h"
#include "BvCore/System/Memory/BvMemory.h"
#include "BvCore/Container/BvVector.h"
#include <Windows.h>
#include <DbgHelp.h>


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
} g_StackTraceHandler;


void GetStackTrace(BvStackTrace& stack, const u32 numFrames)
{
	stack.frames.Reserve(numFrames);

	CONTEXT             context;
	STACKFRAME64        stackFrame;
	DWORD64             displacement;
	HANDLE				process = GetCurrentProcess();
	constexpr auto kMaxNameSize = MAX_PATH;
	u8 dbgHelpSymbolMem[sizeof(IMAGEHLP_SYMBOL64) + kMaxNameSize];

	RtlCaptureContext(&context);
	memset(&stackFrame, 0, sizeof(STACKFRAME64));

	displacement = 0;
	stackFrame.AddrPC.Offset = context.Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = context.Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Rbp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

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

	BOOL result;
	for (auto i = 0u; i < numFrames; i++)
	{
		result = StackWalk64(machine, process, GetCurrentThread(), &stackFrame, &context,
			nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr);
		if (!result)
		{
			break;
		}

		stack.frames.PushBack({});
		auto& currFrame = stack.frames.Back();

		currFrame.m_Address = stackFrame.AddrPC.Offset;

		DWORD moduleBase = SymGetModuleBase64(process, stackFrame.AddrPC.Offset);
		char moduleBuff[kMaxNameSize];
		if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduleBuff, kMaxNameSize))
		{
			currFrame.m_Module = moduleBuff;
		}

		auto pSymbol = reinterpret_cast<IMAGEHLP_SYMBOL64*>(dbgHelpSymbolMem);
		pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + kMaxNameSize;
		pSymbol->MaxNameLength = kMaxNameSize - 1;

		if (SymGetSymFromAddr64(process, stackFrame.AddrPC.Offset, nullptr, pSymbol))
		{
			currFrame.m_Function = pSymbol->Name;
		}

		DWORD offset = 0;
		IMAGEHLP_LINE line;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

		if (SymGetLineFromAddr64(process, stackFrame.AddrPC.Offset, &offset, &line))
		{
			currFrame.m_File = line.FileName;
			currFrame.m_Line = line.LineNumber;
		}

		//SymGetSymFromAddr64(process, (ULONG64)stackFrame.AddrPC.Offset, &displacement, pSymbol);
		//UnDecorateSymbolName(pSymbol->Name, pSymbol->Name, kDbgSymbolNameSize, UNDNAME_COMPLETE);

		//printf
		//(
		//	"Frame %lu:\n"
		//	"    Symbol name:    %s\n"
		//	"    PC address:     0x%08llu\n"
		//	"    Stack address:  0x%08llu\n"
		//	"    Frame address:  0x%08llu\n"
		//	"\n",
		//	frame,
		//	pSymbol->Name,
		//	(ULONG64)stackFrame.AddrPC.Offset,
		//	(ULONG64)stackFrame.AddrStack.Offset,
		//	(ULONG64)stackFrame.AddrFrame.Offset
		//);
	}
}
#pragma warning( pop )