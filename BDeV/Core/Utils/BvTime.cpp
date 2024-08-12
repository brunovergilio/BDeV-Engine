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