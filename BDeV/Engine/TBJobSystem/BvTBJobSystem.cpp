#include "BvTBJobSystem.h"


class BvTBJobSystemWorker
{
	enum class WorkerResult : u8
	{
		kDone,
		kInProgress,
		kStalled
	};

public:
	BvTBJobSystemWorker(BvTBJobSystem* pJobSystem, const u32 jobListPoolSize, const u32 coreIndex);
	BvTBJobSystemWorker(const u32 jobListPoolSize);
	~BvTBJobSystemWorker();

	void AddJobList(BvTBJobList* pJobList);
	void Stop();

private:
	void Process();
	WorkerResult RunJobs(const u32 jobListIndex, const bool runAll = false);

private:
	BvTBJobSystem* m_pJobSystem = nullptr;
	BvVector<BvTBJobList*> m_JobLists;
	std::atomic<u32> m_LastJobIndex;
	u32 m_CurrJobListIndex = 0;
	BvThread m_Thread;
	BvSignal m_WorkSignal;
	BvSpinlock m_Lock;
};


BvTBJobSystemWorker::BvTBJobSystemWorker(BvTBJobSystem* pJobSystem, const u32 jobListPoolSize, const u32 coreIndex)
	: m_pJobSystem(pJobSystem), m_WorkSignal(BvSignal(false)), m_JobLists(jobListPoolSize),
	m_Thread(BvThread([this]()
		{
			while (m_pJobSystem->IsActive())
			{
				if (m_LastJobIndex == m_CurrJobListIndex)
				{
					m_WorkSignal.Wait();
				}

				Process();
			}
		}))
{
	m_Thread.SetAffinity(coreIndex);
}


BvTBJobSystemWorker::BvTBJobSystemWorker(const u32 jobListPoolSize)
	: m_JobLists(jobListPoolSize)
{
}


BvTBJobSystemWorker::~BvTBJobSystemWorker()
{
}


void BvTBJobSystemWorker::AddJobList(BvTBJobList* pJobList)
{
	{
		BvScopedLock<BvSpinlock> lock(m_Lock);
		auto lastJobIndex = m_LastJobIndex.load();
		m_JobLists[lastJobIndex] = pJobList;
		m_LastJobIndex = (lastJobIndex + 1u) % (u32)m_JobLists.Size();
	}
	
	// If m_pJobSystem is not nullptr, then this is a worker thread
	if (m_pJobSystem)
	{
		m_WorkSignal.Set();
	}
	// Otherwise, just process it here
	else
	{
		Process();
	}
}


void BvTBJobSystemWorker::Stop()
{
	// If m_pJobSystem is not nullptr, then this is a worker thread
	if (m_pJobSystem)
	{
		m_WorkSignal.Set();
		m_Thread.Wait();
	}
}


void BvTBJobSystemWorker::Process()
{
	u32 lastJobIndex = m_LastJobIndex.load();
	u32 jobListIndex = 0;
	// Keep track of the last job list that this worker failed processing
	u32 lastSkippedJobListIndex = kU32Max;
	JobListPriority priority = JobListPriority::kLow;
	u32 jobListSize = (u32)m_JobLists.Size();
	while (m_CurrJobListIndex != lastJobIndex)
	{
		// Start with the first available index
		jobListIndex = m_CurrJobListIndex;
		priority = JobListPriority::kLow;
		if (lastSkippedJobListIndex == kU32Max)
		{
			for (auto i = m_CurrJobListIndex; i != lastJobIndex; i = (i + 1) % jobListSize)
			{
				if (m_JobLists[i]->GetPriority() > priority
					&& !m_JobLists[i]->IsWaitingForDependency())
				{
					priority = m_JobLists[i]->GetPriority();
					jobListIndex = i;
					if (priority == JobListPriority::kHigh)
					{
						break;
					}
				}
			}
		}
		else
		{
			jobListIndex = lastSkippedJobListIndex;
			for (auto i = m_CurrJobListIndex; i != lastJobIndex; i = (i + 1) % jobListSize)
			{
				if (i != lastSkippedJobListIndex && m_JobLists[i]->GetPriority() >= priority
					&& !m_JobLists[i]->IsWaitingForDependency())
				{
					priority = m_JobLists[i]->GetPriority();
					jobListIndex = i;
					if (priority == JobListPriority::kHigh)
					{
						break;
					}
				}
			}
		}

		auto result = RunJobs(jobListIndex, priority == JobListPriority::kHigh);
		if (result == WorkerResult::kDone)
		{
			// Remove the JobList from the pool
			for (auto i = jobListIndex; i > m_CurrJobListIndex; i--)
			{
				m_JobLists[i] = m_JobLists[i - 1];
			}
			m_CurrJobListIndex = (m_CurrJobListIndex + 1) % jobListSize;
			lastSkippedJobListIndex = kU32Max;
		}
		else if (result == WorkerResult::kStalled)
		{
			if (lastSkippedJobListIndex == jobListIndex)
			{
				BvThread::YieldExecution();
			}
			lastSkippedJobListIndex = jobListIndex;
		}
		else// if (result == WorkerResult::kInProgress)
		{
			lastSkippedJobListIndex = kU32Max;
		}
	}
}


BvTBJobSystemWorker::WorkerResult BvTBJobSystemWorker::RunJobs(const u32 jobListIndex, const bool runAll)
{
	auto pCurrJobList = m_JobLists[jobListIndex];
	auto jobsDone = 0u;
	auto currJobListJobIndex = 0u;

	do
	{
		currJobListJobIndex = pCurrJobList->m_CurrJob.load();
		// If currjobListJobIndex is equal to the amount of jobs, that may not mean all jobs are done,
		// but it means every job is at least being processed
		if (currJobListJobIndex == pCurrJobList->m_JobCount)
		{
			return WorkerResult::kDone;
		}
		// Do an early-out if there are any sync points or dependencies to be waited on
		// Can sometimes avoid the atomic lock below
		if (pCurrJobList->m_Jobs[currJobListJobIndex].second != nullptr)
		{
			if (pCurrJobList->m_Jobs[currJobListJobIndex].second == BvTBJobList::kSyncPoint)
			{
				if (currJobListJobIndex > pCurrJobList->m_JobsDone.load())
				{
					return WorkerResult::kStalled;
				}
			}
			else
			{
				if (!pCurrJobList->m_Jobs[currJobListJobIndex].second->IsDone())
				{
					return WorkerResult::kStalled;
				}
			}
		}

		if (pCurrJobList->m_Lock.TryLock())
		{
			// If there's no data, then this is a job
			// Move the job pointer forward
			if (pCurrJobList->m_Jobs[currJobListJobIndex].second == nullptr)
			{
				currJobListJobIndex = pCurrJobList->m_CurrJob++;
			}
			else
			{
				// If it is a sync point
				if (pCurrJobList->m_Jobs[currJobListJobIndex].second == BvTBJobList::kSyncPoint)
				{
					// Check and make sure every previous job is done
					// and if it is, move the job pointer forward, and mark this job as done
					// otherwise, quit the function
					if (currJobListJobIndex > pCurrJobList->m_JobsDone.load())
					{
						pCurrJobList->m_Lock.Unlock();
						return WorkerResult::kStalled;
					}
					else
					{
						currJobListJobIndex = pCurrJobList->m_CurrJob++;
						pCurrJobList->m_JobsDone++;
					}
				}
				// If it is a dependency
				else
				{
					// Check if the other joblist is done,
					// and if it is, move the job pointer forward, and mark this job as done
					// otherwise, quit the function
					if (!pCurrJobList->m_Jobs[currJobListJobIndex].second->IsDone())
					{
						pCurrJobList->m_Lock.Unlock();
						return WorkerResult::kStalled;
					}
					else
					{
						currJobListJobIndex = pCurrJobList->m_CurrJob++;
						pCurrJobList->m_JobsDone++;
					}
				}
			}
			pCurrJobList->m_Lock.Unlock();
		}
		else
		{
			return WorkerResult::kStalled;
		}

		// If it's a regular job, run it
		if (pCurrJobList->m_Jobs[currJobListJobIndex].second == nullptr)
		{
			pCurrJobList->m_Jobs[currJobListJobIndex].first.Run();

			// Mark the job as done (and add one for the check below)
			jobsDone = pCurrJobList->m_JobsDone.fetch_add(1) + 1;
		}
	} while (runAll);

	if (jobsDone >= pCurrJobList->m_JobCount)
	{
		return WorkerResult::kDone;
	}
	else
	{
		return WorkerResult::kInProgress;
	}
}


BvTBJobSystem::BvTBJobSystem()
{
}


BvTBJobSystem::~BvTBJobSystem()
{
	Shutdown();
}


void BvTBJobSystem::Initialize(const JobSystemDesc desc)
{
	BvAssert(m_IsActive == false, "Job System has not been initialized yet");
	m_IsActive = true;

	m_JobListPool.Resize(desc.m_JobListPoolSize);
	for (auto& jobList : m_JobListPool)
	{
		new(&jobList) BvTBJobList(this, JobListPriority::kNormal);
	}

	auto numWorkerThreads = 0;
	auto numThreadsPerCore = 1;

	const auto& sysInfo = GetSystemInfo();
	BvAssert(desc.m_NumThreads <= sysInfo.m_NumLogicalProcessors, "Not enough logical processors for the worker threads");

	switch (desc.m_Parallelism)
	{
	case JobSystemDesc::Parallelism::kUseThreadCount:
		numWorkerThreads = sysInfo.m_NumLogicalProcessors;
		numThreadsPerCore = sysInfo.m_NumCores % sysInfo.m_NumLogicalProcessors;
		break;
	case JobSystemDesc::Parallelism::kUseCoreCount:
		numWorkerThreads = sysInfo.m_NumCores;
		//numThreadsPerCore = 1;
		break;
	case JobSystemDesc::Parallelism::kCustom:
		numWorkerThreads = desc.m_NumThreads;
		// For custom parallelism, the core values will come from JobSystemDesc::m_pThreadCores
		break;
	case JobSystemDesc::Parallelism::kSingleThreaded:
		// For debugging purposes
		m_Workers.Resize(1);
		m_Workers[0] = new BvTBJobSystemWorker(desc.m_JobListPoolSize);
		return;
	}

	BvAssert(numWorkerThreads > 0, "There must be at least 1 worker thread on the job system!");
	m_Workers.Resize(numWorkerThreads);
	auto coreIndex = 0;
	auto coreCountReset = 0;
	for (auto i = 0u; i < m_Workers.Size(); i++)
	{
		m_Workers[i] = new BvTBJobSystemWorker(this, desc.m_JobListPoolSize, desc.m_pThreadCores == nullptr ? coreIndex : desc.m_pThreadCores[i]);
		if (++coreCountReset == numThreadsPerCore)
		{
			coreIndex++;
			coreCountReset = 0;
		}
	}
}


void BvTBJobSystem::Shutdown()
{
	bool exp = true;
	if (!m_IsActive.compare_exchange_strong(exp, false))
	{
		return;
	}

	for (auto&& pWorker : m_Workers)
	{
		pWorker->Stop();

		delete pWorker;
	}
}


BvTBJobList* BvTBJobSystem::AllocJobList(const u32 jobListSize, JobListPriority priority)
{
	for (auto& jobList : m_JobListPool)
	{
		if (!jobList.m_InUse.test_and_set())
		{
			jobList.Reset();
			return &jobList;
		}
	}

	return nullptr;
}


void BvTBJobSystem::FreeJobList(BvTBJobList* pJobList)
{
	pJobList->m_InUse.clear();
}


void BvTBJobSystem::Submit(BvTBJobList* pJobList)
{
	for (auto pWorker : m_Workers)
	{
		pWorker->AddJobList(pJobList);
	}
}