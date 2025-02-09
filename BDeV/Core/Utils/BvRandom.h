#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvTime.h"
#include <cmath>


class BvSplitMix64
{
public:
	BvSplitMix64() = default;
	BvSplitMix64(u64 seed);

	u64 Next();
	u64 operator()();

private:
	u64 m_State = BvTime::GetCurrentTimestampInUs();
};


template<typename T>
class BvPCG
{
public:
	static_assert(std::is_same_v<T, u32> || std::is_same_v<T, u64>);
	using FT = std::conditional_t<std::is_same_v<T, u32>, f32, f64>;

	BvPCG() = default;
	BV_DEFAULTCOPYMOVE(BvPCG);
	BvPCG(u64 seed, u64 inc = 0xDEADBEEF)
		: m_State(BvSplitMix64(seed)()), m_Inc(inc) {}

	T Next()
	{
		uint64_t oldState = m_State;
		// Advance the internal state
		m_State = oldState * 6364136223846793005ULL + (m_Inc | 1);
		// Calculate the output using the old state
		T xorshifted = static_cast<T>(((oldState >> 18ull) ^ oldState) >> 27ull);
		T rot = static_cast<T>(oldState >> 59ull);
		return (xorshifted >> rot) | (xorshifted << (u64(-i64(rot)) & 31ull));
	}

	T Next(T min, T max)
	{
		constexpr FT kDelta = FT(0e-5);
		const T range = max - min + 1;

		return static_cast<T>(NextFUNorm() * (FT(range) - kDelta)) + min;
	}

	FT NextF(FT min, FT max)
	{
		return NextFUNorm() * (max - min + 1) + min;
	}

	FT NextFUNorm()
	{
		constexpr T kShift = std::is_same_v<T, u32> ? 32 - 23 : 64 - 53;
		constexpr FT oneOverMantissa = FT(1.0) / (std::is_same_v<T, u32> ? FT((1U << 23) - 1) : FT((1ULL << 53) - 1));

		return static_cast<FT>(Next() >> kShift) * oneOverMantissa;
	}

	FT NextFSNorm()
	{
		return NextFUNorm() * FT(2.0) - FT(1.0);
	}

private:
	u64 m_State = BvSplitMix64()();
	u64 m_Inc = 0;
};