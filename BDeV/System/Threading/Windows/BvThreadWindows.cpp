#include "BvThreadWindows.h"
#include "BDeV/System/Threading/Windows/BvFiberWindows.h"
#include "BDeV/System/Debug/BvDebug.h"
#include <utility>
#include <process.h>


BvFiber& GetThreadFiberInternal();
u32 CALLBACK ThreadEntryPoint(void* pData);


BvThread::BvThread()
{
}


BvThread::BvThread(BvThread && rhs) noexcept
{
	*this = std::move(rhs);
}


BvThread & BvThread::operator =(BvThread && rhs) noexcept
{
	if (this != &rhs)
	{
		Destroy();

		m_ThreadId = rhs.m_ThreadId;
		m_hThread = rhs.m_hThread;
		m_pDelegate = rhs.m_pDelegate;

		rhs.m_ThreadId = 0;
		rhs.m_hThread = nullptr;
		rhs.m_pDelegate = nullptr;
	}

	return *this;
}


BvThread::~BvThread()
{
	Destroy();
}


void BvThread::Wait()
{
	BvAssert(m_hThread != nullptr, "Thread handle is invalid");

	WaitForSingleObject(m_hThread, INFINITE);
}


void BvThread::SetAffinity(const u32 affinityMask) const
{
	BvAssert(m_hThread != nullptr, "Thread handle is invalid");

	SetThreadAffinityMask(m_hThread, 1ull << static_cast<DWORD_PTR>(affinityMask));
}


void BvThread::Sleep(const u32 miliseconds)
{
	::Sleep(miliseconds);
}


void BvThread::Yield()
{
	SwitchToThread();
}


const BvThread & BvThread::GetCurrentThread()
{
	static thread_local BvThread thisThread;
	if (!thisThread.m_hThread)
	{
		thisThread.m_hThread = ::GetCurrentThread();
		thisThread.m_ThreadId = GetThreadId(thisThread.m_hThread);
	}

	return thisThread;
}


const u32 BvThread::GetCurrentProcessor()
{
	return GetCurrentProcessorNumber();
}


void BvThread::ConvertToFiber(void* pData)
{
	auto& fiber = GetThreadFiberInternal();
	BvAssert(fiber.m_pFiber == nullptr, "Fiber already converted / created");
	fiber.m_pFiber = ConvertThreadToFiberEx(pData, FIBER_FLAG_FLOAT_SWITCH);
	BvAssert(fiber.m_pFiber != nullptr, "Couldn't convert Thread to Fiber");
}


void BvThread::ConvertFromFiber()
{
	auto& fiber = GetThreadFiberInternal();
	BvAssert(fiber.m_pFiber != nullptr, "Thread not yet converted to Fiber");
	BOOL result = ConvertFiberToThread();
	BvAssert(result, "Couldn't convert Fiber to Thread");
	fiber.m_pFiber = nullptr;
}


const BvFiber& BvThread::GetThreadFiber() const
{
	return GetThreadFiberInternal();
}


const bool BvThread::IsFiber() const
{
	return GetThreadFiber().GetFiber() != nullptr;
}


void BvThread::Create(const u32 stackSize)
{
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0U, ThreadEntryPoint, m_pDelegate, 0U, reinterpret_cast<u32*>(&m_ThreadId)));
}


void BvThread::Destroy()
{
	auto& fiber = GetThreadFiberInternal();
	if (fiber.m_pFiber)
	{
		ConvertFromFiber();
	}

	if (m_hThread && m_pDelegate)
	{
		CloseHandle(m_hThread);

		delete m_pDelegate;
	}
}


BvFiber& GetThreadFiberInternal()
{
	static thread_local BvFiber thisFiber;

	return thisFiber;
}


u32 ThreadEntryPoint(void* pData)
{
	BvDelegateBase * pDelegate = reinterpret_cast<BvDelegateBase *>(pData);
	pDelegate->Invoke();

	return 0;
}