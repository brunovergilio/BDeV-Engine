#pragma once


#include "BDeV/Core/BvCore.h"


class BvTime
{
public:
	static i64 GetCurrentTimestampInMs();
	static i64 GetCurrentTimestampInUs();

	static i64 GetCurrentUTCTimestampInMs();
	static i64 GetCurrentUTCTimestampInUs();

	static std::tm GetLocalTime();
};