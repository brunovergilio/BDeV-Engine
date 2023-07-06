#pragma once


#include "BDeV/BvCore.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#define BV_SUPRESS_HASH_WARNING	 \
__pragma(warning(suppress:4307)) \
__pragma(warning(suppress:4100))
#else
#define BV_SUPRESS_HASH_WARNING
#endif


BV_API u64 FNV1a64(const void* const pBytes, const size_t size);
BV_API u64 MurmurHash64A(const void* const pBytes, size_t size, u64 seed = 0);


// https://github.com/HowardHinnant/hash_append/issues/7
// https://softwareengineering.stackexchange.com/questions/63595/tea-algorithm-constant-0x9e3779b9-said-to-be-derived-from-golden-ratio-but-the/63599#63599
template <typename Type, typename... Rest>
void HashCombine(u64 & seed, const Type& v, Rest... rest)
{
	seed ^= MurmurHash64A(&v, sizeof(Type)) + 0x9e3779b97f4a7c15llu + (seed << 12) + (seed >> 4);
	(HashCombine(seed, rest), ...);
}


namespace Internal
{
	template<typename Type, size_t size, size_t done>
	constexpr Type ConstexprFNV1a64Helper(const char* const pBytes, Type currHash, Type fnvPrime)
	{
		if constexpr (done < size)
		{
			return ConstexprFNV1aHelper<Type, size, done + 1>(pBytes, (currHash ^ pBytes[done]) * fnvPrime, fnvPrime);
		}
		else
		{
			return currHash;
		}
	}


	namespace MurmurConstants
	{
		constexpr const u64 kM = 0xc6a4a7935bd1e995LLU;
		constexpr const i32 kShift = 47;
	}

	constexpr u64 MurmurXORShiftLeft(u64 k)
	{
		return k ^ (k >> MurmurConstants::kShift);
	}


	constexpr u64 MurmurGet64BitBlock(const char* pBytes, u64 offset = 0)
	{
		if (offset == 7)
		{
			return u64(pBytes[offset]) << (8 * offset);
		}
		else
		{
			return u64(pBytes[offset]) << (8 * offset) | MurmurGet64BitBlock(pBytes, offset + 1);
		}
	}

	constexpr u64 MurmurMix(const char* pBytes, u64 h, u64 offset)
	{
		// k *= m;
		// k ^= k >> shift;
		// k *= m;
		// 
		// h ^= k;
		// h *= m;
		return (h ^ (MurmurXORShiftLeft(MurmurGet64BitBlock(pBytes + offset) * MurmurConstants::kM) * MurmurConstants::kM)) * MurmurConstants::kM;
	}


	constexpr u64 MurmurRemainingBits(const char* pBytes, u64 len, u64 h)
	{
		// switch (size & 7)
		// {
		// case 7: h ^= (u64)(pData2[6]) << 48; [[fallthrough]];
		// case 6: h ^= (u64)(pData2[5]) << 40; [[fallthrough]];
		// case 5: h ^= (u64)(pData2[4]) << 32; [[fallthrough]];
		// case 4: h ^= (u64)(pData2[3]) << 24; [[fallthrough]];
		// case 3: h ^= (u64)(pData2[2]) << 16; [[fallthrough]];
		// case 2: h ^= (u64)(pData2[1]) << 8;	 [[fallthrough]];
		// case 1: h ^= (u64)(pData2[0]);		 [[fallthrough]];
		// h *= m;
		// }
		if (len > 0)
		{
			return MurmurRemainingBits(pBytes, len - 1, h ^ ((u64)pBytes[len - 1]) << ((len - 1) * 8));
		}
		else
		{
			return h * MurmurConstants::kM;
		}
	}


	constexpr u64 MurmurFinalRotation(u64 h)
	{
		// h ^= h >> shift;
		// h *= m;
		// h ^= h >> shift;
		return MurmurXORShiftLeft(MurmurXORShiftLeft(h) * MurmurConstants::kM);
	}


	constexpr u64 MurmurLoop(const char* pBytes, u64 len, u64 h, u64 offset)
	{
		if (len - offset >= 8)
		{
			return MurmurLoop(pBytes, len, MurmurMix(pBytes, h, offset), offset + 8);
		}
		else
		{
			return MurmurFinalRotation(MurmurRemainingBits(pBytes, len - offset, h));
		}
	}


	constexpr u64 ConstexprMurmurHash64AHelper(const char* pBytes, u64 len, u64 seed = 0)
	{
		return Internal::MurmurLoop(pBytes, len, seed ^ (len * Internal::MurmurConstants::kM), 0);
	}
}


constexpr size_t ConstexprStringLength(const char * pBytes)
{
	if (*pBytes)
	{
		return 1 + ConstexprStringLength(pBytes + 1);
	}

	return 0;
}


#define ConstexprFNV1a64(pBytes) BV_SUPRESS_HASH_WARNING Internal::ConstexprFNV1a64Helper<u64, ConstexprStringLength(pBytes), 0>(pBytes, 14695981039346656037ull, 1099511628211ull)
#define ConstexprMurmurHash64A(pBytes) Internal::ConstexprMurmurHash64AHelper(pBytes, ConstexprStringLength(pBytes), 0)


template<typename Type>
struct BvHash
{
	size_t operator()(const Type& value) const
	{
		return MurmurHash64A(&value, sizeof(Type));
	}
};