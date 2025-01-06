#include "BDeV/Core/System/Threading/BvThread.h"
#include "BDeV/Core/System/Threading/BvFiber.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Container/BvText.h"
#include "BDeV/Core/System/Memory/BvMemoryArea.h"
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

		std::swap(m_ThreadId, rhs.m_ThreadId);
		std::swap(m_hThread, rhs.m_hThread);
		std::swap(m_pTask, rhs.m_pTask);
		std::swap(m_Suspended, rhs.m_Suspended);
	}

	return *this;
}


BvThread::~BvThread()
{
	Destroy();
}


void BvThread::Start()
{
	BV_ASSERT(m_hThread != nullptr, "Thread handle is invalid");
	if (m_Suspended)
	{
		m_Suspended = false;
		ResumeThread(m_hThread);
	}
}


void BvThread::Wait()
{
	BV_ASSERT(m_hThread != nullptr, "Thread handle is invalid");
	if (!m_Suspended)
	{
		WaitForSingleObject(m_hThread, INFINITE);
	}
}


void BvThread::SetAffinityMask(u64 affinityMask) const
{
	BV_ASSERT(m_hThread != nullptr, "Thread handle is invalid");
	BV_ASSERT(affinityMask != 0, "Affinity mask must have at least one bit set");

	SetThreadAffinityMask(m_hThread, static_cast<DWORD_PTR>(affinityMask));
}


void BvThread::LockToCore(u32 coreIndex) const
{
	BV_ASSERT(m_hThread != nullptr, "Thread handle is invalid");
	BV_ASSERT(coreIndex < 64, "This implementation supports only up to 64 cores");

	SetThreadAffinityMask(m_hThread, 1ull << static_cast<DWORD_PTR>(coreIndex));
}


void BvThread::SetName(const char* pThreadName) const
{
#if (BV_COMPILER == BV_COMPILER_MSVC) && BV_COMPILER_VERSION >= 1913
	HRESULT hr = S_OK;
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pThreadName, 0, nullptr, 0);
		wchar_t* pThreadNameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pThreadName, 0, pThreadNameW, sizeNeeded);
		hr = SetThreadDescription(m_hThread, pThreadNameW);
	}

	if (FAILED(hr))
	{
		// TODO: Handle error
	}
#else
	constexpr DWORD kMSVCException = 0x406D1388;

#pragma pack(push,8)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} THREADNAME_INFO;
#pragma pack(pop)

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = pThreadName;
	info.dwThreadID = (DWORD)m_ThreadId;
	info.dwFlags = 0;

	__try
	{
		RaiseException(kMSVCException, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}


void BvThread::SetPriority(Priority priority) const
{
	constexpr i32 priorities[] =
	{
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_TIME_CRITICAL,
		THREAD_PRIORITY_BELOW_NORMAL,
		THREAD_PRIORITY_LOWEST,
	};

	SetThreadPriority(m_hThread, priorities[(i32)priority]);
}


void BvThread::Sleep(u32 miliseconds)
{
	::Sleep(miliseconds);
}


void BvThread::Yield()
{
	SwitchToThread();
}


const BvThread& BvThread::GetCurrentThread()
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
	//auto& fiber = GetThreadFiberInternal();
	//BV_ASSERT(fiber.m_hFiber == nullptr, "Fiber already converted / created");
	//fiber.m_hFiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
	//BV_ASSERT(fiber.m_hFiber != nullptr, "Couldn't convert Thread to Fiber");
}


void BvThread::ConvertFromFiber()
{
	//auto& fiber = GetThreadFiberInternal();
	//BV_ASSERT(fiber.m_hFiber != nullptr, "Thread not yet converted to Fiber");
	//BOOL result = ConvertFiberToThread();
	//BV_ASSERT(result, "Couldn't convert Fiber to Thread");
	//fiber.m_hFiber = nullptr;
}


const BvFiber& BvThread::GetThreadFiber() const
{
	return GetThreadFiberInternal();
}


bool BvThread::IsFiber() const
{
	return false;
}


void BvThread::Create()
{
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0U, ThreadEntryPoint, m_pTask, 0U, reinterpret_cast<u32*>(&m_ThreadId)));
}


void BvThread::Create(const CreateInfo& createInfo)
{
	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, createInfo.m_StackSize, ThreadEntryPoint, m_pTask,
		createInfo.m_CreateSuspended ? CREATE_SUSPENDED : 0u, reinterpret_cast<u32*>(&m_ThreadId)));
	if (createInfo.m_Priority != Priority::kAuto)
	{
		SetPriority(createInfo.m_Priority);
	}
	if (createInfo.m_AffinityMask != 0)
	{
		SetAffinityMask(createInfo.m_AffinityMask);
	}

	m_Suspended = createInfo.m_CreateSuspended;
}


void BvThread::Destroy()
{
	if (m_hThread == GetCurrentThread().m_hThread)
	{
		//auto& fiber = GetThreadFiberInternal();
		//if (fiber.m_hFiber)
		//{
		//	ConvertFromFiber();
		//}
	}

	if (m_hThread && m_pTask)
	{
		CloseHandle(m_hThread);
		BV_DELETE_ARRAY((u8*)m_pTask);
	}
}


BvFiber& GetThreadFiberInternal()
{
	static thread_local BvFiber thisFiber;

	return thisFiber;
}


u32 CALLBACK ThreadEntryPoint(void* pData)
{
	IBvTask* pDelegate = reinterpret_cast<IBvTask *>(pData);
	pDelegate->Run();

	_endthreadex(0);

	return 0;
}