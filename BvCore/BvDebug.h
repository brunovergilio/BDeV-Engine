#pragma once


#include "BvCore/BvPlatform.h"


#if defined(BV_DEBUG)

#include <cassert>
#include <stdio.h>
#include <cstdlib>

constexpr size_t kDebugMsgLen = 2048;

#define BV_ERROR(format, ...)						\
{																\
	char formattedErrorMessage[kDebugMsgLen]{};					\
	char errorMessage[kDebugMsgLen]{};							\
	sprintf_s(formattedErrorMessage, format, ## __VA_ARGS__);	\
	sprintf_s(errorMessage, "%s\n"								\
		"Function: %s()\n"										\
		"File: %s\n"											\
		"Line: %d", formattedErrorMessage,						\
		BV_FUNCTION, BV_FILE, BV_LINE);							\
	BvError(errorMessage);										\
}

#if (BV_PLATFORM == BV_PLATFORM_WIN32)

#include <intrin.h>

#define BvAssert(cond) assert(cond)
#define BvAssertMsg(cond, msg) BvAssert(cond && msg)
#define BvCompilerAssert(cond, msg) static_assert(cond, msg)
#define BvDebugBreak() __debugbreak()
#define BvDebugPrintString(msg) OutputDebugString(msg)

inline void BvError(const char * const pMessage)
{
	auto result = MessageBoxA(nullptr, pMessage, "BDeV Error", MB_ABORTRETRYIGNORE | MB_ICONERROR);
	if (result == IDABORT)
	{
		exit(1);
	}
	if (result == IDRETRY)
	{
		BvDebugBreak();
		return;
	}
	if (result == IDIGNORE)
	{
		return;
	}
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

#else

// Other platforms

#endif // #if BV_PLATFORM == BV_PLATFORM_WIN32



#define BvDebugPrint(...) \
{ \
	char buf[kDebugMsgLen]; \
	sprintf_s(buf, __VA_ARGS__); \
	BvDebugPrintString(buf); \
}

#define BvConsolePrint(...) \
{ \
	char buf[kDebugMsgLen]; \
	sprintf_s(buf, __VA_ARGS__); \
	printf_s(buf); \
}



#else

#define BvAssert(cond)
#define BvAssertMsg(cond, msg)
#define BvCompilerAssert(cond, msg)
#define BvDebugBreak()
#define BvDebugPrintString(msg)
#define BvDebugPrint(...)

#endif // #if defined(BV_DEBUG)