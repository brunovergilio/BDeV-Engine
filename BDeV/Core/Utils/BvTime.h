#pragma once


#include "BDeV/Core/BvCore.h"


class BvTime
{
public:
	struct DateTime
	{
		i32 m_Year;
		u32 m_Month;
		u32 m_Day;
		u32 m_WeekDay;
		i32 m_Hours;
		i32 m_Minutes;
		i64 m_Seconds;
		i64 m_Milliseconds;
	};

	static i64 GetCurrentTimestampInMs();
	static i64 GetCurrentTimestampInUs();

	static i64 GetCurrentUTCTimestampInMs();
	static i64 GetCurrentUTCTimestampInUs();

	static BvTime::DateTime GetCurrentDateTime();
};