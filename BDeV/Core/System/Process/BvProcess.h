#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/Container/BvVector.h"


struct BvSystemInfo
{
	u32 m_NumCores;
	u32 m_NumLogicalProcessors;
	u32 m_PageSize;
	u32 m_LargePageSize;

	struct CoreInfo
	{
		u32 m_NumLogicalProcessors;
		const u32* m_pLogicalProcessorIndices;
		bool m_SMT;
	};
	const CoreInfo* m_pCoreInfos;
};


struct BvStackTrace
{
	struct Frame
	{
		BvString m_Function;
		BvString m_Module;
		BvString m_File;
		u64 m_Address;
		u32 m_Line;
	};

	BvVector<Frame> frames;
};


namespace BvSystem
{
	const BvSystemInfo& GetSystemInfo();
	bool GetStackTrace(BvStackTrace& stackTrace, const u32 numFramesToSkip = 1, const u32 numFramesToRecord = 3);
};


namespace BvCPU
{
	void Yield();
}


// For Linux and MacOS
//info.m_NumLogicalProcessors = sysconf(_SC_NPROCESSORS_ONLN);
//info.m_PageSize = sysconf(_SC_PAGESIZE);
//#if defined(__APPLE__)
//int physicalCores;
//size_t size = sizeof(physicalCores);
//sysctlbyname("hw.physicalcpu", &physicalCores, &size, nullptr, 0);
//info.m_NumCores = physicalCores;
//size_t size;
//int cacheSize;
//size = sizeof(cacheSize);
//sysctlbyname("hw.l1icachesize", &cacheSize, &size, nullptr, 0);
//info.m_L1CacheCount = cacheSize;
//sysctlbyname("hw.l2cachesize", &cacheSize, &size, nullptr, 0);
//info.m_L2CacheCount = cacheSize;
//sysctlbyname("hw.l3cachesize", &cacheSize, &size, nullptr, 0);
//info.m_L3CacheCount = cacheSize;
//info.m_LargePageSize = 2_mb // mac doesn't have that - default to 2 mb
//#elif defined(__linux__)
//info.m_NumCores = sysconf(_SC_NPROCESSORS_CONF);
//info.m_L1CacheCount = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index0/size");
//info.m_L2CacheCount = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index2/size");
//info.m_L3CacheCount = readCacheSize("/sys/devices/system/cpu/cpu0/cache/index3/size");
//info.m_LargePageSize = sysconf(_SC_LARGE_PAGESIZE); // If supported
//#endif


//#include <execinfo.h>
//#include <dlfcn.h>
//#include <cxxabi.h>
//#include <vector>
//#include <string>
//
//void captureStackTrace(BvStackTrace& trace, size_t maxFrames = 64)
// {
//	void* buffer[maxFrames];
//	int frameCount = backtrace(buffer, maxFrames);
//	char** symbols = backtrace_symbols(buffer, frameCount);
//
//	if (!symbols) {
//		return;
//	}
//
//	for (int i = 0; i < frameCount; ++i) {
//		BvStackTrace::Frame frame;
//
//		// Resolve symbol information
//		Dl_info info;
//		if (dladdr(buffer[i], &info) && info.dli_sname) {
//			// Demangle function name
//			int status = 0;
//			char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
//			frame.m_Function = (status == 0 && demangled) ? demangled : info.dli_sname;
//			free(demangled);
//
//			frame.m_Module = info.dli_fname ? info.dli_fname : "Unknown Module";
//			frame.m_Address = reinterpret_cast<uint64_t>(buffer[i]);
//		}
//		else {
//			frame.m_Function = "Unknown Function";
//			frame.m_Module = "Unknown Module";
//			frame.m_Address = reinterpret_cast<uint64_t>(buffer[i]);
//		}
//
//		// File and line resolution using addr2line (Linux only)
//#if defined(__linux__)
//		char command[512];
//		snprintf(command, sizeof(command), "addr2line -e %s -p -f %p", frame.m_Module.c_str(), buffer[i]);
//		FILE* pipe = popen(command, "r");
//		if (pipe) {
//			char line[256];
//			if (fgets(line, sizeof(line), pipe)) {
//				frame.m_File = line;
//				// Optionally, parse the file and line number from the output
//			}
//			pclose(pipe);
//		}
//#elif defined(__APPLE__)
//     char command[512];
//		snprintf(command, sizeof(command), "atos -o %s -l 0x%lx 0x%lx", binaryPath.c_str(), 0x0, address);
//		
//		FILE* pipe = popen(command, "r");
//		if (pipe) {
//			char line[256];
//			if (fgets(line, sizeof(line), pipe)) {
//				std::string output(line);
//		
//				// Parse output, e.g., "function_name (in module_name) (source_file:line)"
//				size_t fileStart = output.find_last_of('(');
//				size_t lineStart = output.find_last_of(':');
//				size_t lineEnd = output.find_last_of(')');
//		
//				if (fileStart != std::string::npos && lineStart != std::string::npos && lineEnd != std::string::npos) {
//					frame.m_Function = output.substr(0, fileStart - 1);
//					frame.m_File = output.substr(fileStart + 1, lineStart - fileStart - 1);
//					frame.m_Line = std::stoi(output.substr(lineStart + 1, lineEnd - lineStart - 1));
//				}
//			}
//			pclose(pipe);
//		}
// // OR
//		char command[512];
//		snprintf(command, sizeof(command), "lldb --batch -o \"image lookup --address 0x%lx\" %s", address, binaryPath.c_str());
//		
//		FILE* pipe = popen(command, "r");
//		if (pipe) {
//			char line[256];
//			while (fgets(line, sizeof(line), pipe)) {
//				std::string output(line);
//		
//				// Look for file/line information, e.g., "file: <path>, line: <number>"
//				size_t filePos = output.find("file:");
//				size_t linePos = output.find("line:");
//				if (filePos != std::string::npos && linePos != std::string::npos) {
//					frame.m_File = output.substr(filePos + 6, linePos - filePos - 7);
//					frame.m_Line = std::stoi(output.substr(linePos + 6));
//				}
//		
//				// Optionally capture function/module names here too
//			}
//			pclose(pipe);
//		}
//#endif
//
//		trace.frames.push_back(frame);
//	}
//
//	free(symbols);
//}