#include "BvParallelJobSystem.h"
#include "BDeV/Core/System/Process/BvProcess.h"
#include <bit>
#include <BDeV/Core/Container/BvFixedVector.h>


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

	BvParallelJobSystemWorker(u32 jobListPoolSize, u32 coreIndex, const char* pName, BvParallelJobList::CategoryFlags category);
	~BvParallelJobSystemWorker();

	void AddJobList(BvParallelJobList* pJobList);
	static WorkerResult RunJobs(BvParallelJobList* pCurrJobList, bool runAll = false);
	void Stop();
	bool IsIdle();

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
	std::atomic<u32> m_LastJobListIndex;
	std::atomic<u32> m_FirstJobListIndex;
	std::atomic<bool> m_IsIdle;
	std::atomic<bool> m_Active;
	BvParallelJobList::CategoryFlags m_Category;
};

BV_USE_ENUM_CLASS_OPERATORS(BvParallelJobSystemWorker::WorkerResult);


BvParallelJobSystemWorker::BvParallelJobSystemWorker(u32 jobListPoolSize, u32 coreIndex, const char* pName, BvParallelJobList::CategoryFlags category)
	: m_JobLists(std::bit_ceil(jobListPoolSize)), m_Category(category),
	m_Thread(BvThread([this, coreIndex, pName]()
		{
			m_Thread.SetAffinity(coreIndex);
			if (pName)
			{
				m_Thread.SetName(pName);
			}
			m_Active.store(true, std::memory_order::relaxed);

			while (m_Active.load(std::memory_order::acquire))
			{
				if (m_LastJobListIndex.load(std::memory_order::relaxed) - m_FirstJobListIndex.load(std::memory_order::relaxed) == 0)
				{
					m_IsIdle.store(true, std::memory_order::relaxed);
					m_WorkSignal.Wait();
					m_IsIdle.store(false, std::memory_order::relaxed);
				}

				Process();
			}
		}))
{
}


BvParallelJobSystemWorker::~BvParallelJobSystemWorker()
{
}


// Called by the job system from multiple threads
void BvParallelJobSystemWorker::AddJobList(BvParallelJobList* pJobList)
{
	{
		BvScopedLock lock(m_Lock);

		auto lastJobListIndex = m_LastJobListIndex.load(std::memory_order::relaxed);

		// There may be a scenario where we don't have enough space for another job list, so spin-wait. One thing to note is
		// that if this wait is necessary and this call is being done from the worker thread itself, it'll cause a deadlock.
		// This can happen if a job running on this worker thread submits another job list and its pool is already full.
		while (lastJobListIndex - m_FirstJobListIndex.load(std::memory_order::relaxed) >= m_JobLists.Size())
		{
			BV_ASSERT(m_Thread.GetId() != BvThread::GetCurrentThread().GetId(), "Deadlock on job system's worker thread, increase job list pool count");
			BvCPU::Yield();
		}

		m_JobLists[lastJobListIndex & ((u32)m_JobLists.Size() - 1)] = pJobList;
		m_LastJobListIndex.store(lastJobListIndex + 1, std::memory_order::relaxed);
	}
	
	m_WorkSignal.Set();
}


BvParallelJobSystemWorker::WorkerResult BvParallelJobSystemWorker::RunJobs(BvParallelJobList* pCurrJobList, bool runAll)
{
	auto currJobListJobIndex = 0u;

	WorkerResult result = WorkerResult::kOk;
	do
	{
		// If currJobListJobIndex is equal to the amount of jobs, that may not mean all jobs are done,
		// but it means every job is at least being processed
		currJobListJobIndex = pCurrJobList->m_CurrJob.load(std::memory_order::relaxed);
		if (currJobListJobIndex >= pCurrJobList->m_JobCount)
		{
			return result | WorkerResult::kDone;
		}

		// Do an early-out if there are any sync points or dependencies to be waited on
		// Can sometimes avoid the atomic lock below
		if (pCurrJobList->m_pJobs[currJobListJobIndex].m_SyncType == BvParallelJobList::JobSyncType::kSyncPoint)
		{
			if (currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order::relaxed))
			{
				return result | WorkerResult::kStalled;
			}
		}
		else if (pCurrJobList->m_pJobs[currJobListJobIndex].m_SyncType == BvParallelJobList::JobSyncType::kJobListDependency)
		{
			if (!pCurrJobList->m_pJobs[currJobListJobIndex].m_pDependency->IsDone())
			{
				return result | WorkerResult::kStalled;
			}
		}

		// Try to lock the job list
		if (!pCurrJobList->m_Lock.test_and_set(std::memory_order::acquire))
		{
			// Fetch a new index from it
			currJobListJobIndex = pCurrJobList->m_CurrJob.fetch_add(1, std::memory_order::relaxed);
			if (currJobListJobIndex >= pCurrJobList->m_JobCount)
			{
				pCurrJobList->m_Lock.clear(std::memory_order::release);
				return result | WorkerResult::kDone;
			}

			// If it is a sync point
			if (pCurrJobList->m_pJobs[currJobListJobIndex].m_SyncType == BvParallelJobList::JobSyncType::kSyncPoint)
			{
				// Check and make sure every previous job is done
				if (currJobListJobIndex > pCurrJobList->m_JobsDone.load(std::memory_order::relaxed))
				{
					pCurrJobList->m_CurrJob.fetch_sub(1, std::memory_order::relaxed);
					pCurrJobList->m_Lock.clear(std::memory_order::release);
					return result | WorkerResult::kStalled;
				}
			}
			// If it is a dependency
			else if (pCurrJobList->m_pJobs[currJobListJobIndex].m_SyncType == BvParallelJobList::JobSyncType::kJobListDependency)
			{
				// Check if the other job list is done
				if (!pCurrJobList->m_pJobs[currJobListJobIndex].m_pDependency->IsDone())
				{
					pCurrJobList->m_CurrJob.fetch_sub(1, std::memory_order::relaxed);
					pCurrJobList->m_Lock.clear(std::memory_order::release);
					return result | WorkerResult::kStalled;
				}
			}

			pCurrJobList->m_Lock.clear(std::memory_order::release);
		}
		else
		{
			return result | WorkerResult::kStalled;
		}

		pCurrJobList->m_pJobs[currJobListJobIndex].m_Job();

		result |= WorkerResult::kInProgress;

		pCurrJobList->m_JobsDone.fetch_add(1, std::memory_order::relaxed);

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
	m_Active.store(false, std::memory_order::release);
	m_WorkSignal.Set();
	m_Thread.Wait();
}


bool BvParallelJobSystemWorker::IsIdle()
{
	return m_IsIdle.load(std::memory_order::relaxed);
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
		lastJobListIndex = m_LastJobListIndex.load(std::memory_order::relaxed);
		firstJobListIndex = m_FirstJobListIndex.load(std::memory_order::relaxed);

		if (numJobLists < kMaxLocalJobLists && lastJobListIndex != firstJobListIndex)
		{
			pJobLists[numJobLists++] = m_JobLists[firstJobListIndex & ((u32)m_JobLists.Size() - 1)];
			m_FirstJobListIndex.store(firstJobListIndex + 1, std::memory_order::relaxed);
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
					&& !pJobLists[i]->IsWaitingForDependency())
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
					&& !pJobLists[i]->IsWaitingForDependency())
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

		auto result = RunJobs(pJobLists[jobListIndex], priority == BvParallelJobList::Priority::kHigh);
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
	return m_Active.load(std::memory_order::acquire);
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
	if (desc.m_NumWorkerThreadDescs == 0)
	{
		return;
	}

	m_pMemoryArena = desc.m_pMemoryArena;
	if (!m_pMemoryArena)
	{
		using MemoryArenaType = BvMemoryArena<BvGrowableHeapAllocator, BvNoLock, BvNoBoundsChecker, BvNoMemoryMarker, BvNoMemoryTracker>;

		// Reserve extra space just in case
		size_t maxSize = (desc.m_JobListPoolSize * sizeof(BvParallelJobList) + desc.m_JobPoolSize * sizeof(BvParallelJobList::Job)) << 2;
		m_pMemory = BV_NEW(BvGrowableHeapAllocator)(maxSize);
		m_pMemoryArena = BV_NEW(MemoryArenaType)(reinterpret_cast<BvGrowableHeapAllocator*>(m_pMemory), BV_NAME_ID("Job System Memory Arena"));
	}

	const auto& sysInfo = BvSystem::GetSystemInfo();

	m_WorkerCount = desc.m_NumWorkerThreadDescs;
	m_pWorkers = reinterpret_cast<BvParallelJobSystemWorker*>(BV_MALLOC(*m_pMemoryArena, sizeof(BvParallelJobSystemWorker) * m_WorkerCount, alignof(BvParallelJobSystemWorker)));
	char tmpBuffer[48]{};
	for (auto i = 0u; i < m_WorkerCount; i++)
	{
		u32 processorIndex = JobSystemDesc::kAutoSelectProcessor;
		const char* pName = tmpBuffer;
		BvParallelJobList::CategoryFlags categoryFlags = BvParallelJobList::CategoryFlags::kAny;
		if (desc.m_pWorkerThreadDescs)
		{
			auto& workerDesc = desc.m_pWorkerThreadDescs[i];
			processorIndex = workerDesc.m_LogicalProcessorIndex;
			pName = workerDesc.m_pName;
			categoryFlags = workerDesc.m_CategoryFlags;
		}
		else
		{
			snprintf(tmpBuffer, 48, "Parallel Job System - Worker Thread %u", i);
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

		auto pAllocator = reinterpret_cast<BvGrowableHeapAllocator*>(m_pMemory);
		BV_DELETE(pAllocator);
		m_pMemory = nullptr;

		BV_DELETE(m_pMemoryArena);
		m_pMemoryArena = nullptr;
	}
}


BvParallelJobList* BvParallelJobSystem::AllocJobList(u32 maxJobs, u32 maxSyncs, BvParallelJobList::Priority priority, BvParallelJobList::CategoryFlags category)
{
	return BV_MNEW(*m_pMemoryArena, BvParallelJobList)(this, maxJobs, maxSyncs, priority, category);
}


void BvParallelJobSystem::FreeJobList(BvParallelJobList*& pJobList)
{
	BV_MDELETE(*m_pMemoryArena, pJobList);
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
		BvParallelJobSystemWorker::RunJobs(pJobList, true);
	}
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


// Empty stub for sync points and dependencies
void NoOp()
{
}


void BvParallelJobList::AddSyncPoint()
{
	BV_ASSERT(m_JobCount < m_MaxJobCount, "JobList is full");
	if (m_JobCount == 0
		|| m_JobCount == m_MaxJobCount - 1
		|| m_pJobs[m_JobCount - 1].m_SyncType == JobSyncType::kSyncPoint)
	{
		return;
	}

	auto& job = AddInternal();
	job.m_Job.Set(NoOp);
	job.m_SyncType = JobSyncType::kSyncPoint;
}


void BvParallelJobList::AddJobListDependency(BvParallelJobList* pDependency)
{
	BV_ASSERT(m_JobCount < m_MaxJobCount, "JobList is full");
	BV_ASSERT(pDependency != nullptr, "Needs a valid JobList");
	if (m_JobCount == m_MaxJobCount - 1)
	{
		return;
	}

	auto& job = AddInternal();
	job.m_Job.Set(NoOp);
	job.m_SyncType = JobSyncType::kJobListDependency;
	job.m_pDependency = pDependency;
}


void BvParallelJobList::Reset(bool resetJobs)
{
	if (resetJobs)
	{
		m_JobCount = 0;
	}

	m_CurrJob.store(0, std::memory_order::relaxed);
	m_JobsDone.store(0, std::memory_order::relaxed);
}


void BvParallelJobList::Submit()
{
	m_pJobSystem->Submit(this);
}


bool BvParallelJobList::IsDone() const
{
	return m_JobsDone.load(std::memory_order::relaxed) == m_JobCount;
}


bool BvParallelJobList::IsWaitingForDependency() const
{
	auto index = m_CurrJob.load(std::memory_order::relaxed);
	return index < m_JobCount
		&& m_pJobs[index].m_SyncType == JobSyncType::kJobListDependency
		&& !m_pJobs[index].m_pDependency->IsDone();
}


void BvParallelJobList::Wait() const
{
	while (!IsDone())
	{
		BvCPU::Yield();
	}
}


BvParallelJobList::BvParallelJobList(BvParallelJobSystem* pJobSystem, u32 maxJobs, u32 maxSyncs, BvParallelJobList::Priority priority, BvParallelJobList::CategoryFlags category)
	: m_pJobSystem(pJobSystem), m_Priority(priority), m_MaxJobCount(maxJobs + maxSyncs)
{
}


BvParallelJobList::~BvParallelJobList()
{
}


BvParallelJobList::Job& BvParallelJobList::AddInternal()
{
	return m_pJobs[m_JobCount++];
}