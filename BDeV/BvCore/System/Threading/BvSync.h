#pragma once


#include "BvCore/BvDefines.h"
#include "BvCore/BvDebug.h"
#include <utility>


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
using BvAtomicVal = LONG;
#endif


class BvAtomic
{
public:
	BvAtomic();
	~BvAtomic();

	BvAtomic(const BvAtomicVal value);

	// Non-Atomic methods
	void SetValue(const BvAtomicVal value);
	const BvAtomicVal GetValue() const;

	// Atomic methods
	const BvAtomicVal Load();
	void Store(const BvAtomicVal value);
	const BvAtomicVal CompareExchange(const BvAtomicVal xchg, const BvAtomicVal cmp);
	const BvAtomicVal Increment();
	const BvAtomicVal Decrement();
	const BvAtomicVal Add(const BvAtomicVal value);
	const BvAtomicVal Sub(const BvAtomicVal value);

private:
	volatile BvAtomicVal m_Value = 0U;
};


class BvSpinlock
{
public:
	BvSpinlock();
	~BvSpinlock();

	BvSpinlock(BvSpinlock && rhs);
	BvSpinlock & operator =(BvSpinlock && rhs);
	BvSpinlock(const BvSpinlock & rhs) = delete;
	BvSpinlock & operator =(const BvSpinlock & rhs) = delete;

	void Lock();
	bool TryLock();
	void Unlock();

private:
	BvAtomic m_Lock;
};


class BvMutex
{
public:
	BvMutex();
	~BvMutex();

	BvMutex(BvMutex && rhs);
	BvMutex & operator =(BvMutex && rhs);
	BvMutex(const BvMutex & rhs) = delete;
	BvMutex & operator =(const BvMutex & rhs) = delete;

	void Lock();
	bool TryLock();
	void Unlock();

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	CRITICAL_SECTION m_Mutex{};
#endif
};


class BvNoLock
{
public:
	BvNoLock() {}
	~BvNoLock() {}

	BvNoLock(BvNoLock && rhs) = default;
	BvNoLock & operator =(BvNoLock && rhs) = default;
	BvNoLock(const BvNoLock & rhs) = delete;
	BvNoLock & operator =(const BvNoLock & rhs) = delete;

	BV_INLINE void Lock() {}
	BV_INLINE bool TryLock() { return true; }
	BV_INLINE void Unlock() {}
};


template<class LockType>
class BvScopedLock
{
public:
	BvScopedLock(LockType & lock) : m_Lock(lock) { m_Lock.Lock(); }
	~BvScopedLock() { m_Lock.Unlock(); }

	BvScopedLock() = delete;
	BvScopedLock(BvScopedLock && rhs) = delete;
	BvScopedLock & operator =(BvScopedLock && rhs) = delete;
	BvScopedLock(const BvScopedLock & rhs) = delete;
	BvScopedLock & operator =(const BvScopedLock & rhs) = delete;

private:
	LockType & m_Lock;
};


constexpr u32 kInfinite = INFINITE;


class BvEvent
{
public:
	BvEvent();
	explicit BvEvent(const bool manualReset, const bool signaled = false);
	~BvEvent();

	BvEvent(BvEvent && rhs);
	BvEvent & operator =(BvEvent && rhs);
	BvEvent(const BvEvent & rhs) = delete;
	BvEvent & operator =(const BvEvent & rhs) = delete;

	void Set();
	void Reset();
	bool Wait(const u32 timeout = kInfinite);

private:
	void Destroy();

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HANDLE m_hEvent = nullptr;
#endif
};