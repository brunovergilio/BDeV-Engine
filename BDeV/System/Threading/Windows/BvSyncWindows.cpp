#include "BvSyncWindows.h"


BvMutex::BvMutex()
{
	InitializeCriticalSection(&m_Mutex);
}


BvMutex::~BvMutex()
{
	DeleteCriticalSection(&m_Mutex);
}


BvMutex::BvMutex(BvMutex && rhs) noexcept
{
	*this = std::move(rhs);
}


BvMutex & BvMutex::operator =(BvMutex && rhs) noexcept
{
	if (this != &rhs)
	{
		DeleteCriticalSection(&m_Mutex);
		m_Mutex = rhs.m_Mutex;
		rhs.m_Mutex = {};
	}

	return *this;
}


void BvMutex::Lock()
{
	EnterCriticalSection(&m_Mutex);
}


bool BvMutex::TryLock()
{
	return TryEnterCriticalSection(&m_Mutex);
}


void BvMutex::Unlock()
{
	LeaveCriticalSection(&m_Mutex);
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
			YieldProcessor();
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
		Destroy();
		m_hEvent = rhs.m_hEvent;
		rhs.m_hEvent = nullptr;
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