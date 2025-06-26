#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvTime.h"
#include <cmath>


class BvSplitMix64
{
public:
	BV_INLINE BvSplitMix64()
		: m_State(BvTime::GetCurrentUTCTimestampInUs()) {}
	BV_INLINE BvSplitMix64(u64 seed)
		: m_State(seed) {}

	BV_INLINE u64 Next()
	{
		u64 result = m_State;
		m_State = m_State + 0x9E3779B97F4A7C15ULL;
		result = (result ^ (result >> 30ULL)) * 0xBF58476D1CE4E5B9ULL;
		result = (result ^ (result >> 27ULL)) * 0x94D049BB133111EBULL;
		return result ^ (result >> 31ULL);
	}

	BV_INLINE u64 operator()()
	{
		return Next();
	}

	template<size_t N>
	BV_INLINE void Generate(u64(&states)[N])
	{
		for (auto& state : states)
		{
			state = Next();
		}
	}

private:
	u64 m_State;
};


namespace Internal
{
	template<typename FloatType, typename ValueType, typename = typename std::enable_if_t<(std::is_same_v<FloatType, f32> || std::is_same_v<FloatType, f64>)>>
	BV_INLINE FloatType NextF(ValueType value)
	{
		using UIntFromFloatType = std::conditional_t<std::is_same_v<FloatType, f32>, u32, u64>;
		using UnsignedType = std::conditional_t<(sizeof(ValueType) >= sizeof(UIntFromFloatType)), ValueType, UIntFromFloatType>;
		using SignedType = std::make_signed_t<ValueType>;

		constexpr u32 kMantissaBits = std::is_same_v<FloatType, f32> ? 23 : 52;
		constexpr u32 kValueBits = sizeof(ValueType) * CHAR_BIT;

		if constexpr (kValueBits > kMantissaBits)
		{
			value >>= (kValueBits - kMantissaBits);
		}

		constexpr FloatType kOneOverMantissa = FloatType(1.0) / FloatType((UnsignedType(1) << kMantissaBits) - 1);

		return FloatType(SignedType(value)) * kOneOverMantissa;
	}

	template<typename Type>
	constexpr Type RotL(const Type value, u32 shift)
	{
		return (value << shift) | (value >> (sizeof(Type) * CHAR_BIT - shift));
	}

	template<typename Type>
	constexpr Type RotR(const Type value, u32 shift)
	{
		return (value >> shift) | (value << (sizeof(Type) * CHAR_BIT - shift));
	}
}


// PCG
class BvRandom32
{
public:
	BvRandom32()
		: m_State(BvSplitMix64()()), m_Inc(0xDEADBEEF) {}
	BV_DEFAULTCOPYMOVE(BvRandom32);
	BvRandom32(u64 seed, u64 inc = 0xDEADBEEF)
		: m_State(seed), m_Inc(inc) {}

	BV_INLINE u32 Next()
	{
		u64 state = m_State;
		// Advance the internal state
		constexpr u64 multiplier = 6364136223846793005ULL;
		m_State = state * multiplier + (m_Inc | 1);
		// Calculate the output using the old state
		u32 xorshifted = ((state >> 18u) ^ state) >> 27u;
		u32 rot = state >> 59u;
		return (xorshifted >> (rot & 31u)) | (xorshifted << ((-i32(rot)) & 31u));
	}

	BV_INLINE u32 Next(u32 min, u32 max)
	{
		return (Next() % (max - min + 1)) + min;
	}

	template<typename FloatType, typename = typename std::enable_if_t<(std::is_same_v<FloatType, f32> || std::is_same_v<FloatType, f64>)>>
	BV_INLINE FloatType NextF()
	{
		return Internal::NextF<FloatType>(Next());
	}

	template<typename FloatType, typename = typename std::enable_if_t<(std::is_same_v<FloatType, f32> || std::is_same_v<FloatType, f64>)>>
	BV_INLINE FloatType NextF(FloatType min, FloatType max)
	{
		return Internal::NextF<FloatType>(Next()) * (max - min) + min;
	}

	BV_INLINE u32 operator()()
	{
		return Next();
	}

private:
	u64 m_State;
	u64 m_Inc;
};


// Xoroshiro256SS
class BvRandom64
{
public:
	BvRandom64()
	{
		BvSplitMix64().Generate(m_State);
	}
	BV_DEFAULTCOPYMOVE(BvRandom64);
	BvRandom64(u64 seed1, u64 seed2, u64 seed3, u64 seed4)
		: m_State{ seed1, seed2, seed3, seed4 } {}

	BV_INLINE u64 Next()
	{
		const u64 result = Internal::RotL(m_State[1] * 5, 7) * 9;
		const u64 t = m_State[1] << 17;
		m_State[2] ^= m_State[0];
		m_State[3] ^= m_State[1];
		m_State[1] ^= m_State[2];
		m_State[0] ^= m_State[3];
		m_State[2] ^= t;
		m_State[3] = Internal::RotL(m_State[3], 45);
		return result;
	}

	BV_INLINE u64 Next(u64 min, u64 max)
	{
		return (Next() % (max - min + 1)) + min;
	}

	template<typename FloatType, typename = typename std::enable_if_t<(std::is_same_v<FloatType, f32> || std::is_same_v<FloatType, f64>)>>
	BV_INLINE FloatType NextF()
	{
		return Internal::NextF<FloatType>(Next());
	}

	template<typename FloatType, typename = typename std::enable_if_t<(std::is_same_v<FloatType, f32> || std::is_same_v<FloatType, f64>)>>
	BV_INLINE FloatType NextF(FloatType min, FloatType max)
	{
		return Internal::NextF<FloatType>(Next()) * (max - min) + min;
	}

	BV_INLINE u64 operator()()
	{
		return Next();
	}

private:
	u64 m_State[4];
};