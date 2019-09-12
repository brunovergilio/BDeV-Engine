#include "BvCore/Utils/Hash.h"


u32 FNV1a32(const void * const pBytes, const u32 count)
{
	constexpr u32 fnvOffsetBasis = 2166136261u;
	constexpr u32 fnvPrime = 16777619u;

	const u8 * const pAsByte = reinterpret_cast<const u8 * const>(pBytes);

	u32 hash = fnvOffsetBasis;
	for (auto i = 0u; i < count; i++)
	{
		hash ^= pAsByte[i];
		hash *= fnvPrime;
	}

	return hash;
}


u64 FNV1a64(const void * const pBytes, const u32 count)
{
	constexpr u64 fnvOffsetBasis = 14695981039346656037ull;
	constexpr u64 fnvPrime = 1099511628211ull;

	const u8 * const pAsByte = reinterpret_cast<const u8 * const>(pBytes);
	u64 hash = fnvOffsetBasis;
	for (auto i = 0u; i < count; i++)
	{
		hash ^= pAsByte[i];
		hash *= fnvPrime;
	}

	return hash;
}