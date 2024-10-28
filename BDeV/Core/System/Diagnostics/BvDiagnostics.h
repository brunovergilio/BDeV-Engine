#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/RenderAPI/BvColor.h"


class BvConsole
{
public:
	// Default white text and black background printf
	static i32 PrintF(const char* pFormat, ...);
	// Custom color text and black background printf
	static i32 PrintF(const BvColorI& textColor, const char* pFormat, ...);
	// Custom color text and custom color background printf
	static i32 PrintF(const BvColorI& textColor, const BvColorI& backGroundColor, const char* pFormat, ...);
};


class BvDebug
{
public:
	// Same as printf but to the debug window (if one exists)
	static i32 PrintF(const char* pFormat, ...);

	// Should only be called by assert macros
	static void RaiseAssertionError(const char* pCondition, const BvSourceInfo& sourceInfo, const char* pFormat, ...);
};


class BvError
{
public:
	static void RaiseError(const BvSourceInfo& sourceInfo, const char* pFormat, ...);
	static void RaiseOSError(const BvSourceInfo& sourceInfo);
};


#if BV_DEBUG

#define BvAssert(cond, msg, ...) do															\
{																							\
	if (!(cond))																			\
	{																						\
		BvDebug::RaiseAssertionError(#cond, BV_SOURCE_INFO, msg __VA_OPT__(,) __VA_ARGS__);	\
	}																						\
} while (false)

#define BvAssertOnce(cond, msg, ...) do														\
{																							\
	static bool log = true;																	\
	if (log && !(cond))																		\
	{																						\
		BvDebug::RaiseAssertionError(#cond, BV_SOURCE_INFO, msg __VA_OPT__(,) __VA_ARGS__);	\
		log = false;																		\
	}																						\
} while (false)


#else

#define BvAssert(cond, msg, ...)
#define BvAssertOnce(cond, msg, ...)

#endif // #if BV_DEBUG


// Tests a condition and crash if it fails with a custom message
#define BvCrashIfFailed(cond, msg, ...) do									\
{																			\
	if (!(cond))															\
	{																		\
		BvError::RaiseError(BV_SOURCE_INFO, msg __VA_OPT__(,) __VA_ARGS__);	\
	}																		\
} while (false)


// Tests a condition resulting from an OS call and crash if it fails with a predefined system message
#define BvOSCrashIfFailed(cond) do				\
{												\
	if (!(cond))								\
	{											\
		BvError::RaiseOSError(BV_SOURCE_INFO);	\
	}											\
} while (false)