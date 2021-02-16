#include "BvCore/System/Threading/BvAtomic.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


#include <intrin.h>
#include <atomic>


i8 AtomicAdd(Atomic8 * pDst, i8 value) { return _InterlockedExchangeAdd8(reinterpret_cast<volatile char *>(pDst), value); }
i8 AtomicSub(Atomic8 * pDst, i8 value) { return _InterlockedExchangeAdd8(reinterpret_cast<volatile char *>(pDst), -value); }
i8 AtomicExchange(Atomic8 * pDst, i8 value) { return _InterlockedExchange8(reinterpret_cast<volatile char *>(pDst), value); }
i8 AtomicCompareExchange(Atomic8 * pDst, i8 exchange, i8 comparand) { return _InterlockedCompareExchange8(reinterpret_cast<volatile char *>(pDst), exchange, comparand); }
i8 AtomicAnd(Atomic8 * pDst, i8 value) { return _InterlockedAnd8(reinterpret_cast<volatile char *>(pDst), value); }
i8 AtomicOr(Atomic8 * pDst, i8 value) { return _InterlockedOr8(reinterpret_cast<volatile char *>(pDst), value); }
i8 AtomicXor(Atomic8 * pDst, i8 value) { return _InterlockedXor8(reinterpret_cast<volatile char *>(pDst), value); }

i16 AtomicAdd(Atomic16 * pDst, i16 value) { return _InterlockedExchangeAdd16(reinterpret_cast<volatile short *>(pDst), value); }
i16 AtomicSub(Atomic16 * pDst, i16 value) { return _InterlockedExchangeAdd16(reinterpret_cast<volatile short *>(pDst), -value); }
i16 AtomicExchange(Atomic16 * pDst, i16 value) { return _InterlockedExchange16(reinterpret_cast<volatile short *>(pDst), value); }
i16 AtomicCompareExchange(Atomic16 * pDst, i16 exchange, i16 comparand) { return _InterlockedCompareExchange16(reinterpret_cast<volatile short *>(pDst), exchange, comparand); }
i16 AtomicAnd(Atomic16 * pDst, i16 value) { return _InterlockedAnd16(reinterpret_cast<volatile short *>(pDst), value); }
i16 AtomicOr(Atomic16 * pDst, i16 value) { return _InterlockedOr16(reinterpret_cast<volatile short *>(pDst), value); }
i16 AtomicXor(Atomic16 * pDst, i16 value) { return _InterlockedXor16(reinterpret_cast<volatile short *>(pDst), value); }

i32 AtomicAdd(Atomic32 * pDst, i32 value) { return _InterlockedExchangeAdd(reinterpret_cast<volatile long *>(pDst), value); }
i32 AtomicSub(Atomic32 * pDst, i32 value) { return _InterlockedExchangeAdd(reinterpret_cast<volatile long *>(pDst), -value); }
i32 AtomicExchange(Atomic32 * pDst, i32 value) { return _InterlockedExchange(reinterpret_cast<volatile long *>(pDst), value); }
i32 AtomicCompareExchange(Atomic32 * pDst, i32 exchange, i32 comparand) { return _InterlockedCompareExchange(reinterpret_cast<volatile long *>(pDst), exchange, comparand); }
i32 AtomicAnd(Atomic32 * pDst, i32 value) { return _InterlockedAnd(reinterpret_cast<volatile long *>(pDst), value); }
i32 AtomicOr(Atomic32 * pDst, i32 value) { return _InterlockedOr(reinterpret_cast<volatile long *>(pDst), value); }
i32 AtomicXor(Atomic32 * pDst, i32 value) { return _InterlockedXor(reinterpret_cast<volatile long *>(pDst), value); }

i64 AtomicAdd(Atomic64 * pDst, i64 value) { return _InterlockedExchangeAdd64(reinterpret_cast<volatile long long *>(pDst), value); }
i64 AtomicSub(Atomic64 * pDst, i64 value) { return _InterlockedExchangeAdd64(reinterpret_cast<volatile long long *>(pDst), -value); }
i64 AtomicExchange(Atomic64 * pDst, i64 value) { return _InterlockedExchange64(reinterpret_cast<volatile long long *>(pDst), value); }
i64 AtomicCompareExchange(Atomic64 * pDst, i64 exchange, i64 comparand) { return _InterlockedCompareExchange64(reinterpret_cast<volatile long long *>(pDst), exchange, comparand); }
i64 AtomicAnd(Atomic64 * pDst, i64 value) { return _InterlockedAnd64(reinterpret_cast<volatile long long *>(pDst), value); }
i64 AtomicOr(Atomic64 * pDst, i64 value) { return _InterlockedOr64(reinterpret_cast<volatile long long *>(pDst), value); }
i64 AtomicXor(Atomic64 * pDst, i64 value) { return _InterlockedXor64(reinterpret_cast<volatile long long *>(pDst), value); }

#endif