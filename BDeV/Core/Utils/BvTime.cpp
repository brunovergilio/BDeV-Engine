#include "BvTime.h"
#include <chrono>


i64 BvTime::GetCurrentTimestampInMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


i64 BvTime::GetCurrentTimestampInUs()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


i64 BvTime::GetCurrentUTCTimestampInMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


i64 BvTime::GetCurrentUTCTimestampInUs()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


std::tm BvTime::GetLocalTime()
{
	auto now = std::chrono::system_clock::now();
	auto nowInTimeT = std::chrono::system_clock::to_time_t(now);
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	return *std::localtime(&nowInTimeT);
#else
	std::tm result;
	return *std::localtime_r(&nowInTimeT, &result);
#endif
}