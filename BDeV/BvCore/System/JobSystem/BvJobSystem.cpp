#include "BvJobSystem.h"
#include "BvCore/System/Threading/BvProcess.h"
#include "BvCore/Utils/BvEnum.h"


BvThreadLocal u32 g_ThreadIndex = 0xFFFFFFFF;



constexpr u32 g_NumFibers = 16;
constexpr u32 g_FiberStackSize = 128;


BvJobSystem JobSys;


BvJobSystem::BvJobSystem()
{
}


BvJobSystem::~BvJobSystem()
{
	BvAssertMsg(m_Active.GetValue() == 0, "The Job System hasn't been shutdown");
}


void BvJobSystem::Initialize()
{
	BvAssertMsg(m_Active.GetValue() == 0, "Job System already active");

	BvSystemInfo sysInfo;
	m_NumCores = sysInfo.GetNumCores();

	m_Active.Increment();

	m_MainThreadFiber.CreateFromThread();

	m_pThreads = new BvThread[m_NumCores];
	m_pThreadData = new ThreadLocalStorage[m_NumCores];
	m_pThreadFibers = new BvFiber[m_NumCores];
	m_pFibers = new BvFiber[g_NumFibers];
	m_pFiberStates = new BvAtomic[g_NumFibers];
	m_pJobQueues = new BvSafeQueue<BvJob *, BvSpinlock>[3];
	new(&m_pJobQueues[0]) BvSafeQueue<BvJob *, BvSpinlock>(1024);
	new(&m_pJobQueues[1]) BvSafeQueue<BvJob *, BvSpinlock>(1024);
	new(&m_pJobQueues[2]) BvSafeQueue<BvJob *, BvSpinlock>(1024);

	for (u32 i = 0; i < g_NumFibers; i++)
	{
		m_pFibers[i].Create(FiberFunc, nullptr);
	}

	for (u32 i = 0; i < m_NumCores; i++)
	{
		m_pThreads[i].Start(ThreadFunc, reinterpret_cast<void *>(i));
	}
}


void BvJobSystem::Shutdown()
{
	BvAssertMsg(m_Active.GetValue() == 1, "Job System not active yet");

	m_Active.Decrement();

	for (u32 i = 0; i < m_NumCores; i++)
	{
		m_pThreads[i].Wait();
	}

	for (u32 i = 0; i < g_NumFibers; i++)
	{
		m_pFibers[i].Destroy();
	}

	m_MainThreadFiber.DestroyFromThread();

	BvDeleteArray(m_pThreads);
	BvDeleteArray(m_pThreadData);
	BvDeleteArray(m_pThreadFibers);
	BvDeleteArray(m_pFibers);
	BvDeleteArray(m_pFiberStates);
	BvDeleteArray(m_pJobQueues);
}


void BvJobSystem::AddJob(BvJob * const pJob, BvJobCounter * const pCounter)
{
	BvAssertMsg(m_Active.GetValue() == 1, "Job System not active yet");

	if (pCounter)
	{
		pCounter->Set(1);
	}
	pJob->m_pCounter = pCounter;

	m_pJobQueues[static_cast<u32>(pJob->m_Priority)].Enqueue(pJob);
}


void BvJobSystem::AddJobs(BvJob * const pJobs, const u32 count, BvJobCounter * const pCounter)
{
	BvAssertMsg(m_Active.GetValue() == 1, "Job System not active yet");

	if (pCounter)
	{
		pCounter->Set(count);
	}
	for (u32 i = 0; i < count; i++)
	{
		pJobs[i].m_pCounter = pCounter;

		m_pJobQueues[static_cast<u32>(pJobs[i].m_Priority)].Enqueue(&pJobs[i]);
	}
}


BvJobCounter * BvJobSystem::AllocCounter()
{
	BvAssertMsg(m_Active.GetValue() == 1, "Job System not active yet");

	return new BvJobCounter();
}


void BvJobSystem::FreeCounter(BvJobCounter * & pCounter)
{
	BvAssertMsg(m_Active.GetValue() == 1, "Job System not active yet");

	BvDelete(pCounter);
}


void BvJobSystem::WaitForCounter(BvJobCounter * & pCounter)
{
	BvAssertMsg(m_Active.GetValue() == 1, "Job System not active yet");
	BvAssertMsg(pCounter != nullptr, "Job Counter was nullptr");

	if (!pCounter->IsDone())
	{
		auto & tls = m_pThreadData[g_ThreadIndex];
		if (g_ThreadIndex != UINT32_MAX && tls.currWaitCount < 4)
		{
			u32 waitIndex = tls.currWaitCount;
			tls.waitJobs[waitIndex].waitFiberIndex = tls.fiberIndex;
			tls.waitJobs[waitIndex].pCounter = pCounter;
			tls.currWaitCount++;

			m_pFiberStates[tls.fiberIndex].Store(2);
			m_pThreadFibers[g_ThreadIndex].Activate();
		}
		else
		{
			while (!pCounter->IsDone())
			{
				BvPause();
			}
		}
	}
}


void BvJobSystem::ProcessThread(const u32 index)
{
	m_pThreadData[index].threadId = m_pThreads[index].GetId();
	m_pThreadFibers[index].CreateFromThread();

	BvJob * pJob = nullptr;
	u32 fiberIndex = 0;
	while (m_Active.Load())
	{
		ProcessWaitingJobs();

		// Get a hold of a job
		if (!pJob)
		{
			if (!GetJob(pJob))
			{
				BvSleep(1);
				continue;
			}
			m_pThreadData[index].pCurrJob = pJob;
		}

		// Get a hold of a fiber
		fiberIndex = GetFreeFiber(fiberIndex);
		if (fiberIndex == UINT32_MAX)
		{
			BvPause();
			continue;
		}

		// Associate the data to the thread and switch to the fiber
		m_pThreadData[index].fiberIndex = fiberIndex;
		m_pFibers[fiberIndex].Activate();
		m_pFiberStates[fiberIndex].CompareExchange(0, 1);

		// Nullify the pointer
		pJob = nullptr;
	}

	m_pThreadFibers[index].DestroyFromThread();
}


void BvJobSystem::ProcessFiber()
{
	while (m_Active.Load())
	{
		m_pThreadData[g_ThreadIndex].pCurrJob->Execute();

		m_pThreadFibers[g_ThreadIndex].Activate();
	}
}


bool BvJobSystem::GetJob(BvJob * & pJob)
{
	return (m_pJobQueues[2].Dequeue(pJob)
		|| m_pJobQueues[1].Dequeue(pJob)
		|| m_pJobQueues[0].Dequeue(pJob));
}


u32 BvJobSystem::GetFreeFiber(const u32 currIndex)
{
	BvAtomicVal cmp = 0;
	BvAtomicVal xchg = 1;
	// Do a cyclical search
	for (u32 i = currIndex + 1; i < g_NumFibers; i++)
	{
		if (m_pFiberStates[i].Load() == cmp)
		{
			if (m_pFiberStates[i].CompareExchange(xchg, cmp) == cmp)
			{
				return i;
			}
		}
	}
	for (u32 i = 0; i <= currIndex; i++)
	{
		if (m_pFiberStates[i].Load() == cmp)
		{
			if (m_pFiberStates[i].CompareExchange(xchg, cmp) == cmp)
			{
				return i;
			}
		}
	}

	return UINT32_MAX;
}


void BvJobSystem::ProcessWaitingJobs()
{
	auto & tls = m_pThreadData[g_ThreadIndex];
	u32 readyIndex = UINT32_MAX;
	for (u32 i = 0; i < tls.currWaitCount; i++)
	{
		if (tls.waitJobs[i].pCounter->IsDone())
		{
			readyIndex = i;
			break;
		}
	}

	if (readyIndex == UINT32_MAX)
	{
		return;
	}

	tls.fiberIndex = tls.waitJobs[readyIndex].waitFiberIndex;
	m_pFiberStates[tls.fiberIndex].Store(1);

	for (u32 i = readyIndex; i < tls.currWaitCount - 1; i++)
	{
		tls.waitJobs[i] = tls.waitJobs[i + 1];
	}
	tls.currWaitCount--;

	m_pFibers[tls.fiberIndex].Activate();
}


void BvJobSystem::ThreadFunc(void * pData)
{
	g_ThreadIndex = reinterpret_cast<u32>(pData);
	JobSys.m_pThreads[g_ThreadIndex].SetAffinity(g_ThreadIndex);

	JobSys.ProcessThread(g_ThreadIndex);
}


void BvJobSystem::FiberFunc(void *)
{
	JobSys.ProcessFiber();
}