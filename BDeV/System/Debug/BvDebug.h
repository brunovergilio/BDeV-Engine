#pragma once


#include "BDeV/BvCore.h"
#include <cassert>


namespace BvDebugConstants
{
	constexpr u32 kMaxDebugMsgLen = 2048;
	static thread_local char s_ErrorMessage[kMaxDebugMsgLen];
}


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


// ================
// PrintF functions

// Default white text and black background printf
BV_API i32 PrintF(const char* pFormat, ...);
// Custom color text and black background printf
BV_API i32 PrintF(ConsoleColor textColor, const char* pFormat, ...);
// Custom color text and custom background printf
BV_API i32 PrintF(ConsoleColor textColor, ConsoleColor backGroundColor, const char* pFormat, ...);
// Same as printf but to the debug window (if one exists)
BV_API i32 DPrintF(const char* pFormat, ...);
// ================

BV_API void RaiseError(const char* pMessage, const char* pTitle = "BDeV Error");
BV_API u32 GetOSErrorCode();
BV_API const char* GetOSErrorMessage();
BV_API void DbgBreak();


#if BV_DEBUG

#define BV_ERROR(format, ...)											\
{																		\
	sprintf(BvDebugConstants::s_ErrorMessage, format, ## __VA_ARGS__);	\
	RaiseError(BvDebugConstants::s_ErrorMessage);						\
}

#define BV_OS_ERROR()															\
{																				\
	BV_ERROR("OS error [(0x%X)]: %s", GetOSErrorCode(), GetOSErrorMessage());	\
}

#if BV_PLATFORM == BV_PLATFORM_WIN32
#define BvAssert(cond, msg) if (!(cond)) { BV_ERROR("Error: %s\nCondition: %s\nFile: %s\nLine: %d\nFunction: %s", msg, #cond, BV_FILE, BV_LINE, BV_FUNCTION); }
#else
#define BvAssert(cond, msg) assert((cond) && msg)
#endif

#define BvCompilerAssert(cond, msg) static_assert((cond), msg)
#define BvDebugBreak() DbgBreak()

#else

#define BvAssert(cond, msg)
#define BvCompilerAssert(cond, msg)
#define BvDebugBreak()
#define BV_ERROR(format, ...)
#define BV_OS_ERROR()

#endif // #if BV_DEBUG