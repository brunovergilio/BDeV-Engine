#include "BvCore/System/Threading/Win32/BvSyncWin32.h"


BvMutex::BvMutex()
{
	InitializeCriticalSection(&m_Mutex);
}


BvMutex::~BvMutex()
{
	DeleteCriticalSection(&m_Mutex);
}


BvMutex::BvMutex(BvMutex && rhs)
{
	*this = std::move(rhs);
}


BvMutex & BvMutex::operator =(BvMutex && rhs)
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


BvEvent::BvEvent()
{
}


BvEvent::BvEvent(const bool manualReset, const bool signaled)
	: m_hEvent(CreateEvent(nullptr, manualReset, signaled, nullptr))
{
	BvAssert(m_hEvent != nullptr);
}


BvEvent::~BvEvent()
{
	Destroy();
}


BvEvent::BvEvent(BvEvent && rhs)
{
	*this = std::move(rhs);
}


BvEvent & BvEvent::operator =(BvEvent && rhs)
{
	if (this != &rhs)
	{
		Destroy();
		m_hEvent = rhs.m_hEvent;
		rhs.m_hEvent = nullptr;
	}

	return *this;
}


void BvEvent::Set()
{
	SetEvent(m_hEvent);
}


void BvEvent::Reset()
{
	ResetEvent(m_hEvent);
}


bool BvEvent::Wait(const u32 timeout)
{
	return WaitForSingleObject(m_hEvent, timeout) == WAIT_OBJECT_0;
}


void BvEvent::Destroy()
{
	if (m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = nullptr;
	}
}