#pragma once


#include "BvCore/BvPlatform.h"
#include <type_traits>
#include "../../BvDebug.h"
#include "../../Utils/BvUtils.h"


using Atomic8	= volatile i8;
using Atomic16	= volatile i16;
using Atomic32	= volatile i32;
using Atomic64	= volatile i64;

i8 AtomicAdd(Atomic8 * pDst, i8 value);
i8 AtomicSub(Atomic8 * pDst, i8 value);
i8 AtomicExchange(Atomic8 * pDst, i8 value);
i8 AtomicCompareExchange(Atomic8 * pDst, i8 exchange, i8 comparand);
i8 AtomicAnd(Atomic8 * pDst, i8 value);
i8 AtomicOr(Atomic8 * pDst, i8 value);
i8 AtomicXor(Atomic8 * pDst, i8 value);

i16 AtomicAdd(Atomic16 * pDst, i16 value);
i16 AtomicSub(Atomic16 * pDst, i16 value);
i16 AtomicExchange(Atomic16 * pDst, i16 value);
i16 AtomicCompareExchange(Atomic16 * pDst, i16 exchange, i16 comparand);
i16 AtomicAnd(Atomic16 * pDst, i16 value);
i16 AtomicOr(Atomic16 * pDst, i16 value);
i16 AtomicXor(Atomic16 * pDst, i16 value);

i32 AtomicAdd(Atomic32 * pDst, i32 value);
i32 AtomicSub(Atomic32 * pDst, i32 value);
i32 AtomicExchange(Atomic32 * pDst, i32 value);
i32 AtomicCompareExchange(Atomic32 * pDst, i32 exchange, i32 comparand);
i32 AtomicAnd(Atomic32 * pDst, i32 value);
i32 AtomicOr(Atomic32 * pDst, i32 value);
i32 AtomicXor(Atomic32 * pDst, i32 value);

i64 AtomicAdd(Atomic64 * pDst, i64 value);
i64 AtomicSub(Atomic64 * pDst, i64 value);
i64 AtomicExchange(Atomic64 * pDst, i64 value);
i64 AtomicCompareExchange(Atomic64 * pDst, i64 exchange, i64 comparand);
i64 AtomicAnd(Atomic64 * pDst, i64 value);
i64 AtomicOr(Atomic64 * pDst, i64 value);
i64 AtomicXor(Atomic64 * pDst, i64 value);


namespace Internal
{
	template<typename Type, size_t Size>
	struct BvAtomicGuard
	{
		BvCompilerAssert(std::is_integral_v<Type> == true, "Atomic variables can only be of integral types");
		BvCompilerAssert(Size <= 8, "Atomic variables can only have up to 8 bytes of size");
		BvCompilerAssert((Size & (Size - 1)) == 0, "Atomic variables can only have sizes that are powers of 2");
		typedef void type;
	};

	template<typename Type, size_t Size>
	using BvAtomicGuardT = typename BvAtomicGuard<Type, Size>::type;

	template<typename Type, size_t Size = sizeof(Type), typename = typename BvAtomicGuardT<Type, Size>>
	class BvAtomicBase
	{
	};

#define BV_BUILD_ATOMIC_CLASS(bytes, bits)																									\
	template<typename Type>																													\
	class BvAtomicBase<Type, bytes>																											\
	{																																		\
	public:																																	\
		using AtomicType = volatile Type;																									\
																																			\
		BvAtomicBase() {}																													\
		BvAtomicBase(Type value) { Exchange(value); }																						\
		BvAtomicBase(const BvAtomicBase & rhs)																								\
			: m_Value(rhs.m_Value) {}																										\
		BvAtomicBase & operator=(const BvAtomicBase & rhs)																					\
		{																																	\
			if (this != &rhs)																												\
			{																																\
				m_Value = rhs.m_Value;																										\
			}																																\
			return *this;																													\
		}																																	\
		BvAtomicBase(BvAtomicBase && rhs)																									\
		{																																	\
			*this = std::move(rhs);																											\
		}																																	\
		BvAtomicBase & operator=(BvAtomicBase && rhs)																						\
		{																																	\
			if (this != &rhs)																												\
			{																																\
				Exchange(rhs.Load());																										\
			}																																\
			return *this;																													\
		}																																	\
		~BvAtomicBase() {}																													\
																																			\
		Type Load() { return Or(0); }																										\
		void Store(Type value) { Exchange(value); }																							\
																																			\
		Type Exchange(Type value) { return static_cast<Type>(AtomicExchange(reinterpret_cast<Atomic##bits *>(&m_Value), value)); }			\
		Type CompareExchange(Type exchange, Type comparand) 																				\
			{ return static_cast<Type>(AtomicCompareExchange(reinterpret_cast<Atomic##bits *>(&m_Value), exchange, comparand)); }			\
																																			\
		Type Add(Type value) { return static_cast<Type>(AtomicAdd(reinterpret_cast<Atomic##bits *>(&m_Value), value)); }					\
		Type Sub(Type value) { return static_cast<Type>(AtomicSub(reinterpret_cast<Atomic##bits *>(&m_Value), value)); }					\
																																			\
		Type And(Type value) { return static_cast<Type>(AtomicAnd(reinterpret_cast<Atomic##bits *>(&m_Value), value)); }					\
		Type Or(Type value) { return static_cast<Type>(AtomicOr(reinterpret_cast<Atomic##bits *>(&m_Value), value)); }						\
		Type Xor(Type value) { return static_cast<Type>(AtomicXor(reinterpret_cast<Atomic##bits *>(&m_Value), value)); }					\
																																			\
		Type operator++() { return Add(1) + 1; }																							\
		Type operator--() { return Sub(1) - 1; }																							\
		Type operator++(int) { return Add(1); }																								\
		Type operator--(int) { return Sub(1); }																								\
		operator Type() { return Load(); }																									\
		BvAtomicBase & operator+=(Type rhs)	{ Add(rhs); return *this; }																		\
		BvAtomicBase & operator-=(Type rhs)	{ Sub(rhs); return *this; }																		\
		BvAtomicBase & operator&=(Type rhs) { And(rhs); return *this; }																		\
		BvAtomicBase & operator|=(Type rhs) { Or(rhs); return *this; }																		\
		BvAtomicBase & operator^=(Type rhs) { Xor(rhs); return *this; }																		\
																																			\
	private:																																\
		AtomicType m_Value{};																												\
	};

BV_BUILD_ATOMIC_CLASS(1, 8)
BV_BUILD_ATOMIC_CLASS(2, 16)
BV_BUILD_ATOMIC_CLASS(4, 32)
BV_BUILD_ATOMIC_CLASS(8, 64)

#undef BV_BUILD_ATOMIC_CLASS
}


template<typename Type>
using BvAtomic = Internal::BvAtomicBase<Type>;


template<typename Type = i32>
class BvSpinlock
{
	BV_NOCOPY(BvSpinlock);

public:
	BvSpinlock() {}
	BvSpinlock(BvSpinlock && rhs)
		: BvSpinlock(std::move(rhs.m_Lock)) {}
	BvSpinlock & operator =(BvSpinlock && rhs)
	{
		if (this != &rhs)
		{
			m_Lock = std::move(rhs.m_Lock);
		}
		return *this;
	}
	~BvSpinlock() {}

	void Lock()
	{
		for (;;)
		{
			if (TryLock())
			{
				return;
			}
			// Modern x86 / x64 CPUs are increasing the latency of the PAUSE instruction, so I'm commenting it out for now
			// https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf
			//BV_PAUSE;
		}
	}
	bool TryLock()
	{
		if (m_Lock.Load() == static_cast<Type>(0))
		{
			if (m_Lock.CompareExchange(1, 0) == 0)
			{
				return true;
			}
		}

		return false;
	}
	void Unlock()
	{
		m_Lock.Store(0);
	}

private:
	BvAtomic<Type> m_Lock;
};