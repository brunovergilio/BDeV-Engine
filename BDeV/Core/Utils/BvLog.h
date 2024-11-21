#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/RenderAPI/BvColor.h"


struct BvLogInfo
{
	const char* m_pChannel;
	size_t m_Level;
	size_t m_Verbosity;
	const BvSourceInfo* m_pSourceInfo;
};


class IBvLogger
{
public:
	virtual ~IBvLogger() {}
	virtual void Log(const char* pChannel, size_t level, size_t verbosity, const BvSourceInfo& sourceInfo, const char* pFormat, ...) = 0;

	BV_INLINE IBvLogger* GetNext() { return m_pNext; }
	BV_INLINE IBvLogger* GetPrev() { return m_pPrev; }
	BV_INLINE void SetNext(IBvLogger* pLogger) { m_pNext = pLogger; }
	BV_INLINE void SetPrev(IBvLogger* pLogger) { m_pPrev = pLogger; }

protected:
	IBvLogger* m_pNext = nullptr;
	IBvLogger* m_pPrev = nullptr;
};


template<typename FilterType, typename FormatterType, typename OutputType>
class BvLogger final : public IBvLogger
{
public:
	BvLogger() {}
	BvLogger(const FilterType& filter, const FormatterType& formatter, const OutputType& output)
		: m_Filter(filter), m_Formatter(formatter), m_Output(output) {}
	BvLogger(BvLogger&& rhs)
	{
		*this = std::move(rhs);
	}
	BvLogger& operator=(BvLogger&& rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_Filter, rhs.m_Filter);
			std::swap(m_Formatter, rhs.m_Formatter);
			std::swap(m_Output, rhs.m_Output);
		}

		return *this;
	}

	void Log(const char* pChannel, size_t level, size_t verbosity, const BvSourceInfo& sourceInfo, const char* pFormat, ...) override
	{
		const BvLogInfo logInfo{ pChannel, level, verbosity, &sourceInfo };
		BvColorI color = BvColorI::White;
		BvColorI backColor = BvColorI::Black;
		if (m_Filter.Filter(logInfo, color, backColor))
		{
			BvString buffer;
			va_list args;
			va_start(args, pFormat);
			m_Formatter.Format(logInfo, buffer, pFormat, args);
			va_end(args);
			m_Output.Write(buffer, color);
		}
	}

private:
	FilterType m_Filter;
	FormatterType m_Formatter;
	OutputType m_Output;
};