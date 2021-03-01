#pragma once


#include "BvCore/BvCore.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#define BV_SUPRESS_HASH_WARNING	 \
__pragma(warning(suppress:4307)) \
__pragma(warning(suppress:4100))
#else
#define BV_SUPRESS_HASH_WARNING
#endif


template<typename Type, Type fnvOffsetBasis, Type fnvPrime>
Type FNV1a(const void * const pBytes, const size_t size)
{
	const u8 * const pAsByte = reinterpret_cast<const u8 * const>(pBytes);

	Type hash = fnvOffsetBasis;
	for (auto i = 0u; i < size; i++)
	{
		hash ^= pAsByte[i];
		hash *= fnvPrime;
	}

	return hash;
}


BV_INLINE u32 FNV1a32(const void * const pBytes, const size_t size)
{
	return FNV1a<u32, 2166136261u, 16777619u>(pBytes, size);
}


BV_INLINE u64 FNV1a64(const void * const pBytes, const size_t size)
{
	return FNV1a<u64, 14695981039346656037ull, 1099511628211ull>(pBytes, size);
}


template<typename Type>
struct Hash
{
	u64 operator()(const Type & val)
	{
		return FNV1a64(&val, sizeof(Type));
	}
};


template <typename Type, typename... Rest>
void HashCombine(u64 & seed, const Type& v, Rest... rest)
{
	seed ^= ::Hash<Type>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	(HashCombine(seed, rest), ...);
}



template<typename Type, size_t size, size_t done>
constexpr Type ConstexprFNV1aHelper(const char * const pBytes, Type currHash, Type fnvPrime)
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


constexpr size_t ConstexprStringLength(const char * pBytes)
{
	if (*pBytes)
	{
		return 1 + ConstexprStringLength(pBytes + 1);
	}

	return 0;
}


#define ConstexprFNV1a32(bytes) BV_SUPRESS_HASH_WARNING ConstexprFNV1aHelper<u32, ConstexprStringLength(bytes), 0>(bytes, 2166136261u, 16777619u)
#define ConstexprFNV1a64(bytes) BV_SUPRESS_HASH_WARNING ConstexprFNV1aHelper<u64, ConstexprStringLength(bytes), 0>(bytes, 14695981039346656037ull, 1099511628211ull)

/*
 * MurmurHash64A (C) Austin Appleby
 */
//u64 MurmurHash64A(const void* const pBytes, size_t size, u64 seed = 0)
//{
//	constexpr u64 m = 0xc6a4a7935bd1e995LLU;
//	constexpr i32 shift = 47;
//
//	u64 h = seed ^ (size * m);
//
//	const u64* pData = (const u64*)pBytes;
//	const u64* pEnd = (size >> 3) + pData;
//
//	while (pData != pEnd)
//	{
//		u64 k = *pData++;
//
//		k *= m;
//		k ^= k >> shift;
//		k *= m;
//
//		h ^= k;
//		h *= m;
//	}
//
//	const u8* pData2 = (const u8*)pData;
//
//	switch (size & 7)
//	{
//	case 7: h ^= (u64)(pData2[6]) << 48; [[fallthrough]];
//	case 6: h ^= (u64)(pData2[5]) << 40; [[fallthrough]];
//	case 5: h ^= (u64)(pData2[4]) << 32; [[fallthrough]];
//	case 4: h ^= (u64)(pData2[3]) << 24; [[fallthrough]];
//	case 3: h ^= (u64)(pData2[2]) << 16; [[fallthrough]];
//	case 2: h ^= (u64)(pData2[1]) << 8;	 [[fallthrough]];
//	case 1: h ^= (u64)(pData2[0]);		 [[fallthrough]];
//		h *= m;
//	};
//
//	h ^= h >> shift;
//	h *= m;
//	h ^= h >> shift;
//
//	return h;
//}