#include "BvHash.h"


u64 FNV1a64(const void* const pBytes, const size_t size)
{
	constexpr u64 fnvOffsetBasis = 14695981039346656037ull;
	constexpr u64 fnvPrime = 1099511628211ull;
	const u8* const pAsByte = reinterpret_cast<const u8* const>(pBytes);

	u64 hash = fnvOffsetBasis;
	for (auto i = 0u; i < size; i++)
	{
		hash ^= pAsByte[i];
		hash *= fnvPrime;
	}

	return hash;
}


u64 MurmurHash64A(const void* const pBytes, size_t size, u64 seed)
{
	constexpr u64 m = 0xc6a4a7935bd1e995LLU;
	constexpr i32 shift = 47;

	u64 h = seed ^ (size * m);

	const u64* pData = (const u64*)pBytes;
	const u64* pEnd = (size >> 3) + pData;

	while (pData != pEnd)
	{
		u64 k = *pData++;

		k *= m;
		k ^= k >> shift;
		k *= m;

		h ^= k;
		h *= m;
	}

	const u8* pData2 = (const u8*)pData;

	switch (size & 7)
	{
	case 7: h ^= (u64)(pData2[6]) << 48; [[fallthrough]];
	case 6: h ^= (u64)(pData2[5]) << 40; [[fallthrough]];
	case 5: h ^= (u64)(pData2[4]) << 32; [[fallthrough]];
	case 4: h ^= (u64)(pData2[3]) << 24; [[fallthrough]];
	case 3: h ^= (u64)(pData2[2]) << 16; [[fallthrough]];
	case 2: h ^= (u64)(pData2[1]) << 8; [[fallthrough]];
	case 1: h ^= (u64)(pData2[0]);
		h *= m;
	};

	h ^= h >> shift;
	h *= m;
	h ^= h >> shift;

	return h;
}