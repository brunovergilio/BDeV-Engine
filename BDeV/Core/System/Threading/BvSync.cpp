#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/System/Process/BvProcess.h"
#include <algorithm>


BvMutex::BvMutex()
{
}


BvMutex::~BvMutex()
{
}


void BvMutex::Lock()
{
	m_Mutex.lock();
}


bool BvMutex::TryLock()
{
	return m_Mutex.try_lock();
}


void BvMutex::Unlock()
{
	m_Mutex.unlock();
}


constexpr u32 kDefaultSpinCount = 100;


BvAdaptiveMutex::BvAdaptiveMutex()
	: m_SpinCount(kDefaultSpinCount)
{
}


BvAdaptiveMutex::~BvAdaptiveMutex()
{
}


//void BvAdaptiveMutex::Lock()
//{
//	constexpr u32 kMinSpinLimit = 8;
//	constexpr u32 kMaxSpinLimit = 512;
//	constexpr u32 kSpintCountBeforeYield = kMaxSpinLimit >> 2;
//
//	u32 currSpinCount{};
//	const u32 lastSpinCount = m_SpinCount.load(std::memory_order::acquire);
//	const u32 maxSpinCount = std::min(lastSpinCount << 1, kMaxSpinLimit);
//	while (!m_Lock.try_lock())
//	{
//		if (currSpinCount++ > maxSpinCount)
//		{
//			m_Lock.lock();
//			break;
//		}
//
//		if (currSpinCount > kSpintCountBeforeYield)
//		{
//			BvCPU::Yield();
//		}
//	}
//
//	// Adaptive smoothing
//	// Dynamically adjust smoothing rate: faster when spinning longer
//	const i32 alphaShift = 4 - i32(currSpinCount > lastSpinCount); // ~1/8 or 1/16
//	const i32 delta = static_cast<i32>(currSpinCount) - static_cast<i32>(lastSpinCount);
//	BV_ASSERT(i32(lastSpinCount) >= (delta >> alphaShift), "must be greater");
//	const u32 newSpinCount = u32(i32(lastSpinCount) + (delta >> alphaShift));
//
//	m_SpinCount.store(std::clamp(newSpinCount, kMinSpinLimit, kMaxSpinLimit), std::memory_order::release);
//}


void BvAdaptiveMutex::Lock()
{
	constexpr u32 kMinSpinLimit = 16;
	constexpr u32 kMaxSpinLimit = 512;

	//const u32 lastSpinCount = m_SpinCount.load(std::memory_order_acquire);
	const u32 maxSpinCount = std::min(m_SpinCount << 1, kMaxSpinLimit);
	u32 currSpinCount{};

	DebugIncLockRequest();
	while (!m_Lock.try_lock())
	{
		if (currSpinCount++ > maxSpinCount)
		{
			DebugIncLockCall();
			m_Lock.lock();
			break;
		}
	}

	//m_SpinCount.store((m_SpinCount.load(std::memory_order_relaxed) + currSpinCount) >> 1, std::memory_order::release);
	m_SpinCount = std::clamp((m_SpinCount + currSpinCount) >> 1, kMinSpinLimit, kMaxSpinLimit);
}


bool BvAdaptiveMutex::TryLock()
{
	return m_Lock.try_lock();
}


void BvAdaptiveMutex::Unlock()
{
	m_Lock.unlock();
}


BvSpinlock::BvSpinlock()
{
}


BvSpinlock::~BvSpinlock()
{
}


void BvSpinlock::Lock()
{
	for (;;)
	{
		if (!m_Lock.exchange(true, std::memory_order::acquire))
		{
			return;
		}

		while (m_Lock.load(std::memory_order::relaxed))
		{
			BvCPU::Yield();
		}
	}
}


bool BvSpinlock::TryLock()
{
	return !m_Lock.load(std::memory_order::relaxed) &&
		!m_Lock.exchange(true, std::memory_order::acquire);
}


void BvSpinlock::Unlock()
{
	m_Lock.store(false, std::memory_order::release);
}


BvWaitEvent::BvWaitEvent()
{
}


BvWaitEvent::BvWaitEvent(ResetType resetType, bool initiallySignaled)
	: m_ResetType(resetType), m_Signaled(initiallySignaled)
{
}


BvWaitEvent::~BvWaitEvent()
{
}


void BvWaitEvent::SetResetType(ResetType resetType)
{
	m_ResetType = resetType;
}


void BvWaitEvent::Set()
{
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		m_Signaled.store(true, std::memory_order::relaxed);
	}
	if (m_ResetType == ResetType::kAuto)
	{
		m_CV.notify_one();
	}
	else
	{
		m_CV.notify_all();
	}
}


void BvWaitEvent::Reset()
{
	std::unique_lock<std::mutex> lock(m_Lock);
	m_Signaled.store(false, std::memory_order::relaxed);
}


bool BvWaitEvent::Wait(u32 timeout)
{
	std::unique_lock<std::mutex> lock(m_Lock);
	// Loop to prevent spurious wake-ups
	while (!m_Signaled.load(std::memory_order::relaxed))
	{
		auto status = m_CV.wait_for(lock, std::chrono::milliseconds(timeout));
		if (status == std::cv_status::timeout)
		{
			return false;
		}
	}
	if (m_ResetType == ResetType::kAuto)
	{
		m_Signaled.store(false, std::memory_order::relaxed);
	}

	return true;
}


BvManualResetEvent::BvManualResetEvent(bool signaled)
{
	if (signaled)
	{
		m_Event.test_and_set(std::memory_order::relaxed);
	}
	else
	{
		m_Event.clear(std::memory_order::relaxed);
	}
}


void BvManualResetEvent::Set()
{
	m_Event.test_and_set(std::memory_order::acq_rel);
	m_Event.notify_all();
}


void BvManualResetEvent::Reset()
{
	m_Event.clear(std::memory_order::release);
}


void BvManualResetEvent::Wait()
{
	while (!m_Event.test(std::memory_order::memory_order_acquire))
	{
		m_Event.wait(false, std::memory_order::acquire);
	}
}


BvAutoResetEvent::BvAutoResetEvent(bool signaled)
{
	m_Event.store(i32(signaled), std::memory_order::relaxed);
}


void BvAutoResetEvent::Set()
{
	// Notify only in case it wasn't already set
	if (m_Event.exchange(1, std::memory_order::release) == 0)
	{
		m_Event.notify_one();
	}
}


void BvAutoResetEvent::Wait()
{
	i32 exp = 1;
	while (!m_Event.compare_exchange_strong(exp, 0, std::memory_order::acquire, std::memory_order::acquire))
	{
		exp = 1;
		m_Event.wait(0, std::memory_order::acquire);
	}
}