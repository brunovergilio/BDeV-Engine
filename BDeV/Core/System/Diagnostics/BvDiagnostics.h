#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/RenderAPI/BvColor.h"


class BvConsole
{
public:
	// Default white text and black background printf
	static void Print(const char* pFormat, ...);
	// Custom color text and black background printf
	static void Print(const BvColorI& textColor, const char* pFormat, ...);
	// Custom color text and custom color background printf
	static void Print(const BvColorI& textColor, const BvColorI& backGroundColor, const char* pFormat, ...);
};


class BvDebug
{
public:
	// Same as printf but to the debug window (if one exists)
	static void Print(const char* pFormat, ...);

	// To force code breaks when debugging
	static void Break();
};


class BvError
{
public:
	BvError() = default;

	i32 GetCode() const;
	const char* GetMessage() const;
};


namespace Internal
{
	// ===============================================
	// These classes should only be called by macros
	// ===============================================

	class BvAssert
	{
	public:
		BvAssert(const char* pCondition, const BvSourceInfo& sourceInfo, const char* pFormat, ...);
	};

	class BvSTDError
	{
	public:
		BvSTDError();
	};

	class BvSystemError
	{
	public:
		BvSystemError();
	};

	class BvCustomError
	{
	public:
		BvCustomError(u32 errorCode, const char* pFormat, ...);
	};

	class BvFatalError
	{
	public:
		BvFatalError(const BvSourceInfo& sourceInfo, const char* pFormat, ...);
	};
}


#if BV_DEBUG

// Asserts a condition and throws an error with a custom message
#define BV_ASSERT(cond, msg, ...) do												\
{																					\
	if (!(cond))																	\
	{																				\
		Internal::BvAssert(#cond, BV_SOURCE_INFO, msg __VA_OPT__(,) __VA_ARGS__);	\
	}																				\
} while (false)

// Asserts a condition and throws an error with a custom message once
#define BV_ASSERT_ONCE(cond, msg, ...) do											\
{																					\
	static bool doAssert = true;													\
	if (doAssert && !(cond))														\
	{																				\
		Internal::BvAssert(#cond, BV_SOURCE_INFO, msg __VA_OPT__(,) __VA_ARGS__);	\
		doAssert = false;															\
	}																				\
} while (false)

#else

#define BV_ASSERT(cond, msg, ...)
#define BV_ASSERT_ONCE(cond, msg, ...)

#endif // #if BV_DEBUG


// Used for generating STD error information
#define BV_STD_ERROR() Internal::BvSTDError();

// Used for generating OS error information
#define BV_SYSTEM_ERROR() Internal::BvSystemError();

// Used for generating custom error information
#define BV_ERROR(errorCode, msg, ...) Internal::BvCustomError(errorCode, msg __VA_OPT__(,) __VA_ARGS__);

// Throws an error with a custom message and exits the application
#define BV_FATAL_ERROR(msg, ...) do											\
{																			\
	Internal::BvFatalError(BV_SOURCE_INFO, msg __VA_OPT__(,) __VA_ARGS__);	\
} while (false)