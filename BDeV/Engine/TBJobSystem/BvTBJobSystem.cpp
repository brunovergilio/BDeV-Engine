#include "BvTBJobSystem.h"
#include <bit>


class BvTBJobSystemWorker
{
public:
	enum class WorkerResult : u8
	{
		kDone,
		kInProgress,
		kStalled
	};

	BvTBJobSystemWorker(u32 jobListPoolSize, u32 coreIndex);
	~BvTBJobSystemWorker();

	void AddJobList(BvTBJobList* pJobList);
	static WorkerResult RunJobs(BvTBJobList* pCurrJobList, bool runAll = false);
	void Stop();

private:
	void Process();
	bool IsActive() const;

private:
	BvVector<BvTBJobList*> m_JobLists;
	std::atomic<u32> m_LastJobListIndex;
	std::atomic<u32> m_FirstJobListIndex;
	BvThread m_Thread;
	BvSignal m_WorkSignal;
	BvAdaptiveMutex m_Lock;
	std::atomic<bool> m_Active;
};


BvTBJobSystemWorker::BvTBJobSystemWorker(u32 jobListPoolSize, u32 coreIndex)
	: m_JobLists(std::bit_ceil(jobListPoolSize)),
	m_Thread(BvThread([this, coreIndex]()
		{
			m_Thread.LockToCore(coreIndex);
			m_Active.store(1, std::memory_order::relaxed);
			while (IsActive())
			{
				if (m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) == 0)
				{
					m_WorkSignal.Wait();
				}

				Process();
			}
		}))
{
}


BvTBJobSystemWorker::~BvTBJobSystemWorker()
{
}


void BvTBJobSystemWorker::AddJobList(BvTBJobList* pJobList)
{
	{
		BvScopedLock lock(m_Lock);

		// This could loop infinitely if the thread that's adding a job list is the same worker thread that owns m_JobLists and the list is full
		// TODO: Figure out how to best handle this
		while (m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) >= (u32)m_JobLists.Size())
		{
			BvProcess::YieldExecution();
		}

		auto lastJobListIndex = m_LastJobListIndex.load(std::memory_order::relaxed) & ((u32)m_JobLists.Size() - 1);
		m_JobLists[lastJobListIndex] = pJobList;
		m_LastJobListIndex.fetch_add(1, std::memory_order::relaxed);
	}
	
	m_WorkSignal.Set();
}


BvTBJobSystemWorker::WorkerResult BvTBJobSystemWorker::RunJobs(BvTBJobList* pCurrJobList, bool runAll)
{
	auto currJobListJobIndex = 0u;

	do
	{
		// If currjobListJobIndex is equal to the amount of jobs, that may not mean all jobs are done,
		// but it means every job is at least being processed
		currJobListJobIndex = pCurrJobList->m_CurrJob.load(std::memory_order::relaxed);
		if (currJobListJobIndex >= (u32)pCurrJobList->m_Jobs.Size())
		{
			return WorkerResult::kDone;
		}

		// Do an early-out if there are any sync points or dependencies to be waited on
		// Can sometimes avoid the atomic lock below
		if (pCurrJobList->m_Jobs[currJobListJobIndex].m_SyncType == BvTBJobList::JobSyncType::kSyncPoint)
		{
			if (currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order::relaxed))
			{
				return WorkerResult::kStalled;
			}
		}
		else if (pCurrJobList->m_Jobs[currJobListJobIndex].m_SyncType == BvTBJobList::JobSyncType::kJobListDependency)
		{
			if (!pCurrJobList->m_Jobs[currJobListJobIndex].m_pDependency->IsDone())
			{
				return WorkerResult::kStalled;
			}
		}

		if (pCurrJobList->m_Lock.TryLock())
		{
			currJobListJobIndex = pCurrJobList->m_CurrJob.fetch_add(1);
			if (currJobListJobIndex >= pCurrJobList->m_Jobs.Size())
			{
				pCurrJobList->m_Lock.Unlock();
				return WorkerResult::kDone;
			}

			// If it is a sync point
			if (pCurrJobList->m_Jobs[currJobListJobIndex].m_SyncType == BvTBJobList::JobSyncType::kSyncPoint)
			{
				// Check and make sure every previous job is done
				if (currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order::relaxed))
				{
					pCurrJobList->m_CurrJob.fetch_sub(1);
					pCurrJobList->m_Lock.Unlock();
					return WorkerResult::kStalled;
				}
			}
			// If it is a dependency
			else if (pCurrJobList->m_Jobs[currJobListJobIndex].m_SyncType == BvTBJobList::JobSyncType::kJobListDependency)
			{
				// Check if the other joblist is done
				if (!pCurrJobList->m_Jobs[currJobListJobIndex].m_pDependency->IsDone())
				{
					pCurrJobList->m_CurrJob.fetch_sub(1);
					pCurrJobList->m_Lock.Unlock();
					return WorkerResult::kStalled;
				}
			}

			pCurrJobList->m_Lock.Unlock();
		}
		else
		{
			return WorkerResult::kStalled;
		}

		pCurrJobList->m_Jobs[currJobListJobIndex].m_Job.Run();

		pCurrJobList->m_JobsDone.fetch_add(1);
	} while (runAll);

	if (currJobListJobIndex + 1 >= pCurrJobList->m_Jobs.Size())
	{
		return WorkerResult::kDone;
	}
	else
	{
		return WorkerResult::kInProgress;
	}
}


void BvTBJobSystemWorker::Stop()
{
	m_Active.store(false, std::memory_order::relaxed);
	m_WorkSignal.Set();
	m_Thread.Wait();
}


void BvTBJobSystemWorker::Process()
{
	constexpr u32 kMaxLocalJobLists = 32;
	BvTBJobList* pJobLists[kMaxLocalJobLists]{};

	// Keep track of the last job list that this worker failed processing
	u32 lastSkippedJobListIndex = kU32Max;

	u32 jobListSize = (u32)m_JobLists.Size();
	u32 lastJobListIndex = 0;
	u32 firstJobListIndex = 0;
	u32 numJobLists = 0;
	while (IsActive())
	{
		lastJobListIndex = m_LastJobListIndex.load(std::memory_order::relaxed);
		firstJobListIndex = m_FirstJobListIndex.load(std::memory_order::relaxed);

		if (numJobLists < kMaxLocalJobLists && lastJobListIndex - firstJobListIndex > 0)
		{
			pJobLists[numJobLists++] = m_JobLists[firstJobListIndex & (jobListSize - 1)];
			m_FirstJobListIndex.fetch_add(1, std::memory_order::relaxed);
		}

		if (numJobLists == 0)
		{
			break;
		}

		u32 jobListIndex = 0;
		JobListPriority priority = JobListPriority::kLow;

		// Start with the first available index
		if (lastSkippedJobListIndex == kU32Max)
		{
			for (auto i = 0u; i < numJobLists; i++)
			{
				if (pJobLists[i]->GetPriority() > priority
					&& !pJobLists[i]->IsWaitingForDependency())
				{
					priority = pJobLists[i]->GetPriority();
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
			for (auto i = 0u; i < numJobLists; i++)
			{
				if (i != lastSkippedJobListIndex && pJobLists[i]->GetPriority() >= priority
					&& !pJobLists[i]->IsWaitingForDependency())
				{
					priority = pJobLists[i]->GetPriority();
					jobListIndex = i;
					if (priority == JobListPriority::kHigh)
					{
						break;
					}
				}
			}
		}

		auto result = RunJobs(pJobLists[jobListIndex], priority == JobListPriority::kHigh);
		if (result == WorkerResult::kDone)
		{
			// Remove the JobList from the pool
			for (auto i = jobListIndex; i < numJobLists - 1; i++)
			{
				pJobLists[i] = pJobLists[i + 1];
			}
			--numJobLists;
			lastSkippedJobListIndex = kU32Max;
		}
		else if (result == WorkerResult::kStalled)
		{
			if (lastSkippedJobListIndex == jobListIndex)
			{
				BvProcess::YieldExecution();
			}
			lastSkippedJobListIndex = jobListIndex;
		}
		else// if (result == WorkerResult::kInProgress)
		{
			lastSkippedJobListIndex = kU32Max;
		}
	}
}


bool BvTBJobSystemWorker::IsActive() const
{
	return m_Active.load(std::memory_order::relaxed);
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
	auto numWorkerThreads = 0;
	auto numThreadsPerCore = 1;

	const auto& sysInfo = BvProcess::GetSystemInfo();
	BvAssert(desc.m_NumThreads <= sysInfo.m_NumLogicalProcessors, "Not enough logical processors for the worker threads");

	switch (desc.m_Parallelism)
	{
	case JobSystemDesc::Parallelism::kUseThreadCount:
		numWorkerThreads = sysInfo.m_NumLogicalProcessors;
		numThreadsPerCore = sysInfo.m_NumLogicalProcessors / sysInfo.m_NumCores;
		break;
	case JobSystemDesc::Parallelism::kUseCoreCount:
		numWorkerThreads = sysInfo.m_NumCores;
		numThreadsPerCore = 1;
		break;
	case JobSystemDesc::Parallelism::kCustom:
		numWorkerThreads = desc.m_NumThreads;
		// For custom parallelism, the core values will come from JobSystemDesc::m_pThreadCores
		break;
	}

	BvAssert(numWorkerThreads > 0, "There must be at least 1 worker thread on the job system!");
	m_Workers.Resize(numWorkerThreads);
	auto coreIndex = 0;
	auto coreCountReset = 0;
	for (auto i = 0u; i < m_Workers.Size(); i++)
	{
		m_Workers[i] = new BvTBJobSystemWorker(desc.m_JobListPoolSize, desc.m_pThreadCores == nullptr ? coreIndex : desc.m_pThreadCores[i]);
		if (++coreCountReset == numThreadsPerCore)
		{
			coreIndex++;
			coreCountReset = 0;
		}
	}
}


void BvTBJobSystem::Shutdown()
{
	for (auto&& pWorker : m_Workers)
	{
		pWorker->Stop();

		delete pWorker;
	}
}


BvTBJobList* BvTBJobSystem::AllocJobList(u32 maxJobs, u32 maxSyncs, JobListPriority priority)
{
	return new BvTBJobList(maxJobs, maxSyncs, priority);
}


void BvTBJobSystem::FreeJobList(BvTBJobList*& pJobList)
{
	delete pJobList;
	pJobList = nullptr;
}


void BvTBJobSystem::Submit(BvTBJobList* pJobList)
{
	for (auto pWorker : m_Workers)
	{
		pWorker->AddJobList(pJobList);
	}
}


class BvTBJobSystemInstance
{
public:
	static BvTBJobSystem s_Instance;
};


BvTBJobSystem* g_pJobSystem = &BvTBJobSystemInstance::s_Instance;


// Empty stub for sync points and dependencies
void NoOp()
{
}


void BvTBJobList::AddSyncPoint()
{
	BvAssert(m_Jobs.Size() < m_Jobs.Capacity(), "JobList is full");
	if (m_Jobs.Size() == 0
		|| m_Jobs.Size() == (u32)m_Jobs.Capacity() - 1
		|| m_Jobs[m_Jobs.Size() - 1].m_SyncType == JobSyncType::kSyncPoint)
	{
		return;
	}

	auto& job = AddInternal();
	job.m_Job.Set(NoOp);
	job.m_SyncType = JobSyncType::kSyncPoint;
}


void BvTBJobList::AddJobListDependency(BvTBJobList* pDependency)
{
	BvAssert(m_Jobs.Size() < m_Jobs.Capacity(), "JobList is full");
	BvAssert(pDependency != nullptr, "Needs a valid JobList");
	if (m_Jobs.Size() == (u32)m_Jobs.Capacity() - 1)
	{
		return;
	}

	auto& job = AddInternal();
	job.m_Job.Set(NoOp);
	job.m_SyncType = JobSyncType::kJobListDependency;
	job.m_pDependency = pDependency;
}


void BvTBJobList::Reset(bool resetJobs)
{
	if (resetJobs)
	{
		m_Jobs.Clear();
	}

	m_CurrJob.store(0);
	m_JobsDone.store(0);
}


void BvTBJobList::Submit(bool threaded)
{
	if (threaded)
	{
		g_pJobSystem->Submit(this);
	}
	else
	{
		BvTBJobSystemWorker::WorkerResult result = BvTBJobSystemWorker::WorkerResult::kDone;
		do
		{
			result = BvTBJobSystemWorker::RunJobs(this, true);
		} while (result != BvTBJobSystemWorker::WorkerResult::kDone);
	}
}


bool BvTBJobList::IsDone() const
{
	return m_JobsDone.load(std::memory_order::relaxed) == m_Jobs.Size();
}


bool BvTBJobList::IsWaitingForDependency() const
{
	auto index = m_CurrJob.load(std::memory_order::relaxed);
	return index < m_Jobs.Size()
		&& m_Jobs[index].m_SyncType == JobSyncType::kJobListDependency
		&& !m_Jobs[index].m_pDependency->IsDone();
}


void BvTBJobList::Wait() const
{
	while (!IsDone())
	{
		BvProcess::YieldExecution();
	}
}


JobListPriority BvTBJobList::GetPriority() const
{
	return m_Priority;
}


BvTBJobList::BvTBJobList(u32 maxJobs, u32 maxSyncs, const JobListPriority priority)
	: m_Priority(priority)
{
	m_Jobs.Reserve(maxJobs + maxSyncs);
}


BvTBJobList::~BvTBJobList()
{
}


BvTBJobList::Job& BvTBJobList::AddInternal()
{
	return m_Jobs.EmplaceBack();
}