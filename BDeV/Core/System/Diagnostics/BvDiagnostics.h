#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/RenderAPI/BvColor.h"


class BvConsole
{
	static void ConsoleHelper();

public:
	// Default white text and black background printf
	static i32 PrintF(const char* pFormat, ...);
	// Custom color text and black background printf
	static i32 PrintF(const BvColorI& textColor, const char* pFormat, ...);
	// Custom color text and custom color background printf
	static i32 PrintF(const BvColorI& textColor, const BvColorI& backGroundColor, const char* pFormat, ...);

	// =====================================================
	// Template variations
	// I'm keeping these but they can bloat the code a lot

	// Default white text and black background printf
	template<typename... Args>
	static void Print(const char* pStr, Args&&... args)
	{
		ConsoleHelper();

		printf(pStr, args...);
	}

	// Custom color text and black background printf
	template<typename... Args>
	static void Print(const BvColorI& textColor, const char* pStr, Args&&... args)
	{
		ConsoleHelper();

		printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
		printf(pStr, args...);
		printf("\033[0m");
	}

	// Custom color text and custom color background printf
	template<typename... Args>
	static void Print(const BvColorI& textColor, const BvColorI& backGroundColor, const char* pStr, Args&&... args)
	{
		ConsoleHelper();

		printf("\033[38;2;%d;%d;%dm", textColor.m_Red, textColor.m_Green, textColor.m_Blue);
		printf("\033[48;2;%d;%d;%dm", backGroundColor.m_Red, backGroundColor.m_Green, backGroundColor.m_Blue);
		printf(pStr, args...);
		printf("\033[0m");
	}
};


class BvDebug
{
public:
	// Same as printf but to the debug window (if one exists)
	static i32 PrintF(const char* pFormat, ...);

	// To force code breaks when debugging
	static void Break();
};


namespace Internal
{
	// This class should only be called by macros
	class BvAssert
	{
	public:
		BvAssert(const char* pCondition, const BvSourceInfo& sourceInfo, const char* pFormat, ...);
	};

	// This class should only be called by macros
	class BvFatalError
	{
	public:
		BvFatalError(const BvSourceInfo& sourceInfo, const char* pFormat, ...);
	};
}


class BvError
{
	BV_NOCOPYMOVE(BvError);

public:
	BvError();

	void FromSTD();
	void FromSystem();
	void MakeCustom(u32 errorCode, const char* pFormat, ...);

	u32 GetErrorCode() const;
	const char* GetErrorMessage() const;
};


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


// Throws an error with a custom message and exits the application
#define BV_FATAL_ERROR(msg, ...) do											\
{																			\
	Internal::BvFatalError(BV_SOURCE_INFO, msg __VA_OPT__(,) __VA_ARGS__);	\
} while (false)