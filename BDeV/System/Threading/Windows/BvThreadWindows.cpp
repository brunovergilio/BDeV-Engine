#include "BDeV/System/Threading/BvThread.h"
#include "BDeV/System/Threading/BvFiber.h"
#include "BDeV/System/Debug/BvDebug.h"
#include <utility>
#include <process.h>
#include <Windows.h>


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

		std::swap(m_ThreadId, rhs.m_ThreadId);
		std::swap(m_hThread, rhs.m_hThread);
		std::swap(m_pDelegate, rhs.m_pDelegate);
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


void BvThread::SetAffinityMask(const u64 affinityMask) const
{
	BvAssert(m_hThread != nullptr, "Thread handle is invalid");

	SetThreadAffinityMask(m_hThread, static_cast<DWORD_PTR>(affinityMask));
}


void BvThread::LockToCore(u32 coreIndex) const
{
	BvAssert(m_hThread != nullptr, "Thread handle is invalid");
	BvAssert(coreIndex < 64, "This implementation supports only 64 cores");

	SetThreadAffinityMask(m_hThread, 1ull << static_cast<DWORD_PTR>(coreIndex));
}


void BvThread::SetName(const char* pThreadName) const
{
	BvAssert(m_hThread != nullptr, "Thread handle is invalid");

	constexpr u32 kMaxThreadNameLength = 64;
	wchar_t threadName[kMaxThreadNameLength];
	mbstate_t state{};
	mbsrtowcs(threadName, &pThreadName, kMaxThreadNameLength - 1, &state);

	auto hr = SetThreadDescription(m_hThread, threadName);
	if (FAILED(hr))
	{
		// TODO: Handle error
	}
}


void BvThread::SetPriority(ThreadPriority priority) const
{
	constexpr i32 priorities[] =
	{
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_TIME_CRITICAL,
		THREAD_PRIORITY_BELOW_NORMAL,
		THREAD_PRIORITY_LOWEST,
	};

	SetThreadPriority(m_hThread, priorities[(i32)priority]);
}


void BvThread::Sleep(const u32 miliseconds)
{
	::Sleep(miliseconds);
}


void BvThread::YieldExecution()
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


u32 BvThread::GetCurrentProcessor()
{
	return GetCurrentProcessorNumber();
}


void BvThread::ConvertToFiber()
{
	auto& fiber = GetThreadFiberInternal();
	BvAssert(fiber.m_pFiber == nullptr, "Fiber already converted / created");
	fiber.m_pFiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
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


bool BvThread::IsFiber() const
{
	return GetThreadFiber().GetFiber() != nullptr;
}


void BvThread::Create(const u32 stackSize)
{
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0U, ThreadEntryPoint, m_pDelegate, 0U, reinterpret_cast<u32*>(&m_ThreadId)));
}


void BvThread::Destroy()
{
	if (m_hThread == GetCurrentThread().m_hThread)
	{
		auto& fiber = GetThreadFiberInternal();
		if (fiber.m_pFiber)
		{
			ConvertFromFiber();
		}
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