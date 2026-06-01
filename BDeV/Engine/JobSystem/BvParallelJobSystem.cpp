#include "BvParallelJobSystem.h"
#include "BDeV/Core/System/Process/BvProcess.h"
#include <bit>
#include <BDeV/Core/Container/BvFixedVector.h>
#include <BDeV/Core/Utils/BvTime.h>
#include "BDeV/Core/System/Threading/BvSync.h"


static const BvParallelJobList* s_pSyncPoint = (BvParallelJobList*)kU64Max;


class alignas(kCacheLineSize) BvParallelJobSystemWorker
{
public:
	enum class WorkerResult : u8
	{
		kOk = 0,
		kInProgress = BvBit(0),
		kDone = BvBit(1),
		kStalled = BvBit(2),
	};

	BvParallelJobSystemWorker(u32 jobListPoolSize, u32 coreIndex, const char* pName, BvParallelJobList::CategoryFlags categoryFlags);
	~BvParallelJobSystemWorker();

	void AddJobList(BvParallelJobList* pJobList);
	static WorkerResult RunJobs(JobSystemWorkerStats& stats, BvParallelJobList* pCurrJobList, bool runAll = false);
	void Stop();
	bool IsIdle();
	BV_INLINE const JobSystemWorkerStats& GetStats() const { return m_Stats; }

private:
	void Process();
	bool IsActive() const;

private:
	static constexpr u32 kMaxJobListPoolSize = 16;

	BvFixedVector<BvParallelJobList*, kMaxJobListPoolSize> m_JobLists;
	BvThread m_Thread;
	//BvWaitEvent m_WorkSignal;
	BvAutoResetEvent m_WorkSignal;
	BvAdaptiveMutex m_Lock;
	JobSystemWorkerStats m_Stats;
	std::atomic<u32> m_LastJobListIndex;
	std::atomic<u32> m_FirstJobListIndex;
	std::atomic<bool> m_IsIdle;
	std::atomic<bool> m_Active;
	BvParallelJobList::CategoryFlags m_CategoryFlags;
};

BV_USE_ENUM_CLASS_OPERATORS(BvParallelJobSystemWorker::WorkerResult);


BvParallelJobSystemWorker::BvParallelJobSystemWorker(u32 jobListPoolSize, u32 coreIndex, const char* pName, BvParallelJobList::CategoryFlags categoryFlags)
	: m_JobLists(std::bit_ceil(jobListPoolSize)), m_CategoryFlags(categoryFlags),
	m_Thread(BvThread([this, coreIndex, pName]()
		{
			m_Thread.SetAffinity(coreIndex);
			if (pName)
			{
				m_Thread.SetName(pName);
			}
			m_Active.store(true, std::memory_order_relaxed);

			auto startTimeNs = BvTime::GetCurrentTimestampInUs();
			auto prevActiveTimeStamp = startTimeNs;
			auto currActiveTimeStamp = prevActiveTimeStamp;

			while (m_Active.load(std::memory_order_acquire))
			{
				if (m_LastJobListIndex.load(std::memory_order_acquire) - m_FirstJobListIndex.load(std::memory_order_relaxed) == 0)
				{
					m_IsIdle.store(true, std::memory_order_relaxed);
					currActiveTimeStamp = BvTime::GetCurrentTimestampInUs();
					m_Stats.m_ActiveTimeUs += currActiveTimeStamp - prevActiveTimeStamp;
					m_WorkSignal.Wait();
					prevActiveTimeStamp = BvTime::GetCurrentTimestampInUs();
					m_IsIdle.store(false, std::memory_order_relaxed);
				}

				Process();

				m_Stats.m_TotalRunningTimeUs = BvTime::GetCurrentTimestampInUs() - startTimeNs;
			}
			auto lastTimeStamp = BvTime::GetCurrentTimestampInUs();
			m_Stats.m_ActiveTimeUs += lastTimeStamp - prevActiveTimeStamp;
			m_Stats.m_TotalRunningTimeUs = lastTimeStamp - startTimeNs;
		}))
{
}


BvParallelJobSystemWorker::~BvParallelJobSystemWorker()
{
}


// Called by the job system from multiple threads
void BvParallelJobSystemWorker::AddJobList(BvParallelJobList* pJobList)
{
	if (!EHasAnyFlags(m_CategoryFlags, pJobList->GetCategoryFlags()))
	{
		return;
	}

	{
		BvScopedLock lock(m_Lock);

		if (!pJobList->IsDone())
		{
			return;
		}

		auto lastJobListIndex = m_LastJobListIndex.load(std::memory_order_acquire);

		// There may be a scenario where we don't have enough space for another job list, so spin-wait. One thing to note is
		// that if this wait is necessary and this call is being done from the worker thread itself, it'll cause a deadlock.
		// This can happen if a job running on this worker thread submits another job list and its pool is already full.
		while (lastJobListIndex - m_FirstJobListIndex.load(std::memory_order_relaxed) >= m_JobLists.Size())
		{
			BV_ASSERT(m_Thread.GetId() != BvThread::GetCurrentThread().GetId(), "Deadlock on job system's worker thread, increase job list pool count");
			BvCPU::Yield();
		}

		m_JobLists[lastJobListIndex & ((u32)m_JobLists.Size() - 1)] = pJobList;
		pJobList->m_CurrJob.store(0, std::memory_order_relaxed);
		pJobList->m_CurrSyncPoint.store(0, std::memory_order_relaxed);
		pJobList->m_JobsDone.store(0, std::memory_order_relaxed);
		pJobList->m_LastSubmittedVersion.fetch_add(1, std::memory_order_relaxed);

		m_LastJobListIndex.fetch_add(1, std::memory_order_release);
	}
	
	m_WorkSignal.Set();
}


BvParallelJobSystemWorker::WorkerResult BvParallelJobSystemWorker::RunJobs(JobSystemWorkerStats& stats, BvParallelJobList* pCurrJobList, bool runAll)
{
	auto currJobListJobIndex = 0u;
	auto currSyncPointIndex = 0u;

	WorkerResult result = WorkerResult::kOk;
	do
	{
		// If currJobListJobIndex is equal to the amount of jobs, that may not mean all jobs are done,
		// but it means every job is at least being processed
		currJobListJobIndex = pCurrJobList->m_CurrJob.load(std::memory_order_relaxed);
		if (currJobListJobIndex >= pCurrJobList->m_JobCount)
		{
			return result | WorkerResult::kDone;
		}

		if (pCurrJobList->m_SyncPointCount)
		{
			currSyncPointIndex = pCurrJobList->m_CurrSyncPoint.load(std::memory_order_relaxed);
			// Do an early-out if there are any sync points or dependencies to be waited on
			// Can sometimes avoid the atomic lock below
			if (pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_Index == currJobListJobIndex)
			{
				if ((pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_pJobList && !pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_pJobList->IsDone())
					|| (!pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_pJobList && currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order_relaxed)))
				{
					return result | WorkerResult::kStalled;
				}
			}
		}

		// Try to lock the job list
		if (!pCurrJobList->m_Lock.test_and_set(std::memory_order_acquire))
		{
			// Fetch a new index from it
			currJobListJobIndex = pCurrJobList->m_CurrJob.fetch_add(1, std::memory_order_relaxed);
			if (currJobListJobIndex >= pCurrJobList->m_JobCount)
			{
				pCurrJobList->m_Lock.clear(std::memory_order_release);
				return result | WorkerResult::kDone;
			}

			if (pCurrJobList->m_SyncPointCount)
			{
				currSyncPointIndex = pCurrJobList->m_CurrSyncPoint.load(std::memory_order_relaxed);
				if (pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_Index == currJobListJobIndex)
				{
					if ((pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_pJobList && !pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_pJobList->IsDone())
						|| (!pCurrJobList->m_pSyncPoints[currSyncPointIndex].m_pJobList && currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order_relaxed)))
					{
						pCurrJobList->m_CurrJob.fetch_sub(1, std::memory_order_relaxed);
						pCurrJobList->m_Lock.clear(std::memory_order_release);
						return result | WorkerResult::kStalled;
					}

					pCurrJobList->m_CurrSyncPoint.fetch_add(1, std::memory_order_relaxed);
				}
			}

			pCurrJobList->m_Lock.clear(std::memory_order_release);
		}
		else
		{
			return result | WorkerResult::kStalled;
		}

		auto jobTime = BvTime::GetCurrentTimestampInUs();
		pCurrJobList->m_pJobs[currJobListJobIndex].m_Job();
		jobTime = BvTime::GetCurrentTimestampInUs() - jobTime;
		stats.m_JobsRun++;
		stats.m_TotalJobTimeUs += jobTime;

		result |= WorkerResult::kInProgress;

		if (pCurrJobList->m_JobsDone.fetch_add(1, std::memory_order_relaxed) + 1 == pCurrJobList->m_JobCount)
		{
			pCurrJobList->m_Version.fetch_add(1, std::memory_order_release);
			result |= WorkerResult::kDone;
			break;
		}
		
		if (currJobListJobIndex + 1 == pCurrJobList->m_JobCount)
		{
			result |= WorkerResult::kDone;
			break;
		}
	} while (runAll);

	return result;
}


void BvParallelJobSystemWorker::Stop()
{
	m_Active.store(false, std::memory_order_release);
	m_WorkSignal.Set();
	m_Thread.Wait();
}


bool BvParallelJobSystemWorker::IsIdle()
{
	return m_IsIdle.load(std::memory_order_relaxed);
}


void BvParallelJobSystemWorker::Process()
{
	constexpr u32 kMaxLocalJobLists = std::bit_ceil(16u);
	BvParallelJobList* pJobLists[kMaxLocalJobLists]{};

	// Keep track of the last job list that this worker stalled on
	u32 lastStalledJobListIndex = kU32Max;

	u32 lastJobListIndex = 0;
	u32 firstJobListIndex = 0;
	u32 numJobLists = 0;
	do
	{
		lastJobListIndex = m_LastJobListIndex.load(std::memory_order_relaxed);
		firstJobListIndex = m_FirstJobListIndex.load(std::memory_order_relaxed);

		if (numJobLists < kMaxLocalJobLists && lastJobListIndex != firstJobListIndex)
		{
			pJobLists[numJobLists++] = m_JobLists[firstJobListIndex & ((u32)m_JobLists.Size() - 1)];
			m_FirstJobListIndex.store(firstJobListIndex + 1, std::memory_order_relaxed);
		}

		if (numJobLists == 0)
		{
			break;
		}

		u32 jobListIndex = 0;
		BvParallelJobList::Priority priority = BvParallelJobList::Priority::kNormal;

		// Start with the first available index
		if (lastStalledJobListIndex == kU32Max)
		{
			for (auto i = 0u; i < numJobLists; i++)
			{
				if (pJobLists[i]->GetPriority() > priority
					&& !pJobLists[i]->IsWaitingForJobList())
				{
					priority = pJobLists[i]->GetPriority();
					jobListIndex = i;
					if (priority == BvParallelJobList::Priority::kHigh)
					{
						break;
					}
				}
			}
		}
		else
		{
			jobListIndex = lastStalledJobListIndex;
			for (auto i = 0u; i < numJobLists; i++)
			{
				if (i != lastStalledJobListIndex && pJobLists[i]->GetPriority() >= priority
					&& !pJobLists[i]->IsWaitingForJobList())
				{
					priority = pJobLists[i]->GetPriority();
					jobListIndex = i;
					if (priority == BvParallelJobList::Priority::kHigh)
					{
						break;
					}
				}
			}
		}

		auto result = RunJobs(m_Stats, pJobLists[jobListIndex], priority == BvParallelJobList::Priority::kHigh);
		if (EHasFlag(result, WorkerResult::kDone))
		{
			// Remove the JobList from the pool
			if (jobListIndex != numJobLists - 1)
			{
				std::swap(pJobLists[jobListIndex], pJobLists[numJobLists - 1]);
			}
			numJobLists--;
			lastStalledJobListIndex = kU32Max;
		}
		else if (EHasFlag(result, WorkerResult::kStalled))
		{
			if (lastStalledJobListIndex == jobListIndex && !EHasFlag(result, WorkerResult::kInProgress))
			{
				BvCPU::Yield();
			}
			lastStalledJobListIndex = jobListIndex;
		}
		else
		{
			lastStalledJobListIndex = kU32Max;
		}
	} while (numJobLists > 0);
}


bool BvParallelJobSystemWorker::IsActive() const
{
	return m_Active.load(std::memory_order_acquire);
}


bool BvParallelJobResult::IsDone() const
{
	return m_pJobList->IsDone();
}


void BvParallelJobResult::Wait()
{
	m_pJobList->Wait();
}


BvParallelJobResult::BvParallelJobResult(BvParallelJobList* pJobList)
	: m_pJobList(pJobList)
{
}


BvParallelJobSystem::BvParallelJobSystem()
{
}


BvParallelJobSystem::~BvParallelJobSystem()
{
	Shutdown();
}


void BvParallelJobSystem::Initialize(const JobSystemDesc& desc)
{
	if (desc.m_NumWorkerThreads == 0)
	{
		return;
	}

	m_pMemoryArena = desc.m_pMemoryArena;
	if (!m_pMemoryArena)
	{
		// Reserve extra space just in case
		size_t maxSize = (desc.m_JobListPoolSize * sizeof(BvParallelJobList) + desc.m_JobPoolSize * sizeof(BvParallelJob)) << 2;
		m_pMemoryArena->SetName("Job System Memory Arena");
	}

	const auto& sysInfo = BvSystem::GetSystemInfo();

	m_WorkerCount = desc.m_NumWorkerThreads;
	m_pWorkers = reinterpret_cast<BvParallelJobSystemWorker*>(BV_MALLOC(*m_pMemoryArena, sizeof(BvParallelJobSystemWorker) * m_WorkerCount, alignof(BvParallelJobSystemWorker)));
	for (auto i = 0u; i < m_WorkerCount; i++)
	{
		u32 processorIndex = JobSystemDesc::kAutoSelectProcessor;
		const char* pName = nullptr;
		BvParallelJobList::CategoryFlags categoryFlags = BvParallelJobList::CategoryFlags::kAny;
		if (desc.m_pWorkerThreadDescs)
		{
			auto& workerDesc = desc.m_pWorkerThreadDescs[i];
			processorIndex = workerDesc.m_LogicalProcessorIndex;
			pName = workerDesc.m_pName;
			categoryFlags = workerDesc.m_CategoryFlags;
		}

		if (processorIndex == JobSystemDesc::kAutoSelectProcessor)
		{
			// Try to pick one processor per core
			if (sysInfo.m_NumCores > i)
			{
				processorIndex = sysInfo.m_pCoreInfos[i].m_pLogicalProcessorIndices[0];
			}
			else
			{
				processorIndex = 0;
			}
		}

		new(&m_pWorkers[i]) BvParallelJobSystemWorker(desc.m_JobListPoolSize, processorIndex, pName, categoryFlags);
	}
}


void BvParallelJobSystem::Shutdown()
{
	if (m_pMemory)
	{
		for (auto i = 0; i < m_WorkerCount; ++i)
		{
			m_pWorkers[i].Stop();
			m_pWorkers[i].~BvParallelJobSystemWorker();
		}
		BV_MFREE(*m_pMemoryArena, m_pWorkers);

		BV_DELETE(m_pMemoryArena);
		m_pMemoryArena = nullptr;
	}
}


BvParallelJobList* BvParallelJobSystem::AllocJobList(u32 maxJobs, u32 maxSyncs, BvParallelJobList::Priority priority, BvParallelJobList::CategoryFlags categoryFlags)
{
	BV_ASSERT(maxJobs > 0, "Need at least 1 job");

	BvParallelJob* pJobs = BV_MNEW_ARRAY(*m_pMemoryArena, BvParallelJob, maxJobs + maxSyncs);
	BvParallelJobList::SyncPoint* pSyncPoints = nullptr;
	if (maxSyncs)
	{
		pSyncPoints = BV_MNEW_ARRAY(*m_pMemoryArena, BvParallelJobList::SyncPoint, maxSyncs);
	}
	auto pJobList = BV_MNEW(*m_pMemoryArena, BvParallelJobList);
	pJobList->Set(pJobs, pSyncPoints, maxJobs, maxSyncs, priority, categoryFlags, false);

	return pJobList;
}


void BvParallelJobSystem::FreeJobList(BvParallelJobList*& pJobList)
{
	BV_MDELETE_IN_PLACE(*m_pMemoryArena, pJobList);
	BV_MDELETE_ARRAY(*m_pMemoryArena, pJobList->m_pJobs);
	if (pJobList->m_pSyncPoints)
	{
		BV_MDELETE_ARRAY(*m_pMemoryArena, pJobList->m_pSyncPoints);
	}
	pJobList = nullptr;
}


void BvParallelJobSystem::Submit(BvParallelJobList* pJobList)
{
	if (m_WorkerCount)
	{
		for (auto i = 0; i < m_WorkerCount; ++i)
		{
			m_pWorkers[i].AddJobList(pJobList);
		}
	}
	else
	{
		JobSystemWorkerStats dummy;
		BvParallelJobSystemWorker::RunJobs(dummy, pJobList, true);
	}
}


BvParallelJobResult BvParallelJobSystem::Submit(const BvParallelJob& job, BvParallelJobList::Priority priority, BvParallelJobList::CategoryFlags categoryFlags)
{
	BvParallelJobList* pJobList;
	BvParallelJob* pJob;
	AcquireJobData(pJobList, pJob);
	*pJob = job;

	pJobList->Set(pJob, nullptr, 1, 0, priority, categoryFlags, true);

	Submit(pJobList);

	return BvParallelJobResult(pJobList);
}


BvParallelJobResult BvParallelJobSystem::Submit(u32 jobCount, const BvParallelJob* pJobs, BvParallelJobList::Priority priority, BvParallelJobList::CategoryFlags categoryFlags)
{
	BvParallelJobList* pJobList;
	BvParallelJob* pDstJobs;
	AcquireJobData(pJobList, pDstJobs, jobCount);
	for (auto i = 0; i < jobCount; i++)
	{
		pDstJobs[i] = pJobs[i];
	}

	pJobList->Set(pDstJobs, nullptr, jobCount, 0, priority, categoryFlags, true);

	Submit(pJobList);

	return BvParallelJobResult(pJobList);
}

void BvParallelJobSystem::Wait()
{
	for (auto i = 0; i < m_WorkerCount; ++i)
	{
		while (!m_pWorkers[i].IsIdle())
		{
			BvCPU::Yield();
		}
	}
}


const JobSystemWorkerStats& BvParallelJobSystem::GetStats(u32 workerThreadIndex) const
{
	BV_ASSERT(workerThreadIndex < m_WorkerCount, "Invalid worker thread index");
	return m_pWorkers[workerThreadIndex].GetStats();
}


void BvParallelJobSystem::AcquireJobData(BvParallelJobList*& pJobList, BvParallelJob*& pJobs, u32 jobCount)
{
	auto jobIndex = m_CurrJobIndex.fetch_add(jobCount, std::memory_order_relaxed) & (m_JobPoolSize - 1);
	pJobs = &m_JobPool[jobIndex];

	auto jobListIndex = m_CurrJobListIndex.fetch_add(1, std::memory_order_relaxed) & (m_JobListPoolSize - 1);
	pJobList = &m_JobListPool[jobListIndex];
}

// Empty stub for sync points and dependencies
void NoOp()
{
}


void BvParallelJobList::AddJob(const BvParallelJob& job)
{
	AddInternal() = job;
}


void BvParallelJobList::AddSyncPoint(BvParallelJobList* pWaitJobList)
{
	BV_ASSERT(m_JobCount < m_MaxJobs, "JobList is full");
	BV_ASSERT(m_SyncPointCount < m_MaxSyncPoints, "No syncpoints available");

	m_pSyncPoints[m_SyncPointCount++] = { pWaitJobList, m_JobCount };
	auto& job = AddInternal();
	job.m_Job.Set(NoOp);
}


void BvParallelJobList::Reset(bool resetJobs)
{
	if (resetJobs)
	{
		for (; m_JobCount > 0; m_JobCount--)
		{
			m_pJobs[m_JobCount - 1].m_Job.~BvCMTask();
		}
	}
}


bool BvParallelJobList::IsDone() const
{
	return m_Version.load(std::memory_order_acquire) != m_LastSubmittedVersion.load(std::memory_order_relaxed);
}


void BvParallelJobList::Wait() const
{
	while (!IsDone())
	{
		BvCPU::Yield();
	}
}


BvParallelJobList::BvParallelJobList()
{
}


BvParallelJobList::~BvParallelJobList()
{
}


void BvParallelJobList::Set(BvParallelJob* pJobs, SyncPoint* pSyncPoints, u32 maxJobs, u32 maxSyncPoints, Priority priority, CategoryFlags categoryFlags, bool temp)
{
	m_pJobs = pJobs;
	m_pSyncPoints = pSyncPoints;
	m_MaxJobs = maxJobs;
	m_MaxSyncPoints = maxSyncPoints;
	m_Priority = priority;
	m_CategoryFlags = categoryFlags;

	if (temp)
	{
		m_JobCount = maxJobs;
		m_SyncPointCount = maxSyncPoints;
	}
}


bool BvParallelJobList::IsWaitingForJobList() const
{
	auto index = m_CurrJob.load(std::memory_order_relaxed);
	if (index >= m_JobCount || m_SyncPointCount == 0)
	{
		return false;
	}

	auto syncPointIndex = m_CurrSyncPoint.load(std::memory_order_relaxed);
	return m_pSyncPoints[syncPointIndex].m_Index == index
		&& m_pSyncPoints[syncPointIndex].m_pJobList
		&& !m_pSyncPoints[syncPointIndex].m_pJobList->IsDone();
}


BvParallelJob& BvParallelJobList::AddInternal()
{
	return m_pJobs[m_JobCount++];
}