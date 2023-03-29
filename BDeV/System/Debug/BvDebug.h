#pragma once


#include "BDeV/BvCore.h"
#include <cassert>


enum ConsoleColor : u16
{
	kBlack,
	kBlue,
	kGreen,
	kAqua,
	kRed,
	kPurple,
	kYellow,
	kWhite,
	kGrey,
	kLightBlue,
	kLightGreen,
	kLightAqua,
	kLightRed,
	kLightPurple,
	kLightYellow,
	kBrightWhite,
};


constexpr u32 kDebugMsgLen = 2048;


// ================
// PrintF functions

// Default white text and black background printf
BV_API int PrintF(const char* pFormat, ...);
// Custom color text and black background printf
BV_API int PrintF(ConsoleColor textColor, const char* pFormat, ...);
// Custom color text and custom background printf
BV_API int PrintF(ConsoleColor textColor, ConsoleColor backGroundColor, const char* pFormat, ...);
// Same as printf but to the debug window (if one exists)
BV_API int DPrintF(const char* pFormat, ...);
// ================

BV_API void RaiseError(const char* pMessage, const char* pTitle = "BDeV Error");
BV_API u32 GetOSError(char* pErrorMessage = nullptr, u32 bufferSize = 0);
BV_API void MakeDebugBreak();


#if BV_DEBUG

#define BvAssert(cond, msg) assert((cond) && msg)
#define BvCompilerAssert(cond, msg) static_assert(cond, msg)
#define BvDebugBreak() MakeDebugBreak()

#define BV_ERROR(format, ...)								\
{															\
	static thread_local char errorMessage[kDebugMsgLen]{};	\
	sprintf_s(errorMessage, format, ## __VA_ARGS__);		\
	RaiseError(errorMessage);								\
}

#define BV_OS_ERROR()												\
{																	\
	static thread_local char osErrorMessage[kDebugMsgLen]{};		\
	u32 errorCode = GetOSError(osErrorMessage, kDebugMsgLen);		\
	BV_ERROR("OS error [(0x%X)]: %s", errorCode, osErrorMessage);	\
}

#else

#define BvAssert(cond, msg)
#define BvCompilerAssert(cond, msg)
#define BvDebugBreak()
#define BV_ERROR(format, ...)
#define BV_OS_ERROR()

#endif // #if BV_DEBUG