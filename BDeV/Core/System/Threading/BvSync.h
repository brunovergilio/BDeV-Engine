#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include <mutex>
#include <atomic>
#include <condition_variable>


class BvMutex
{
	BV_NOCOPY(BvMutex);

public:
	BvMutex();
	~BvMutex();

	void Lock();
	bool TryLock();
	void Unlock();

private:
	std::mutex m_Mutex{};
};


class BvAdaptiveMutex
{
	BV_NOCOPY(BvAdaptiveMutex);

public:
	BvAdaptiveMutex();
	~BvAdaptiveMutex();

	void Lock();
	bool TryLock();
	void Unlock();

private:
	std::mutex m_Lock;
	std::atomic<u32> m_SpinCount;
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


class BvNoLock
{
	BV_NOCOPY(BvNoLock);

public:
	BvNoLock() {}
	~BvNoLock() {}

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


class BvSignal
{
	BV_NOCOPY(BvSignal);

public:
	enum class ResetType : u8
	{
		kAuto,
		kManual,
	};

	BvSignal();
	explicit BvSignal(ResetType resetType, bool initiallySignaled = false);
	~BvSignal();

	void SetResetType(ResetType resetType);

	void Set();
	void Reset();
	bool Wait(u32 timeout = kU32Max);

private:
	std::condition_variable m_CV;
	std::mutex m_Lock;
	std::atomic<bool> m_Signaled;
	ResetType m_ResetType = ResetType::kAuto;
};