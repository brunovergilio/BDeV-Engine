#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvColor.h"


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

	// To force code breaks when debugging (should be used with macros)
	static void Break();

	// Asserts a condition (should be used with macros)
	static void Assert(const char* pCondition, const std::source_location& sourceInfo, const char* pFormat, ...);
};


#if BV_DEBUG

// Asserts a condition and throws an error with a custom message
#define BV_ASSERT(cond, msg, ...) do															\
{																								\
	if (!(cond))																				\
	{																							\
		BvDebug::Assert(#cond, std::source_location::current(), msg __VA_OPT__(,) __VA_ARGS__);	\
	}																							\
} while (0)

// Asserts a condition and throws an error with a custom message once
#define BV_ASSERT_ONCE(cond, msg, ...) do														\
{																								\
	static bool doAssert = true;																\
	if (doAssert && !(cond))																	\
	{																							\
		BvDebug::Assert(#cond, std::source_location::current(), msg __VA_OPT__(,) __VA_ARGS__);	\
		doAssert = false;																		\
	}																							\
} while (0)

#else

#define BV_ASSERT(cond, msg, ...)
#define BV_ASSERT_ONCE(cond, msg, ...)

#endif // #if BV_DEBUG


struct BvLogInfo
{
	const std::source_location& m_SourceInfo;
	const char* m_pMessage;
	const char* m_pChannel;
	u32 m_Level;
	u32 m_Verbosity;
};


class IBvLogger
{
public:
	virtual ~IBvLogger() {}
	virtual void Log(const BvLogInfo& logInfo) = 0;

	friend class BvLoggerManager;

private:
	BV_INLINE IBvLogger* GetNext() { return m_pNext; }
	BV_INLINE IBvLogger* GetPrev() { return m_pPrev; }
	BV_INLINE void SetNext(IBvLogger* pLogger) { m_pNext = pLogger; }
	BV_INLINE void SetPrev(IBvLogger* pLogger) { m_pPrev = pLogger; }

private:
	IBvLogger* m_pNext = nullptr;
	IBvLogger* m_pPrev = nullptr;
};


template<typename MessageType, typename FilterType, typename FormatterType, typename OutputType>
class BvLogger final : public IBvLogger
{
public:
	BvLogger() {}
	BvLogger(const FilterType& filter, const FormatterType& formatter, const OutputType& output) noexcept
		: m_Filter(filter), m_Formatter(formatter), m_Output(output) {}
	BvLogger(BvLogger&& rhs) noexcept
	{
		*this = std::move(rhs);
	}
	BvLogger& operator=(BvLogger&& rhs)
	{
		if (this != &rhs)
		{
			m_Filter = std::move(rhs.m_Filter);
			m_Formatter = std::move(rhs.m_Formatter);
			m_Output = std::move(rhs.m_Output);
		}

		return *this;
	}

	void Log(const BvLogInfo& logInfo) override
	{
		if (m_Filter.Filter(logInfo))
		{
			m_Formatter.Format(logInfo, m_Buffer);
			m_Output.Write(logInfo, m_Buffer);
		}
	}

private:
	MessageType m_Buffer;
	FilterType m_Filter;
	FormatterType m_Formatter;
	OutputType m_Output;
};


class BvNoLogFilter
{
public:
	BvNoLogFilter() {}
	BvNoLogFilter(BvNoLogFilter&& rhs) noexcept = default;
	BvNoLogFilter& operator=(BvNoLogFilter&& rhs) noexcept = default;

	BV_INLINE bool Filter(const BvLogInfo& logInfo) const { return true; }
};


template<typename MessageType>
class BvNoLogFormatter
{
public:
	BvNoLogFormatter() {}
	BvNoLogFormatter(BvNoLogFormatter&& rhs) noexcept = default;
	BvNoLogFormatter& operator=(BvNoLogFormatter&& rhs) noexcept = default;

	BV_INLINE void Format(const BvLogInfo& logInfo, MessageType& message) const {}
};


template<typename MessageType>
class BvNoLogOutput
{
public:
	BvNoLogOutput() {}
	BvNoLogOutput(BvNoLogOutput&& rhs) noexcept = default;
	BvNoLogOutput& operator=(BvNoLogOutput&& rhs) noexcept = default;

	BV_INLINE void Write(const BvLogInfo& logInfo, const MessageType& message) {}
};


class BvLoggerManager
{
public:
	BvLoggerManager() = default;
	~BvLoggerManager() = default;

	void RegisterLogger(IBvLogger* pLogger);
	void UnregisterLogger(IBvLogger* pLogger);
	void Dispatch(const BvLogInfo& logInfo);

private:
	IBvLogger* m_pLoggerList = nullptr;
};


namespace Logging
{
	// This method sets the current logger manager to be used (not thread-safe). Since the engine project is only built
	// as a static library, for dll projects that use it, consider having an exported method in the other project to
	// allow setting the logger manager instead of calling this function directly from inside the dll, as it
	// will have another instance of the underlying static variables.
	void SetLoggerManager(BvLoggerManager* pLoggerManager);

	// Used by macros to dispatch information to multiple loggers
	void Dispatch(const char* pChannel, u32 level, u32 verbosity, const std::source_location& sourceInfo, const char* pMessage, ...);
}


#define BV_LOG(channel, level, verbosity, message, ...) do												\
{																										\
	Logging::Dispatch(channel, level, verbosity, std::source_location::current(), message __VA_OPT__(, ) __VA_ARGS__);	\
} while (0)


namespace Internal
{
	void ReportFatalError(const char* pMessage);

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	const char* GetWindowsErrorMessage(u32 errorCode = kU32Max);
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)
}


#define BV_TRACE(channel, message, ...) BV_LOG(channel, 0, 0, message __VA_OPT__(, ) __VA_ARGS__)
#define BV_INFO(channel, message, ...) BV_LOG(channel, 1, 0, message __VA_OPT__(, ) __VA_ARGS__)
#define BV_WARNING(channel, message, ...) BV_LOG(channel, 2, 0, message __VA_OPT__(, ) __VA_ARGS__)
#define BV_ERROR(channel, message, ...) BV_LOG(channel, 3, 0, message __VA_OPT__(, ) __VA_ARGS__)
#define BV_FATAL(channel, message, ...) do						\
{																\
	BV_LOG(channel, 4, 0, message __VA_OPT__(, ) __VA_ARGS__);	\
	Internal::ReportFatalError(message);						\
} while (0)


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#define BV_SYS_ERROR(errorCode) BV_ERROR("System", Internal::GetWindowsErrorMessage(errorCode))
#define BV_SYS_FATAL(errorCode) BV_FATAL("System", Internal::GetWindowsErrorMessage(errorCode))
#else
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)