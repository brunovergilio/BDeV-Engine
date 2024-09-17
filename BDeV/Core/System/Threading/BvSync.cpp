#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/System/Threading/BvProcess.h"


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


void BvAdaptiveMutex::Lock()
{
	constexpr u32 kMinSpinLimit = 4;
	constexpr u32 kMaxSpinLimit = 256;

	u32 currSpinCount{};
	u32 lastSpinCount = m_SpinCount.load(std::memory_order::acquire);
	u32 maxSpinCount = std::min(lastSpinCount << 1, kMaxSpinLimit);
	while (!m_Lock.try_lock())
	{
		if (currSpinCount++ > maxSpinCount)
		{
			if (!m_Lock.try_lock())
			{
				m_Lock.lock();
			}
			break;
		}
	}

	if (currSpinCount <= kMinSpinLimit)
	{
		m_SpinCount.store(std::max(kMinSpinLimit, (lastSpinCount + currSpinCount) >> 1), std::memory_order::release);
	}
	else
	{
		m_SpinCount.store((lastSpinCount + currSpinCount) >> 1, std::memory_order::release);
	}
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
			BvProcess::YieldExecution();
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