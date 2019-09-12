#pragma once


#include "BvDefines.h"


#if defined(BV_DEBUG)

#include <cassert>
#include <stdio.h>

#define BV_DEBUG_MSG_LEN 2048

#if (BV_PLATFORM == BV_PLATFORM_WIN32)

#include <intrin.h>

#define BvAssert(cond) assert(cond)
#define BvAssertMsg(cond, msg) BvAssert(cond && msg)
#define BvCompilerAssert(cond) static_assert(cond)
#define BvCompilerAssertMsg(cond, msg) static_assert(cond, msg)
#define BvDebugBreak() __debugbreak()
#define BvDebugPrintString(msg) OutputDebugString(msg)
#else

// Other platforms

#endif // #if BV_PLATFORM == BV_PLATFORM_WIN32

#define BvDebugPrint(...) \
{ \
	char buf[BV_DEBUG_MSG_LEN]; \
	sprintf_s(buf, __VA_ARGS__); \
	BvDebugPrintString(buf); \
}

#else

#define BvAssert(cond)
#define BvAssertMsg(cond, msg)
#define BvCompilerAssert(cond)
#define BvCompilerAssertMsg(cond, msg)
#define BvDebugBreak()
#define BvDebugPrintString(msg)
#define BvDebugPrint(...)

#endif // #if defined(BV_DEBUG)