#include "BvFBJS.h"
#include "BDeV/System/Threading/BvThread.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/System/Threading/BvProcess.h"
#include <bit>


struct JobList
{
	JobDecl* m_pJobs;
	u32 m_JobCount;
	std::atomic<u32> m_CurrJobIndex;
};


class BvFBJSWorker
{
public:
	BvFBJSWorker(BvFBJS* pJobSystem, u32 jobPoolSize)
		: m_pJobSystem(pJobSystem), m_JobPoolSize(jobPoolSize),
		m_Signal(false), m_WorkerThread([this]()
			{
				WorkerFunction();
			}) {}

	~BvFBJSWorker() {}

	void SubmitJobs(JobDecl* pJobs, u32 jobCount)
	{
		m_Lock.Lock();

		while (m_LastJobIndex.load(std::memory_order::relaxed) - m_FirstJobIndex.load(std::memory_order::relaxed) >= m_JobPoolSize)
		{
			BvProcess::YieldExecution();
		}

		u32 currIndex = m_LastJobIndex.load(std::memory_order::relaxed) & (m_JobPoolSize - 1);
		for (auto i = 0u; i < jobCount; ++i)
		{
			m_ppJobs[currIndex] = &pJobs[currIndex];
		}
		m_LastJobIndex.fetch_add(jobCount, std::memory_order::relaxed);

		m_Lock.Unlock();

		m_Signal.Set();
	}

private:
	void WorkerFunction()
	{
		while (m_pJobSystem->IsActive())
		{
			m_Signal.Wait();

			Process();
		}
	}

	void Process()
	{
		constexpr auto kMaxLocalJobListSize = 32u;
		JobDecl* pLocalJobList[kMaxLocalJobListSize];
		FiberData* pFiberData = nullptr;
		auto currIndex = kU32Max;
		while (m_pJobSystem->IsActive())
		{
			JobDecl* pJob = nullptr;
			while (!pJob)
			{
				auto firstJobIndex = m_FirstJobIndex.load(std::memory_order::relaxed);
				auto lastJobIndex = m_LastJobIndex.load(std::memory_order::relaxed);
				if (firstJobIndex == lastJobIndex)
				{
					break;
				}

				currIndex = firstJobIndex & (m_JobPoolSize - 1);
				if (m_ppJobs[currIndex]->m_FetchLock.fetch_add(1, std::memory_order::seq_cst) == 0)
				{
					pJob = m_ppJobs[currIndex];
				}
				m_FirstJobIndex.fetch_add(1, std::memory_order::relaxed);
			}
			
			if (pJob)
			{
				if (!pFiberData)
				{
					// Get Fiber
				}
			}
			else
			{
				// Check suspended jobs
			}

			if (pFiberData)
			{
			}
		}
	}

private:
	BvFBJS* m_pJobSystem = nullptr;
	BvThread m_WorkerThread;
	BvAdaptiveMutex m_Lock;
	BvSignal m_Signal;
	JobDecl** m_ppJobs = nullptr;
	u32 m_JobPoolSize = 0;
	std::atomic<u32> m_FirstJobIndex;
	std::atomic<u32> m_LastJobIndex;
};


void BvFBJS::Initialize()
{

}


void BvFBJS::Shutdown()
{

}


JobDecl* BvFBJS::AcquireJobs(u32 jobCount)
{

}


void BvFBJS::Submit(JobDecl* pJobs, u32 jobCount)
{

}


bool BvFBJS::IsActive() const
{
	return m_Active.load(std::memory_order::seq_cst);
}


BvFBJS::BvFBJS()
{
}


BvFBJS::~BvFBJS()
{
}


class BvFBJSInstance
{
public:
	static BvFBJS m_FBJS;
};


BvFBJS* g_pJobSys = &BvFBJSInstance::m_FBJS;