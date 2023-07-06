#include "BDeV/System/Threading/BvSync.h"
#include "../BvProcess.h"
#include <Windows.h>


BvMutex::BvMutex()
{
}


BvMutex::~BvMutex()
{
}


BvMutex::BvMutex(BvMutex && rhs) noexcept
{
}


BvMutex & BvMutex::operator =(BvMutex && rhs) noexcept
{
	return *this;
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
			YieldProcessorExecution();
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


BvSignal::BvSignal(const bool manualReset, const bool signaled)
	: m_hEvent(CreateEvent(nullptr, manualReset, signaled, nullptr))
{
	BvAssert(m_hEvent != nullptr, "Invalid event handle");
}


BvSignal::~BvSignal()
{
	Destroy();
}


BvSignal::BvSignal(BvSignal && rhs) noexcept
{
	*this = std::move(rhs);
}


BvSignal & BvSignal::operator =(BvSignal && rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_hEvent, rhs.m_hEvent);
	}

	return *this;
}


void BvSignal::Set()
{
	SetEvent(m_hEvent);
}


void BvSignal::Reset()
{
	ResetEvent(m_hEvent);
}


bool BvSignal::Wait(const u32 timeout)
{
	return WaitForSingleObject(m_hEvent, timeout) == WAIT_OBJECT_0;
}


void BvSignal::Destroy()
{
	if (m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = nullptr;
	}
}