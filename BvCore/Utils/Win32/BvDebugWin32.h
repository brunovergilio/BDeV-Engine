#pragma once


#include <intrin.h>
#include <cassert>


constexpr size_t kDebugMsgLen = 2048;


void RaiseError(const char* const pMessage, const char* pTitle = "BDeV Error");
int DPrintF(const char* pFormat, ...);


#define BvAssert(cond, msg) assert((cond) && msg)
#define BvCompilerAssert(cond, msg) static_assert(cond, msg)
#define BvDebugBreak() __debugbreak()


#define BV_ERROR(format, ...)									\
{																\
	char formattedErrorMessage[kDebugMsgLen]{};					\
	char errorMessage[kDebugMsgLen]{};							\
	sprintf_s(formattedErrorMessage, format, ## __VA_ARGS__);	\
	sprintf_s(errorMessage, "%s\n"								\
		"Function: %s()\n"										\
		"File: %s\n"											\
		"Line: %d", formattedErrorMessage,						\
		BV_FUNCTION, BV_FILE, BV_LINE);							\
	RaiseError(errorMessage);									\
}


#define BV_WIN32_ERROR()														\
{																				\
	auto errorCode = GetLastError();											\
	DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;	\
	char win32ErrorMessage[kDebugMsgLen]{};										\
	DWORD dwResult = FormatMessage(dwFlags, nullptr, errorCode, 0,				\
		win32ErrorMessage, kDebugMsgLen, nullptr);								\
	BV_ERROR("Win32 error: (0x%X) %s", errorCode, win32ErrorMessage);			\
}