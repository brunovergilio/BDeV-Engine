#include "BvTBJobSystem.h"
#include "BDeV/Core/System/Process/BvProcess.h"
#include <bit>


class BvTBJobSystemInstance
{
public:
	static BvTBJobSystem* GetInstance()
	{
		static BvTBJobSystem s_Instance;
		return &s_Instance;
	}
};


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
	bool IsIdle();

private:
	void Process();
	bool IsActive() const;

private:
	BvVector<BvTBJobList*> m_JobLists;
	BvThread m_Thread;
	BvSignal m_WorkSignal;
	BvAdaptiveMutex m_Lock;
	std::atomic<u32> m_LastJobListIndex;
	std::atomic<u32> m_FirstJobListIndex;
	std::atomic<bool> m_IsIdle;
	std::atomic<bool> m_Active;
};


BvTBJobSystemWorker::BvTBJobSystemWorker(u32 jobListPoolSize, u32 coreIndex)
	: m_JobLists(std::bit_ceil(jobListPoolSize)),
	m_Thread(BvThread([this, coreIndex]()
		{
			m_Thread.LockToCore(coreIndex);
			m_Active.store(1, std::memory_order::relaxed);
			do
			{
				if (m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) == 0)
				{
					m_IsIdle.store(true, std::memory_order::relaxed);
					m_WorkSignal.Wait();
					m_IsIdle.store(false, std::memory_order::relaxed);
				}

				Process();
			} while (IsActive());
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

		// This could loop infinitely if the thread that's adding a job list is the same worker thread that owns m_JobLists,
		// and the list happens to be already full. If that's ever the case, increase the job list size.
		while (m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) >= (u32)m_JobLists.Size())
		{
			BvProcess::Yield();
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

		if (!pCurrJobList->m_Lock.test_and_set())
		{
			currJobListJobIndex = pCurrJobList->m_CurrJob.fetch_add(1);
			if (currJobListJobIndex >= pCurrJobList->m_Jobs.Size())
			{
				pCurrJobList->m_Lock.clear();
				return WorkerResult::kDone;
			}

			// If it is a sync point
			if (pCurrJobList->m_Jobs[currJobListJobIndex].m_SyncType == BvTBJobList::JobSyncType::kSyncPoint)
			{
				// Check and make sure every previous job is done
				if (currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order::relaxed))
				{
					pCurrJobList->m_CurrJob.fetch_sub(1);
					pCurrJobList->m_Lock.clear();
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
					pCurrJobList->m_Lock.clear();
					return WorkerResult::kStalled;
				}
			}

			pCurrJobList->m_Lock.clear();
		}
		else
		{
			return WorkerResult::kStalled;
		}

		pCurrJobList->m_Jobs[currJobListJobIndex].m_Job.Run();

		pCurrJobList->m_JobsDone.fetch_add(1, std::memory_order::relaxed);
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


bool BvTBJobSystemWorker::IsIdle()
{
	return m_IsIdle.load(std::memory_order::relaxed);
}


void BvTBJobSystemWorker::Process()
{
	constexpr u32 kMaxLocalJobLists = 32;
	BvTBJobList* pJobLists[kMaxLocalJobLists]{};

	// Keep track of the last job list that this worker stalled on
	u32 lastSkippedJobListIndex = kU32Max;

	u32 jobListSize = (u32)m_JobLists.Size();
	u32 lastJobListIndex = 0;
	u32 firstJobListIndex = 0;
	u32 numJobLists = 0;
	while (m_Active.load(std::memory_order::relaxed))
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
		BvTBJobList::Priority priority = BvTBJobList::Priority::kLow;

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
					if (priority == BvTBJobList::Priority::kHigh)
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
					if (priority == BvTBJobList::Priority::kHigh)
					{
						break;
					}
				}
			}
		}

		auto result = RunJobs(pJobLists[jobListIndex], priority == BvTBJobList::Priority::kHigh);
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
				BvProcess::Yield();
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


void BvTBJobSystem::Initialize(const JobSystemDesc& desc)
{
	auto numWorkerThreads = 0u;
	auto numThreadsPerCore = 1u;

	const auto& sysInfo = BvProcess::GetSystemInfo();
	BvAssert(desc.m_NumWorkerThreads <= sysInfo.m_NumLogicalProcessors, "Not enough logical processors for the worker threads");

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
		numWorkerThreads = desc.m_NumWorkerThreads;
		// For custom parallelism, the core values will come from JobSystemDesc::m_pThreadCores
		break;
	case JobSystemDesc::Parallelism::kSingleThreaded:
		return;
	}

	BvAssert(numWorkerThreads > 0, "There must be at least 1 worker thread on the job system!");
	m_Workers.Resize(numWorkerThreads);
	auto coreIndex = 0u;
	auto coreCountReset = 0u;
	for (auto i = 0u; i < m_Workers.Size(); i++)
	{
		m_Workers[i] = new BvTBJobSystemWorker(desc.m_JobListPoolSize, desc.m_pWorkerThreadDescs == nullptr ? coreIndex : desc.m_pWorkerThreadDescs[i].m_CoreIndex);
		if (++coreCountReset == numThreadsPerCore)
		{
			coreIndex++;
			coreCountReset = 0;
		}
	}
}


void BvTBJobSystem::Shutdown()
{
	for (auto pWorker : m_Workers)
	{
		pWorker->Stop();

		delete pWorker;
	}
	m_Workers.Clear();
}


BvTBJobList* BvTBJobSystem::AllocJobList(u32 maxJobs, u32 maxSyncs, BvTBJobList::Priority priority)
{
	return new BvTBJobList(this, maxJobs, maxSyncs, priority);
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


void BvTBJobSystem::Wait()
{
	for (auto pWorker : m_Workers)
	{
		while (!pWorker->IsIdle())
		{
			BvProcess::Yield();
		}
	}
}


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
		m_pJobSystem->Submit(this);
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
		BvProcess::Yield();
	}
}


BvTBJobList::Priority BvTBJobList::GetPriority() const
{
	return m_Priority;
}


BvTBJobList::Category BvTBJobList::GetCategory() const
{
	return m_Category;
}


BvTBJobList::BvTBJobList(BvTBJobSystem* pJobSystem, u32 maxJobs, u32 maxSyncs, BvTBJobList::Priority priority)
	: m_pJobSystem(pJobSystem), m_Priority(priority)
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


BvTBJobSystem* GetJobSystem()
{
	return BvTBJobSystemInstance::GetInstance();
}