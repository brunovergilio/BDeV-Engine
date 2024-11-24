#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/RenderAPI/BvColor.h"


struct BvLogInfo
{
	const BvSourceInfo& m_SourceInfo;
	const char* m_pMessage;
	const char* m_pChannel;
	u32 m_Level;
	u32 m_Verbosity;
};


class IBvLogger
{
	friend class BvLoggerController;

public:
	virtual ~IBvLogger() {}
	virtual void Log(const BvLogInfo& logInfo) = 0;

private:
	BV_INLINE IBvLogger* GetNext() { return m_pNext; }
	BV_INLINE IBvLogger* GetPrev() { return m_pPrev; }
	BV_INLINE void SetNext(IBvLogger* pLogger) { m_pNext = pLogger; }
	BV_INLINE void SetPrev(IBvLogger* pLogger) { m_pPrev = pLogger; }

private:
	IBvLogger* m_pNext = nullptr;
	IBvLogger* m_pPrev = nullptr;
};


template<typename FilterType, typename FormatterType, typename OutputType>
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
	BvString m_Buffer;
	FilterType m_Filter;
	FormatterType m_Formatter;
	OutputType m_Output;
};


class BvNoLogFilter
{
public:
	BvNoLogFilter() {}
	BvNoLogFilter(BvNoLogFilter&& rhs) noexcept
	{
		*this = std::move(rhs);
	}
	BvNoLogFilter& operator=(BvNoLogFilter&& rhs) noexcept
	{
		return *this;
	}

	bool Filter(const BvLogInfo& logInfo) const
	{
		return true;
	}
};


class BvNoLogFormatter
{
public:
	BvNoLogFormatter() {}
	BvNoLogFormatter(BvNoLogFormatter&& rhs) noexcept
	{
		*this = std::move(rhs);
	}
	BvNoLogFormatter& operator=(BvNoLogFormatter&& rhs) noexcept
	{
		return *this;
	}

	void Format(const BvLogInfo& logInfo, BvString& message) const
	{
	}
};


class BvNoLogOutput
{
public:
	BvNoLogOutput() {}
	BvNoLogOutput(BvNoLogOutput&& rhs) noexcept
	{
		*this = std::move(rhs);
	}
	BvNoLogOutput& operator=(BvNoLogOutput&& rhs) noexcept
	{
		return *this;
	}

	void Write(const BvLogInfo& logInfo, const BvString& message)
	{
	}
};


namespace Logging
{
	void RegisterLogger(IBvLogger* pLogger);
	void UnregisterLogger(IBvLogger* pLogger);
	void Dispatch(const char* pChannel, u32 level, u32 verbosity, const BvSourceInfo& sourceInfo, const char* pMessage, ...);
}


#define BV_LOG(channel, level, verbosity, message, ...) do												\
{																										\
	Logging::Dispatch(channel, level, verbosity, BV_SOURCE_INFO, message __VA_OPT__(, ) __VA_ARGS__);	\
} while (false)