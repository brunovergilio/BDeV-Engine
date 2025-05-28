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
		if (!m_Lock.exchange(true, std::memory_order::memory_order_acquire))
		{
			return;
		}

		while (m_Lock.load(std::memory_order::memory_order_relaxed))
		{
			BvCPU::Yield();
		}
	}
}


bool BvSpinlock::TryLock()
{
	return !m_Lock.load(std::memory_order::memory_order_relaxed) &&
		!m_Lock.exchange(true, std::memory_order::memory_order_acquire);
}


void BvSpinlock::Unlock()
{
	m_Lock.store(false, std::memory_order::memory_order_release);
}


BvSignal::BvSignal()
{
}


BvSignal::BvSignal(ResetType resetType, bool initiallySignaled)
	: m_ResetType(resetType), m_Signaled(initiallySignaled)
{
}


BvSignal::~BvSignal()
{
}


void BvSignal::SetResetType(ResetType resetType)
{
	m_ResetType = resetType;
}


void BvSignal::Set()
{
	{
		std::unique_lock<std::mutex> lock(m_Lock);
		m_Signaled.store(true, std::memory_order::relaxed);
	}
	m_CV.notify_one();
}


void BvSignal::Reset()
{
	std::unique_lock<std::mutex> lock(m_Lock);
	m_Signaled.store(false, std::memory_order::relaxed);
}


bool BvSignal::Wait(u32 timeout)
{
	auto ms = std::chrono::milliseconds(1);
	std::unique_lock<std::mutex> lock(m_Lock);
	while (!m_Signaled.load(std::memory_order::relaxed))
	{
		auto status = m_CV.wait_for(lock, timeout * ms);
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