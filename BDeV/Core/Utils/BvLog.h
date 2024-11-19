#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/RenderAPI/BvColor.h"


struct BvLogInfo
{
	size_t m_Channel;
	size_t m_Type;
	size_t m_Verbosity;
	const BvSourceInfo* m_pSourceInfo;
};


class IBvLogger
{
public:
	virtual ~IBvLogger() {}
	virtual void Log(size_t channel, size_t type, size_t verbosity, const BvSourceInfo& sourceInfo, const char* pFormat, ...) = 0;
};


template<typename FilterType, typename FormatType, typename OutputType>
class BvLogger final : public IBvLogger
{
public:
	void Log(size_t channel, size_t type, size_t verbosity, const BvSourceInfo& sourceInfo, const char* pFormat, ...)
	{
		const BvLogInfo logInfo{ channel, type, verbosity, &sourceInfo };
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
	FormatType m_Formatter;
	OutputType m_Output;
};