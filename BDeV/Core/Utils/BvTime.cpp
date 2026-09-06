#include "BvTime.h"
#include <chrono>


using namespace std::chrono;


i64 BvTime::GetCurrentTimestampInMs()
{
	return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}


i64 BvTime::GetCurrentTimestampInUs()
{
	return duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
}


i64 BvTime::GetCurrentUTCTimestampInMs()
{
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}


i64 BvTime::GetCurrentUTCTimestampInUs()
{
	return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}


u64 BvTime::GetClockCycles()
{
#if defined(BV_ARCH_X64)
	return __rdtsc();
#elif defined(BV_ARCH_ARM64)
	// macOS ARM64: Read the hardware performance monitoring unit directly
	// via the assembly instruction mapping or the OS provided high-res mach thread duty cycle
	return __builtin_readcyclecounter(); // Clang compiler builtin for absolute core cycles
#else
#error "Unsupported architecture"
	return 0;
#endif // defined(BV_ARCH_X64)
}


u64 BvTime::GetClockCyclesSerial()
{
#if defined(BV_ARCH_X64)
	_mm_lfence();
	auto tsc = __rdtsc();
	return tsc;
#elif defined(BV_ARCH_ARM64)
	// macOS ARM64: Read the hardware performance monitoring unit directly
	// via the assembly instruction mapping or the OS provided high-res mach thread duty cycle
	return __builtin_readcyclecounter(); // Clang compiler builtin for absolute core cycles
#else
#error "Unsupported architecture"
	return 0;
#endif // defined(BV_ARCH_X64)
}

BvTime::DateTime BvTime::GetCurrentDateTime()
{
	auto now = system_clock::now();
	auto zt = zoned_time(current_zone(), now).get_local_time();
	auto dp = floor<days>(zt);
	year_month_day ymd(dp);
	hh_mm_ss<milliseconds> dayTime(floor<milliseconds>(zt - dp));

	return BvTime::DateTime{ static_cast<i32>(ymd.year()), static_cast<u32>(ymd.month()), static_cast<u32>(ymd.day()), weekday(dp).c_encoding(),
		dayTime.hours().count(), dayTime.minutes().count(), dayTime.seconds().count(), dayTime.subseconds().count() };
}