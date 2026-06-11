#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "Third Party/xxhash.h"
#include "Third Party/rapidhash.h"
#include "BDeV/Core/Utils/Third Party/constexpr-xxh3.h"


struct BvXXHash
{
	BvXXHash() = default;
	explicit BvXXHash(u64 seed)
		: m_Seed(seed) {}

	template<typename T>
	u64 operator()(const T& value) const
	{
		static_assert(std::is_trivially_copyable_v<T>);
		return XXH3_64bits_withSeed(&value, sizeof(value), m_Seed);
	}

	u64 operator()(const void* pValue, size_t length) const
	{
		return XXH3_64bits_withSeed(pValue, length, m_Seed);
	}

private:
	u64 m_Seed = 0;
};


struct BvRapidHash
{
	BvRapidHash() = default;
	explicit BvRapidHash(u64 seed)
		: m_Seed(seed) {}

	template<typename T>
	u64 operator()(const T& value) const
	{
		static_assert(std::is_trivially_copyable_v<T>);
		return rapidhash_withSeed(&value, sizeof(value), m_Seed);
	}

	u64 operator()(const void* pValue, size_t length) const
	{
		return rapidhash_withSeed(pValue, length, m_Seed);
	}

private:
	u64 m_Seed = 0;
};


template<typename T, typename Hasher = BvRapidHash>
struct BvHash
{
	u64 operator()(const T& value) const
	{
		return Hasher()(value);
	}
};


// https://github.com/HowardHinnant/hash_append/issues/7
// https://softwareengineering.stackexchange.com/questions/63595/tea-algorithm-constant-0x9e3779b9-said-to-be-derived-from-golden-ratio-but-the/63599#63599
template<typename Type, typename... Rest>
void HashCombine(u64& seed, const Type& v, Rest... rest)
{
	seed ^= BvXXHash()(&v, sizeof(Type)) + 0x9e3779b97f4a7c15llu + (seed << 12) + (seed >> 4);
	(HashCombine(seed, rest), ...);
}


template<typename Hasher, typename... Args>
u64 HashCombineSingle(const Hasher& hasher, const Args&... args)
{
	u64 seed = 0;

	auto combineFn = [&](const auto& value)
		{
			seed ^= hasher(value)
				+ 0x9e3779b97f4a7c15ull
				+ (seed << 12)
				+ (seed >> 4);
		};

	(combineFn(args), ...);

	return seed;
}