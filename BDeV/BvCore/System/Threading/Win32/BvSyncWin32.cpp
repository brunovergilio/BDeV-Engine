#include "BvCore/System/Threading/BvSync.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


BvAtomic::BvAtomic()
{
}


BvAtomic::~BvAtomic()
{
}


BvAtomic::BvAtomic(const BvAtomicVal value)
	: m_Value(value)
{
}


// Non-Atomic methods
void BvAtomic::SetValue(const BvAtomicVal value)
{
	m_Value = value;
}


const BvAtomicVal BvAtomic::GetValue() const
{
	return m_Value;
}


// Atomic methods
const BvAtomicVal BvAtomic::Load() { return InterlockedCompareExchange(&m_Value, 0, 0); }
void  BvAtomic::Store(const BvAtomicVal value) { InterlockedExchange(&m_Value, value); }
const BvAtomicVal BvAtomic::CompareExchange(const BvAtomicVal xchg, const BvAtomicVal cmp) { return InterlockedCompareExchange(&m_Value, xchg, cmp); }
const BvAtomicVal BvAtomic::Increment() { return InterlockedIncrement(&m_Value); }
const BvAtomicVal BvAtomic::Decrement() { return InterlockedDecrement(&m_Value); }
const BvAtomicVal BvAtomic::Add(const BvAtomicVal value) { return InterlockedAdd(&m_Value, value); }
const BvAtomicVal BvAtomic::Sub(const BvAtomicVal value) { return InterlockedAdd(&m_Value, -value); }


BvSpinlock::BvSpinlock()
{
}


BvSpinlock::~BvSpinlock()
{
}


BvSpinlock::BvSpinlock(BvSpinlock && rhs)
{
	*this = std::move(rhs);
}


BvSpinlock & BvSpinlock::operator =(BvSpinlock && rhs)
{
	if (this != &rhs)
	{
		m_Lock = rhs.m_Lock;
		rhs.m_Lock.SetValue(0);
	}

	return *this;
}


void BvSpinlock::Lock()
{
	for (;;)
	{
		if (m_Lock.Load() == 0)
		{
			if (m_Lock.CompareExchange(1, 0) == 0)
			{
				break;
			}
		}
		BvPause();
	}
}


bool BvSpinlock::TryLock()
{
	if (m_Lock.Load() == 0)
	{
		return m_Lock.CompareExchange(1, 0) == 0;
	}

	return false;
}


void BvSpinlock::Unlock()
{
	m_Lock.Store(0);
}


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


#endif