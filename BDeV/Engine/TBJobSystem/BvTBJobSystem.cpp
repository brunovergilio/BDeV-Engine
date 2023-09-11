#include "BvTBJobSystem.h"
#include <bit>


class BvTBJobSystemInstance
{
public:
	static BvTBJobSystem s_TBJobSystem;
};


class BvTBJobSystemWorker
{
	enum class WorkerResult : u8
	{
		kDone,
		kInProgress,
		kStalled
	};

public:
	BvTBJobSystemWorker(const u32 jobListPoolSize, const u32 coreIndex, bool threaded = true);
	BvTBJobSystemWorker(const u32 jobListPoolSize);
	~BvTBJobSystemWorker();

	void AddJobList(BvTBJobList* pJobList);
	void Stop();
	bool IsIdle();

private:
	void Process();
	WorkerResult RunJobs(const u32 jobListIndex, const bool runAll = false);

private:
	BvVector<BvTBJobList*> m_JobLists;
	std::atomic<u32> m_LastJobListIndex;
	std::atomic<u32> m_FirstJobListIndex;
	BvThread m_Thread;
	BvSignal m_WorkSignal;
	BvAdaptiveMutex m_Lock;
	std::atomic<bool> m_Active;
};


BvTBJobSystemWorker::BvTBJobSystemWorker(const u32 jobListPoolSize, const u32 coreIndex, bool threaded)
	: m_WorkSignal(false), m_JobLists(std::bit_ceil(jobListPoolSize)),
	m_Thread(threaded ? BvThread([this, coreIndex]()
		{
			m_Thread.SetAffinityMask(coreIndex);
			m_Active.store(1, std::memory_order::relaxed);
			while (m_Active.load(std::memory_order::relaxed))
			{
				if (m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) == 0)
				{
					m_WorkSignal.Wait();
				}

				while (m_Active.load(std::memory_order::relaxed)
					&& m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) == 0)
				{
					BvProcess::YieldExecution();
				}

				Process();
			}
		}) : BvThread())
{
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
		BvScopedLock lock(m_Lock);

		while (m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) >= (u32)m_JobLists.Size())
		{
			BvProcess::YieldExecution();
		}

		auto lastJobListIndex = m_LastJobListIndex.load(std::memory_order::relaxed) % (u32)m_JobLists.Size();
		m_JobLists[lastJobListIndex] = pJobList;
		m_LastJobListIndex.fetch_add(1, std::memory_order::relaxed);
	}
	
	m_WorkSignal.Set();
}


void BvTBJobSystemWorker::Stop()
{
	m_Active.store(false, std::memory_order::relaxed);
	m_WorkSignal.Set();
	m_Thread.Wait();
}


bool BvTBJobSystemWorker::IsIdle()
{
	return m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) == 0;
}


void BvTBJobSystemWorker::Process()
{
	constexpr u32 kMaxLocalJobLists = 32;
	BvTBJobList* pJobLists[kMaxLocalJobLists]{};

	u32 jobListIndex = 0;
	// Keep track of the last job list that this worker failed processing
	u32 lastSkippedJobListIndex = kU32Max;
	JobListPriority priority = JobListPriority::kLow;

	u32 jobListSize = (u32)m_JobLists.Size();
	u32 lastJobListIndex = 0;
	u32 firstJobListIndex = 0;
	u32 numJobLists = 0;
	do
	{
		lastJobListIndex = m_LastJobListIndex.load(std::memory_order::relaxed);
		firstJobListIndex = m_FirstJobListIndex.load(std::memory_order::relaxed);

		if (numJobLists < kMaxLocalJobLists && lastJobListIndex - firstJobListIndex > 0)
		{
			pJobLists[numJobLists++] = m_JobLists[firstJobListIndex % jobListSize];
			m_FirstJobListIndex.fetch_add(1, std::memory_order::relaxed);
		}

		if (numJobLists == 0)
		{
			break;
		}

		// Start with the first available index
		priority = JobListPriority::kLow;
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

		auto result = RunJobs(jobListIndex, priority == JobListPriority::kHigh);
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
				BvThread::YieldExecution();
			}
			lastSkippedJobListIndex = jobListIndex;
		}
		else// if (result == WorkerResult::kInProgress)
		{
			lastSkippedJobListIndex = kU32Max;
		}
	} while (m_Active.load(std::memory_order::relaxed));
}


BvTBJobSystemWorker::WorkerResult BvTBJobSystemWorker::RunJobs(const u32 jobListIndex, const bool runAll)
{
	auto pCurrJobList = m_JobLists[jobListIndex];
	auto currJobListJobIndex = 0u;

	do
	{
		currJobListJobIndex = pCurrJobList->m_CurrJob.load(std::memory_order::relaxed);

		// If currjobListJobIndex is equal to the amount of jobs, that may not mean all jobs are done,
		// but it means every job is at least being processed
		if (currJobListJobIndex >= pCurrJobList->m_Jobs.Size())
		{
			return WorkerResult::kDone;
		}

		// Do an early-out if there are any sync points or dependencies to be waited on
		// Can sometimes avoid the atomic lock below
		if (pCurrJobList->m_Jobs[currJobListJobIndex].second != nullptr)
		{
			if (pCurrJobList->m_Jobs[currJobListJobIndex].second == BvTBJobList::kSyncPoint)
			{
				if (currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order::relaxed))
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

		if (!pCurrJobList->m_Lock.exchange(true))
		{
			currJobListJobIndex = pCurrJobList->m_CurrJob.fetch_add(1);
			if (currJobListJobIndex >= pCurrJobList->m_Jobs.Size())
			{
				pCurrJobList->m_Lock.store(false);
				return WorkerResult::kDone;
			}

			// If it is a sync point
			if (pCurrJobList->m_Jobs[currJobListJobIndex].second == BvTBJobList::kSyncPoint)
			{
				// Check and make sure every previous job is done
				// and if it is, move the job pointer forward, and mark this job as done
				// otherwise, quit the function
				if (currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order::relaxed))
				{
					pCurrJobList->m_CurrJob.fetch_sub(1);
					pCurrJobList->m_Lock.store(false);
					return WorkerResult::kStalled;
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
					pCurrJobList->m_CurrJob.fetch_sub(1);
					pCurrJobList->m_Lock.store(false);
					return WorkerResult::kStalled;
				}
			}

			pCurrJobList->m_Lock.store(false);
		}
		else
		{
			return WorkerResult::kStalled;
		}

		// If it's a regular job, run it
		if (pCurrJobList->m_Jobs[currJobListJobIndex].second == nullptr)
		{
			pCurrJobList->m_Jobs[currJobListJobIndex].first.Run();
		}
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


void BvTBJobSystem::Initialize(const JobSystemDesc desc)
{
	if (m_IsActive)
	{
		return;
	}

	m_IsActive = true;

	m_JobListPool.Resize(desc.m_JobListPoolSize);

	auto numWorkerThreads = 0;
	auto numThreadsPerCore = 1;

	const auto& sysInfo = BvProcess::GetSystemInfo();
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
		m_Workers[i] = new BvTBJobSystemWorker(desc.m_JobListPoolSize, desc.m_pThreadCores == nullptr ? coreIndex : desc.m_pThreadCores[i], i > 0);
		if (++coreCountReset == numThreadsPerCore)
		{
			coreIndex++;
			coreCountReset = 0;
		}
	}
}


void BvTBJobSystem::Shutdown()
{
	if (!m_IsActive)
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
		if (!jobList.m_InUse.exchange(true))
		{
			jobList.Reset();
			return &jobList;
		}
	}

	return nullptr;
}


void BvTBJobSystem::FreeJobList(BvTBJobList* pJobList)
{
	pJobList->m_InUse.store(false);
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
	for (auto i = 1; i < m_Workers.Size(); i++)
	{
		while (!m_Workers[i]->IsIdle())
		{
			BvProcess::YieldExecution();
		}
	}
}


BvTBJobSystem* g_pTBJobSystem = &BvTBJobSystemInstance::s_TBJobSystem;