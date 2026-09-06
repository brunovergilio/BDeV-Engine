#include "BDeV/Core/System/Threading/BvThread.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Utils/BvUTF.h"
#include <utility>
#include <process.h>


BvThread::BvThread()
{
}


BvThread::BvThread(BvThread && rhs) noexcept
	: m_ThreadId(rhs.m_ThreadId), m_hThread(rhs.m_hThread), m_pTaskData(rhs.m_pTaskData), m_IsRunning(rhs.m_IsRunning)
{
	rhs.m_hThread = kNullOSThreadHandle;
	rhs.m_ThreadId = 0;
	rhs.m_pTaskData = nullptr;
	rhs.m_IsRunning = 0;
}


BvThread& BvThread::operator=(BvThread&& rhs) noexcept
{
	if (this != &rhs)
	{
		Destroy();

		m_hThread = rhs.m_hThread;
		m_ThreadId = rhs.m_ThreadId;
		m_pTaskData = rhs.m_pTaskData;
		m_IsRunning = rhs.m_IsRunning;

		rhs.m_hThread = kNullOSThreadHandle;
		rhs.m_ThreadId = 0;
		rhs.m_pTaskData = nullptr;
		rhs.m_IsRunning = 0;
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
	if (!m_IsRunning && m_pTaskData)
	{
		m_IsRunning = true;
		::ResumeThread(m_hThread);
	}
}


void BvThread::Join()
{
	if (m_IsRunning && m_pTaskData)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
		m_IsRunning = false;
	}
}


void BvThread::SetAffinity(u32 logicalProcessorIndex) const
{
	BV_ASSERT(m_hThread != nullptr, "Thread handle is invalid");

	WORD groupCount = ::GetActiveProcessorGroupCount();
	if (groupCount <= 1)
	{
		BV_ASSERT(logicalProcessorIndex < 64, "This implementation supports only up to 64 cores");

		::SetThreadAffinityMask(m_hThread, 1ull << static_cast<DWORD_PTR>(logicalProcessorIndex));
	}
	else
	{
		// Find which group the logicalProcessorIndex falls into
		for (WORD group = 0; group < groupCount; ++group)
		{
			DWORD groupSize = ::GetActiveProcessorCount(group);
			if (logicalProcessorIndex < groupSize)
			{
				GROUP_AFFINITY affinity{};
				affinity.Group = group;
				affinity.Mask = 1ull << logicalProcessorIndex;

				BOOL result = ::SetThreadGroupAffinity(m_hThread, &affinity, nullptr);
				BV_ASSERT(result != 0, "Failed to set thread group affinity");
				
				break;
			}
			logicalProcessorIndex -= groupSize;
		}
	}
}


void BvThread::SetAffinity(u32 logicalProcessorIndexCount, const u32* pLogicalProcessorIndices) const
{
	BV_ASSERT(m_hThread != nullptr, "Thread handle is invalid");

	WORD groupCount = ::GetActiveProcessorGroupCount();
	if (groupCount <= 1)
	{
		DWORD_PTR affinityMask = 0;
		for (auto i = 0; i < logicalProcessorIndexCount; ++i)
		{
			affinityMask |= 1ull << pLogicalProcessorIndices[i];
		}

		::SetThreadAffinityMask(m_hThread, affinityMask);
	}
	else
	{
		WORD selectedGroup = kU16Max;
		KAFFINITY affinityMask = 0;

		u32 groupEndIndex = 0;
		for (WORD group = 0; group < groupCount; ++group)
		{
			DWORD groupSize = ::GetActiveProcessorCount(group);
			u32 groupStartIndex = groupEndIndex;
			groupEndIndex = groupStartIndex + groupSize;

			GROUP_AFFINITY groupAffinity{};
			for (auto i = 0; i < logicalProcessorIndexCount; ++i)
			{
				u32 index = pLogicalProcessorIndices[i];
				if (index >= groupStartIndex && index < groupEndIndex)
				{
					if (selectedGroup == kU16Max)
					{
						selectedGroup = group;
					}
					else
					{
						BV_ASSERT(selectedGroup == group, "All processor indices must belong to the same group");
					}

					affinityMask |= 1ull << (index - groupStartIndex);
				}
			}

			if (selectedGroup != kU16Max)
			{
				break;
			}
		}

		BV_ASSERT(selectedGroup != kU16Max, "No valid group found for provided indices");

		GROUP_AFFINITY groupAffinity{};
		groupAffinity.Group = selectedGroup;
		groupAffinity.Mask = affinityMask;

		BOOL result = ::SetThreadGroupAffinity(m_hThread, &groupAffinity, nullptr);
		BV_ASSERT(result != 0, "Failed to set thread group affinity");
	}
}


void BvThread::SetName(const char* pThreadName) const
{
#if BV_COMPILER_MSVC && BV_COMPILER_VERSION >= 1913
	HRESULT hr = S_OK;
	{
		std::string_view sv(pThreadName);
		auto length = BvUTFCharTraits::LengthFor<wchar_t>(sv.begin(), sv.end()) + 1;
		wchar_t* pThreadNameW = (wchar_t*)BV_STACK_ALLOC(length * sizeof(wchar_t));
		BvUTFCharTraits::GetStr(sv.begin(), sv.end(), pThreadNameW, pThreadNameW + length);
		pThreadNameW[length - 1] = 0;

		hr = ::SetThreadDescription(m_hThread, pThreadNameW);

		if (FAILED(hr))
		{
			BV_SYS_ERROR(hr);
		}
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
		::RaiseException(kMSVCException, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
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

	::SetThreadPriority(m_hThread, priorities[(i32)priority]);
}


void BvThread::Sleep(u32 miliseconds)
{
	::Sleep(miliseconds);
}


void BvThread::Yield()
{
	::SwitchToThread();
}


const BvThread& BvThread::GetCurrentThread()
{
	static thread_local BvThread currentThread = []()
		{
			BvThread thisThread;
			thisThread.m_hThread = ::GetCurrentThread();
			thisThread.m_ThreadId = ::GetCurrentThreadId();

			return thisThread;
		}();

	return currentThread;
}


u32 BvThread::GetCurrentProcessor()
{
	return ::GetCurrentProcessorNumber();
}


void BvThread::Create(const CreateInfo& createInfo)
{
	m_IsRunning = !createInfo.m_CreateSuspended;
	u32 id;
	m_hThread = reinterpret_cast<HANDLE>(::_beginthreadex(nullptr, createInfo.m_StackSize, BvThread::ThreadEntryPoint, m_pTaskData,
		m_IsRunning ? 0u : CREATE_SUSPENDED, &id));
	if (m_hThread == kNullOSThreadHandle)
	{
		BV_SYS_FATAL();
	}
	m_ThreadId = id;
	if (createInfo.m_Priority != Priority::kAuto)
	{
		SetPriority(createInfo.m_Priority);
	}
	if (createInfo.m_LogicalProcessorIndexCount > 0)
	{
		SetAffinity(createInfo.m_LogicalProcessorIndexCount, createInfo.m_pLogicalProcessorIndices);
	}
	if (createInfo.m_pName)
	{
		SetName(createInfo.m_pName);
	}
}


void BvThread::Destroy()
{
	if (m_hThread && m_pTaskData)
	{
		Join();
		::CloseHandle(m_hThread);
		auto pArena = m_pTaskData->m_pArena;
		pArena->Free(m_pTaskData);
		m_pTaskData = nullptr;
		m_hThread = kNullOSThreadHandle;
	}
}


u32 WINAPI BvThread::ThreadEntryPoint(void* pData)
{
	TaskData* pTaskData = reinterpret_cast<TaskData*>(pData);
	pTaskData->m_pInvokeFn(pTaskData->m_pObj);

	::_endthreadex(0);

	return 0;
}