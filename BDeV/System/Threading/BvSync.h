#pragma once


#include "BDeV/Utils/BvUtils.h"
#include <atomic>


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <Windows.h>
#endif


class BV_API BvMutex
{
	BV_NOCOPY(BvMutex);

public:
	BvMutex();
	~BvMutex();

	BvMutex(BvMutex&& rhs) noexcept;
	BvMutex& operator =(BvMutex&& rhs) noexcept;

	void Lock();
	bool TryLock();
	void Unlock();

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	CRITICAL_SECTION m_Mutex{};
#endif
};


class BV_API BvSpinlock
{
	BV_NOCOPYMOVE(BvSpinlock);

public:
	BvSpinlock();
	~BvSpinlock();

	void Lock();
	bool TryLock();
	void Unlock();

private:
	std::atomic<bool> m_Lock;
};


class BV_API BvSignal
{
	BV_NOCOPY(BvSignal);

public:
	BvSignal();
	explicit BvSignal(const bool manualReset, const bool signaled = false);
	~BvSignal();

	BvSignal(BvSignal&& rhs) noexcept;
	BvSignal& operator =(BvSignal&& rhs) noexcept;

	void Set();
	void Reset();
	bool Wait(const u32 timeout = kU32Max);

private:
	void Destroy();

private:
	void* m_hEvent = nullptr;
};


class BV_API BvNoLock
{
	BV_NOCOPY(BvNoLock);

public:
	BvNoLock() {}
	~BvNoLock() {}

	BvNoLock(BvNoLock&& rhs) = default;
	BvNoLock& operator =(BvNoLock&& rhs) = default;

	BV_INLINE void Lock() {}
	BV_INLINE bool TryLock() { return true; }
	BV_INLINE void Unlock() {}
};


template<class LockType>
class BV_API BvScopedLock
{
	BV_NOCOPYMOVE(BvScopedLock);

public:
	BvScopedLock(LockType& lock) : m_Lock(lock) { m_Lock.Lock(); }
	~BvScopedLock() { m_Lock.Unlock(); }

	BvScopedLock() = delete;

private:
	LockType& m_Lock;
};