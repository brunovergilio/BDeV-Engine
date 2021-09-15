#pragma once


#include "BvCore/Utils/BvUtils.h"
#include <windows.h>
#include <atomic>


class BvMutex
{
	BV_NOCOPY(BvMutex);

public:
	BvMutex();
	~BvMutex();

	BvMutex(BvMutex&& rhs);
	BvMutex& operator =(BvMutex&& rhs);

	void Lock();
	bool TryLock();
	void Unlock();

private:
	CRITICAL_SECTION m_Mutex{};
};


class BvSpinlock
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


class BvEvent
{
	BV_NOCOPY(BvEvent);

public:
	BvEvent();
	explicit BvEvent(const bool manualReset, const bool signaled = false);
	~BvEvent();

	BvEvent(BvEvent&& rhs);
	BvEvent& operator =(BvEvent&& rhs);

	void Set();
	void Reset();
	bool Wait(const u32 timeout = kU32Max);

private:
	void Destroy();

private:
	HANDLE m_hEvent = nullptr;
};


class BvNoLock
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
class BvScopedLock
{
	BV_NOCOPYMOVE(BvScopedLock);

public:
	BvScopedLock(LockType& lock) : m_Lock(lock) { m_Lock.Lock(); }
	~BvScopedLock() { m_Lock.Unlock(); }

	BvScopedLock() = delete;

private:
	LockType& m_Lock;
};