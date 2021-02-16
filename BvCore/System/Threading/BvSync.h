#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/BvDebug.h"
#include "BvCore/Utils/BvUtils.h"
#include <utility>


class BvMutex
{
	BV_NOCOPY(BvMutex);

public:
	BvMutex();
	~BvMutex();

	BvMutex(BvMutex && rhs);
	BvMutex & operator =(BvMutex && rhs);

	void Lock();
	bool TryLock();
	void Unlock();

private:
	MutexHandle m_Mutex{};
};


class BvNoLock
{
	BV_NOCOPY(BvNoLock);

public:
	BvNoLock() {}
	~BvNoLock() {}

	BvNoLock(BvNoLock && rhs) = default;
	BvNoLock & operator =(BvNoLock && rhs) = default;

	BV_INLINE void Lock() {}
	BV_INLINE bool TryLock() { return true; }
	BV_INLINE void Unlock() {}
};


template<class LockType>
class BvScopedLock
{
	BV_NOCOPYMOVE(BvScopedLock);

public:
	BvScopedLock(LockType & lock) : m_Lock(lock) { m_Lock.Lock(); }
	~BvScopedLock() { m_Lock.Unlock(); }

	BvScopedLock() = delete;

private:
	LockType & m_Lock;
};


class BvEvent
{
	BV_NOCOPY(BvEvent);

public:
	BvEvent();
	explicit BvEvent(const bool manualReset, const bool signaled = false);
	~BvEvent();

	BvEvent(BvEvent && rhs);
	BvEvent & operator =(BvEvent && rhs);

	void Set();
	void Reset();
	bool Wait(const u32 timeout = kInfinite);

private:
	void Destroy();

private:
	EventHandle m_hEvent{};
};