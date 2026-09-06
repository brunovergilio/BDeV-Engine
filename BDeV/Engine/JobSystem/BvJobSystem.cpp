#include "BvJobSystem.h"
#include "BDeV/Core/System/Threading/BvThread.h"
#include "BDeV/Core/System/Process/BvProcess.h"
#include "BDeV/Core/Container/Third Party/MPMCQueue.h"
#include "BDeV/Core/Utils/BvTime.h"
#include <semaphore>


namespace JobSystem
{
	// =======================================
	// JOB SYSTEM CONTROLLER
	// =======================================

	class BvJobSystemController
	{
	public:
		static void OnJobListCompleted(BvJobSystem* pJS, BvJobList::Result& result)
		{
			pJS->OnJobListCompleted(result);
		}
	};

	// =======================================
	// JOB LIST
	// =======================================

	void BvJobList::SetName(const char* pName)
	{
		m_pName = pName;
	}

	void BvJobList::SetData(u32 maxJobs, BvJob* pJobs, u32 maxChildren /*= 0*/, BvJobList** ppChildren /*= nullptr*/)
	{
		ClearJobsAndDependencies();

		BV_ASSERT(maxJobs > 0 && pJobs != nullptr, "Invalid job data");
		BV_ASSERT(maxChildren == 0 || ppChildren != nullptr, "Invalid child data");

		m_pJobs = pJobs;
		m_MaxJobs = maxJobs;
		m_ppChildren = ppChildren;
		m_MaxChildren = maxChildren;
	}

	void BvJobList::ClearJobsAndDependencies()
	{
		for (auto i = 0; i < m_NumJobs; i++)
		{
			m_pJobs[i].m_Job.Reset();
		}

		m_NumJobs = 0;
		m_NumChildren = 0;
		m_NumParentJobLists = 0;
	}

	void BvJobList::AddChild(BvJobList* pJobList)
	{
		BV_ASSERT(m_NumChildren < m_MaxChildren, "Child joblist array is full");
		m_ppChildren[m_NumChildren++] = pJobList;
		pJobList->m_NumParentJobLists++;
	}

	void BvJobList::SetBatchSize(u32 batchSize)
	{
		BV_ASSERT(batchSize > 0, "Batch size can't be 0");
		m_BatchSize = batchSize;
	}

	void BvJobList::SetPriority(BvJobList::Priority priority)
	{
		m_Priority = priority;
	}

	void BvJobList::Reset()
	{
		m_TotalCycles.store(0, std::memory_order_relaxed);
		m_NumExecutedParents.store(0, std::memory_order_relaxed);
		m_NextJobIndex.store(0, std::memory_order_relaxed);
		m_JobsDone.store(0, std::memory_order_release);
	}

	BvJobList::Result BvJobList::Execute()
	{
		BvJobList::Result result{};
		
		// Fetch a new job index
		auto currJobIndex = m_NextJobIndex.fetch_add(m_BatchSize, std::memory_order_relaxed);
		// Because multiple threads can run the same joblist, we could be in a list that already
		// has all jobs executed or being executed, so we check that here
		if (currJobIndex < m_NumJobs)
		{
			auto endJobIndex = currJobIndex + m_BatchSize;
			// Trim if needed
			if (endJobIndex > m_NumJobs)
			{
				endJobIndex = m_NumJobs;
			}

			auto startCycles = BvTime::GetClockCyclesSerial();
			// Run every job on this batch
			for (auto i = currJobIndex; i < endJobIndex; i++)
			{
				m_pJobs[i].m_Job();
			}
			auto batchCycles = BvTime::GetClockCyclesSerial() - startCycles;
			m_TotalCycles.fetch_add(batchCycles, std::memory_order_relaxed);

			// How many did we run?
			result.m_NumJobsExecuted = endJobIndex - currJobIndex;
			result.m_BatchCycles = batchCycles;

			// Now we update the total number of jobs processed so far. If we just happen to be the one
			// who ran the remaining batch, we need to update the child joblists that depend on us
			auto numJobsDone = m_JobsDone.fetch_add(result.m_NumJobsExecuted, std::memory_order_release) + result.m_NumJobsExecuted;
			if (numJobsDone == m_NumJobs)
			{
				// Mark we're done
				result.m_ListDone = true;
				result.m_ListExhausted = true;

				// Only this call is responsible for updating the child joblists and returning them
				u32 numFinished = 0;
				for (auto i = 0u; i < m_NumChildren; i++)
				{
					// We increment the number of parent joblists that have finished on every child joblist and sort
					// them based on whether they're finished or not
					auto currExecutedParents = m_ppChildren[i]->m_NumExecutedParents.fetch_add(1, std::memory_order_relaxed) + 1;
					if (currExecutedParents == m_ppChildren[i]->NumParentJobLists())
					{
						if (i != numFinished)
						{
							std::swap(m_ppChildren[i], m_ppChildren[numFinished]);
						}
						numFinished++;
					}
				}

				// If any of the child joblists are done waiting, add them to the 'Result' object
				if (numFinished > 0)
				{
					result.m_NumChildren = numFinished;
					result.m_ppChildren = m_ppChildren;
				}

				m_JobsDone.notify_all();
			}
		}
		else
		{
			// Nothing to do on this list anymore
			result.m_ListExhausted = true;
		}

		return result;
	}

	void BvJobList::Wait()
	{
		auto jobsDone = m_JobsDone.load(std::memory_order_acquire);

		while (jobsDone < m_NumJobs)
		{
			m_JobsDone.wait(jobsDone, std::memory_order_acquire);
			jobsDone = m_JobsDone.load(std::memory_order_acquire);
		}
	}

	// =======================================
	// JOB SYSTEM WORKER
	// =======================================

	class alignas(kCacheLineSize) BvJobSystemWorker final
	{
	public:
		struct Stats
		{
			std::atomic<u64> m_NumSchedulerCycles;
			std::atomic<u64> m_NumJobExecutionCycles;
			std::atomic<u64> m_NumJobsExecuted;
			std::atomic<u64> m_NumPausedCycles;
			std::atomic<u64> m_NumPauses;
			std::atomic<u64> m_NumSearchCycles;
			std::atomic<u64> m_NumWorkSearchFailures;
			std::atomic<u64> m_NumStealAttempts;
			std::atomic<u64> m_NumStealQueueProbes;
			std::atomic<u64> m_NumJobListsStolenByWorker;
			std::atomic<u64> m_NumJobListsStolenFromWorker;
			std::atomic<u64> m_NumJobListsSubmitted;
		};

		static constexpr u32 kDefaultJobListQueueSize = 16;
		static constexpr u32 kNumQueues = 3;

		BvJobSystemWorker(BvJobSystem* pOwner, BvJobSystemWorker* pWorkers, u32 numWorkers, u32 workerIndex, u32 workerQueueSize, u32 coreIndex, const char* pName = nullptr)
			: m_pOwner(pOwner), m_pWorkers(pWorkers), m_NumWorkers(numWorkers), m_WorkerIndex(workerIndex)
		{
			for (auto i = 0; i < kNumQueues; i++)
			{
				m_JobLists[i].init(workerQueueSize);
			}

			// Start the threads after initializing the queues
			m_WorkerThread = BvThread([this, coreIndex, pName]()
				{
					Run(coreIndex, pName);
				});
		}

		~BvJobSystemWorker() = default;

		// Called by its owner
		void EnqueueJobList(BvJobList* pJobList)
		{
			bool result = m_JobLists[kNumQueues - 1 - u32(pJobList->GetPriority())].try_emplace(pJobList);
			BV_ASSERT(result, "There must be enough space in the queue - increase capacity");
			m_WorkSignal.release();
		}

		// Called by this and other workers
		bool GetJobList(BvJobList*& pJobList, u32 queueIndex, bool isSteal = false)
		{
			if (m_JobLists[queueIndex].try_pop(pJobList))
			{
				if (isSteal)
				{
					m_Stats.m_NumJobListsStolenFromWorker.fetch_add(1, std::memory_order_relaxed);
				}

				return true;
			}

			return false;
		}

		// Called by its owner
		void ResetStats()
		{
			m_Stats.m_NumSchedulerCycles.store(0, std::memory_order_relaxed);
			m_Stats.m_NumJobExecutionCycles.store(0, std::memory_order_relaxed);
			m_Stats.m_NumJobsExecuted.store(0, std::memory_order_relaxed);
			m_Stats.m_NumPausedCycles.store(0, std::memory_order_relaxed);
			m_Stats.m_NumPauses.store(0, std::memory_order_relaxed);
			m_Stats.m_NumSearchCycles.store(0, std::memory_order_relaxed);
			m_Stats.m_NumWorkSearchFailures.store(0, std::memory_order_relaxed);
			m_Stats.m_NumStealAttempts.store(0, std::memory_order_relaxed);
			m_Stats.m_NumStealQueueProbes.store(0, std::memory_order_relaxed);
			m_Stats.m_NumJobListsStolenByWorker.store(0, std::memory_order_relaxed);
			m_Stats.m_NumJobListsStolenFromWorker.store(0, std::memory_order_relaxed);
			m_Stats.m_NumJobListsSubmitted.store(0, std::memory_order_relaxed);
		}

		// Called by its owner
		void Stop()
		{
			m_Active.store(false, std::memory_order_relaxed);
			m_WorkSignal.release();
			m_WorkerThread.Join();
		}

		// Called by its owner - retrieves stats
		BV_INLINE BvWorkerStats GetStats() const
		{
			return BvWorkerStats
			{
				.m_NumSchedulerCycles = m_Stats.m_NumSchedulerCycles.load(std::memory_order_relaxed),
				.m_NumJobExecutionCycles = m_Stats.m_NumJobExecutionCycles.load(std::memory_order_relaxed),
				.m_NumJobsExecuted = m_Stats.m_NumJobsExecuted.load(std::memory_order_relaxed),
				.m_NumPausedCycles = m_Stats.m_NumPausedCycles.load(std::memory_order_relaxed),
				.m_NumPauses = m_Stats.m_NumPauses.load(std::memory_order_relaxed),
				.m_NumSearchCycles = m_Stats.m_NumSearchCycles.load(std::memory_order_relaxed),
				.m_NumWorkSearchFailures = m_Stats.m_NumWorkSearchFailures.load(std::memory_order_relaxed),
				.m_NumStealAttempts = m_Stats.m_NumStealAttempts.load(std::memory_order_relaxed),
				.m_NumStealQueueProbes = m_Stats.m_NumStealQueueProbes.load(std::memory_order_relaxed),
				.m_NumJobListsStolenByWorker = m_Stats.m_NumJobListsStolenByWorker.load(std::memory_order_relaxed),
				.m_NumJobListsStolenFromWorker = m_Stats.m_NumJobListsStolenFromWorker.load(std::memory_order_relaxed),
				.m_NumJobListsSubmitted = m_Stats.m_NumJobListsSubmitted.load(std::memory_order_relaxed)
			};
		}

	private:
		void Run(u32 coreIndex, const char* pName)
		{
			// Attach to a specific logical processor
			m_WorkerThread.SetAffinity(coreIndex);

			if (pName)
			{
				m_WorkerThread.SetName(pName);
			}
			else
			{
				char name[32]{};
				sprintf(name, "Job System Worker Thread %u", m_WorkerIndex);
				m_WorkerThread.SetName(name);
			}

			// Wait for our first batch of work
			m_WorkSignal.acquire();

			// Run until we're told to stop
			while (m_Active.load(std::memory_order_relaxed))
			{
				BvJobList* pJobList;

				// Have a second loop to minimize the need to check/wait on the work signal or the active flag
				do
				{
					auto schedulerCycles = BvTime::GetClockCyclesSerial();

					auto searchCycles = schedulerCycles;

					pJobList = nullptr;

					// Try to get a joblist from our own queue (any one of them)
					for (auto queueIndex = 0; queueIndex < kNumQueues; queueIndex++)
					{
						if (GetJobList(pJobList, queueIndex))
						{
							break;
						}
					}

					// If that wasn't possible, try to steal from someone else, but stealing will be done in order of joblist priority
					if (!pJobList)
					{
						m_Stats.m_NumStealAttempts.fetch_add(1, std::memory_order_relaxed);

						for (auto queueIndex = 0; queueIndex < kNumQueues; queueIndex++)
						{
							for (auto i = 0; i < m_NumWorkers; i++, m_NextWorkerToStealFrom++)
							{
								m_NextWorkerToStealFrom *= u32(m_NextWorkerToStealFrom < m_NumWorkers);
								if (m_NextWorkerToStealFrom == m_WorkerIndex)
								{
									// Don't steal from ourselves
									continue;
								}

								m_Stats.m_NumStealQueueProbes.fetch_add(1, std::memory_order_relaxed);

								// If we managed to get one, we stop and mark this as the last worker we stole from
								if (m_pWorkers[m_NextWorkerToStealFrom].GetJobList(pJobList, queueIndex, true))
								{
									// Next time we need to steal a joblist, we'll start with the next worker
									m_NextWorkerToStealFrom++;
									break;
								}
							}

							// Did we get one?
							if (pJobList)
							{
								m_Stats.m_NumJobListsStolenByWorker.fetch_add(1, std::memory_order_relaxed);
								break;
							}
						}
					}

					m_Stats.m_NumSearchCycles.fetch_add(BvTime::GetClockCyclesSerial() - searchCycles, std::memory_order_relaxed);

					// No job? Bail
					if (!pJobList)
					{
						m_Stats.m_NumSchedulerCycles.fetch_add(BvTime::GetClockCyclesSerial() - schedulerCycles, std::memory_order_relaxed);
						m_Stats.m_NumWorkSearchFailures.fetch_add(1, std::memory_order_relaxed);
						break;
					}

					u32 numJobsExecuted = 0;

					// Execute as much work as we can on this list
					// TODO: In the future, maybe stack up more joblists and cycle between them, so everyone makes progress;
					// the downside to this would be less joblists to steal, and in turn more failed stealing attempts
					BvJobList::Result result{};
					while (!result.m_ListExhausted)
					{
						result = pJobList->Execute();

						if (result.m_NumJobsExecuted)
						{
							// Update job stats
							numJobsExecuted += result.m_NumJobsExecuted;
							m_Stats.m_NumJobsExecuted.fetch_add(result.m_NumJobsExecuted, std::memory_order_relaxed);
							m_Stats.m_NumJobExecutionCycles.fetch_add(result.m_BatchCycles, std::memory_order_relaxed);
						}

						// If we just happen to be the worker who finished the last batch, submit the child joblists
						if (result.m_ListDone)
						{
							if (result.m_NumChildren)
							{
								m_Stats.m_NumJobListsSubmitted.fetch_add(result.m_NumChildren, std::memory_order_relaxed);
							}

							BvJobSystemController::OnJobListCompleted(m_pOwner, result);
						}
					}

					if (!numJobsExecuted)
					{
						m_Stats.m_NumWorkSearchFailures.fetch_add(1, std::memory_order_relaxed);
					}

					m_Stats.m_NumSchedulerCycles.fetch_add(BvTime::GetClockCyclesSerial() - schedulerCycles, std::memory_order_relaxed);
				} while (pJobList);

				// Check for the work signal again
				if (!m_WorkSignal.try_acquire())
				{
					auto pausedCycles = BvTime::GetClockCyclesSerial();

					// Wait for more work to arrive
					m_WorkSignal.acquire();

					m_Stats.m_NumPausedCycles.fetch_add(BvTime::GetClockCyclesSerial() - pausedCycles, std::memory_order_relaxed);
					m_Stats.m_NumPauses.fetch_add(1, std::memory_order_relaxed);
				}
			}
		}

	private:
		rigtorp::MPMCQueue<BvJobList*> m_JobLists[kNumQueues]; // Queues with joblists
		BvThread m_WorkerThread; // Thread
		BvJobSystem* m_pOwner = nullptr; // Owner
		BvJobSystemWorker* m_pWorkers = nullptr; // Other workers to steal work from
		u32 m_NumWorkers = 0; // Number of workers
		u32 m_WorkerIndex = 0; // This worker's index
		std::counting_semaphore<kI64Max> m_WorkSignal{ 0 }; // Work signal, increment every time a list is submitted
		std::atomic<bool> m_Active{ true }; // Turns the worker on / off
		u32 m_NextWorkerToStealFrom = 0; // Last worker we stole a joblist from

		alignas(kCacheLineSize) Stats m_Stats;
	};

	// =======================================
	// JOB SYSTEM
	// =======================================

	BvJobSystem::BvJobSystem()
	{
	}

	BvJobSystem::~BvJobSystem()
	{
		if (m_pWorkers)
		{
			Shutdown();
		}
	}

	void BvJobSystem::Initialize(const BvJobSystemDesc& jsd /*= JobSystemDesc()*/)
	{
		auto& sysInfo = BvSystem::GetSystemInfo();

		m_PersistentPoolSizes = jsd.m_PersistentPoolSizes;
		// For dynamic sizes, make them all powers of 2 for easy wrap-around
		m_DynamicPoolSizes =
		{
			std::bit_ceil(jsd.m_DynamicPoolSizes.m_JobListPoolSize),
			std::bit_ceil(jsd.m_DynamicPoolSizes.m_JobPoolSize),
			std::bit_ceil(jsd.m_DynamicPoolSizes.m_ChildJobListPoolSize)
		};

		// Limit the amount of worker threads to the number of physical cores
		m_NumWorkers = std::min(jsd.m_NumWorkerThreads, sysInfo.m_NumCores);

		// Keep track of block sizes and offsets
		auto jobListPoolByteSize = sizeof(BvJobList) * (m_PersistentPoolSizes.m_JobListPoolSize + m_DynamicPoolSizes.m_JobListPoolSize);
		auto jobPoolByteSize = sizeof(BvJob) * (m_PersistentPoolSizes.m_JobPoolSize + m_DynamicPoolSizes.m_JobPoolSize);
		auto childJobListPoolByteSize = sizeof(BvJobList*) * (m_PersistentPoolSizes.m_ChildJobListPoolSize + m_DynamicPoolSizes.m_ChildJobListPoolSize);
		auto workerByteSize = sizeof(BvJobSystemWorker) * m_NumWorkers;
		auto memoryNeeded = jobListPoolByteSize + jobPoolByteSize + childJobListPoolByteSize + workerByteSize + alignof(BvJob) + alignof(BvJobList*) + alignof(BvJobSystemWorker);

		// Allocate all the memory we'll need once
		auto pMem = reinterpret_cast<u8*>(m_pArena->Allocate(memoryNeeded, alignof(BvJobList)));
		
		auto offset = 0ull;

		if (jobListPoolByteSize)
		{
			// Assign the joblist pool
			m_pJobListPool = reinterpret_cast<BvJobList*>(pMem);
			BV_CHECK_ALIGNMENT(m_pJobListPool, alignof(BvJobList));
			offset = jobListPoolByteSize;
		}

		if (jobPoolByteSize)
		{
			// Assign the job pool
			m_pJobPool = reinterpret_cast<BvJob*>(BvMemory::AlignMemory(pMem + offset, alignof(BvJob)));
			BV_CHECK_ALIGNMENT(m_pJobPool, alignof(BvJob));
			offset = (size_t(m_pJobPool) - size_t(pMem)) + jobPoolByteSize;
		}

		if (childJobListPoolByteSize)
		{
			// Assign the children pool
			m_ppChildJobListPool = reinterpret_cast<BvJobList**>(BvMemory::AlignMemory(pMem + offset, alignof(BvJobList*)));
			BV_CHECK_ALIGNMENT(m_ppChildJobListPool, alignof(BvJobList*));
			offset = (size_t(m_ppChildJobListPool) - size_t(pMem)) + childJobListPoolByteSize;
		}

		// Assign the workers
		m_pWorkers = reinterpret_cast<BvJobSystemWorker*>(BvMemory::AlignMemory(pMem + offset, alignof(BvJobSystemWorker)));
		BV_CHECK_ALIGNMENT(m_pWorkers, alignof(BvJobSystemWorker));

		// Initialize the joblists, jobs and workers
		for (auto i = 0; i < (m_PersistentPoolSizes.m_JobListPoolSize + m_DynamicPoolSizes.m_JobListPoolSize); i++)
		{
			new(&m_pJobListPool[i]) BvJobList();
		}
		
		for (auto i = 0; i < (m_PersistentPoolSizes.m_JobPoolSize + m_DynamicPoolSizes.m_JobPoolSize); i++)
		{
			new(&m_pJobPool[i]) BvJob();
		}

		auto workerQueueSize = jsd.m_WorkerQueueSize ? jsd.m_WorkerQueueSize : BvJobSystemWorker::kDefaultJobListQueueSize;
		for (auto i = 0; i < m_NumWorkers; i++)
		{
			u32 coreIndex;
			const char* pName = nullptr;
			if (auto pWorkerInfo = jsd.m_pWorkerThreadDescs ? &jsd.m_pWorkerThreadDescs[i] : nullptr)
			{
				// If we were given thread information, use it
				coreIndex = pWorkerInfo->m_LogicalProcessorIndex;
				pName = pWorkerInfo->m_pName;
			}
			else
			{
				// Pick only the first logical processor for each core (if it uses SMT)
				coreIndex = sysInfo.m_pCoreInfos[i].m_pLogicalProcessorIndices[0];
			}

			new(&m_pWorkers[i]) BvJobSystemWorker(this, m_pWorkers, m_NumWorkers, i, workerQueueSize, coreIndex, pName);
		}
	}

	void BvJobSystem::Shutdown()
	{
		// Destroy everything

		// First notify them to stop
		for (auto i = 0; i < m_NumWorkers; i++)
		{
			m_pWorkers[i].Stop();
		}

		// After they're all done, destroy the objects
		for (auto i = 0; i < m_NumWorkers; i++)
		{
			m_pWorkers[i].~BvJobSystemWorker();
		}
		m_pWorkers = nullptr;

		// Free remaining resources
		for (auto i = 0; i < (m_PersistentPoolSizes.m_JobPoolSize + m_DynamicPoolSizes.m_JobPoolSize); i++)
		{
			m_pJobPool[i].~BvJob();
		}

		for (auto i = 0; i < (m_PersistentPoolSizes.m_JobListPoolSize + m_DynamicPoolSizes.m_JobListPoolSize); i++)
		{
			m_pJobListPool[i].~BvJobList();
		}

		void* pMem = m_pJobListPool;
		m_pArena->Free(pMem);
	}

	JobSystem::BvJobList* BvJobSystem::AllocJobList(u32 maxJobs, u32 maxChildren /*= 0*/)
	{
		auto jobListIndex = m_JobListPoolIndex.fetch_add(1, std::memory_order_relaxed);
		auto pJobList = &m_pJobListPool[jobListIndex];

		BvJob* pJobs = nullptr;
		if (maxJobs)
		{
			auto jobIndex = m_JobPoolIndex.fetch_add(maxJobs, std::memory_order_relaxed);
			pJobs = &m_pJobPool[jobIndex];
		}

		BvJobList** ppChildren = nullptr;
		if (maxChildren > 0)
		{
			auto childIndex = m_ChildJobListPoolIndex.fetch_add(maxChildren, std::memory_order_relaxed);

			ppChildren = &m_ppChildJobListPool[childIndex];
		}

		pJobList->SetData(maxJobs, pJobs, maxChildren, ppChildren);

		return pJobList;
	}

	JobSystem::BvJobList* BvJobSystem::AcquireJobList(u32 maxJobs, u32 maxChildren /*= 0*/)
	{
		auto jobListIndex = m_DynamicJobListPoolIndex.fetch_add(1, std::memory_order_relaxed) & (m_DynamicPoolSizes.m_JobListPoolSize - 1);
		auto pJobList = &m_pJobListPool[jobListIndex + m_PersistentPoolSizes.m_JobListPoolSize];

		BV_ASSERT(pJobList->IsDone(), "Joblist not done");

		BvJob* pJobs = nullptr;
		if (maxJobs)
		{
			auto jobIndex = m_JobPoolIndex.fetch_add(maxJobs, std::memory_order_relaxed) & (m_DynamicPoolSizes.m_JobPoolSize - 1);
			pJobs = &m_pJobPool[jobIndex + m_PersistentPoolSizes.m_JobPoolSize];
		}

		BvJobList** ppChildren = nullptr;
		if (maxChildren > 0)
		{
			auto childIndex = m_ChildJobListPoolIndex.fetch_add(maxChildren, std::memory_order_relaxed) & (m_DynamicPoolSizes.m_ChildJobListPoolSize - 1);

			ppChildren = &m_ppChildJobListPool[childIndex + m_PersistentPoolSizes.m_ChildJobListPoolSize];
		}

		pJobList->SetData(maxJobs, pJobs, maxChildren, ppChildren);

		return pJobList;
	}

	void BvJobSystem::ResetAllJoblists()
	{
		Wait(false);

		// Clear persistent slots
		auto currJobPoolIndex = m_JobPoolIndex.load(std::memory_order_relaxed);
		for (auto i = 0; i < currJobPoolIndex; i++)
		{
			m_pJobPool[i].m_Job.Reset();
		}
		m_JobPoolIndex.store(0, std::memory_order_relaxed);

		auto currJobListPoolIndex = m_JobListPoolIndex.load(std::memory_order_relaxed);
		for (auto i = 0; i < currJobListPoolIndex; i++)
		{
			m_pJobListPool[i].ClearJobsAndDependencies();
		}
		m_JobListPoolIndex.store(0, std::memory_order_relaxed);

		m_ChildJobListPoolIndex.store(0, std::memory_order_relaxed);

		// Clear dynamic slots
		currJobPoolIndex = m_DynamicJobPoolIndex.load(std::memory_order_relaxed);
		for (auto i = 0; i < currJobPoolIndex; i++)
		{
			m_pJobPool[m_PersistentPoolSizes.m_JobPoolSize + i].m_Job.Reset();
		}
		m_DynamicJobPoolIndex.store(0, std::memory_order_relaxed);

		currJobListPoolIndex = m_DynamicJobListPoolIndex.load(std::memory_order_relaxed);
		for (auto i = 0; i < currJobListPoolIndex; i++)
		{
			m_pJobListPool[m_PersistentPoolSizes.m_JobListPoolSize + i].ClearJobsAndDependencies();
		}
		m_DynamicJobListPoolIndex.store(0, std::memory_order_relaxed);

		m_DynamicChildJobListPoolIndex.store(0, std::memory_order_relaxed);
	}

	void BvJobSystem::ResetStats()
	{
		for (auto i = 0; i < m_NumWorkers; i++)
		{
			m_pWorkers[i].ResetStats();
		}
	}

	void BvJobSystem::Wait(bool active)
	{
		// Try to steal work if possible, and run as long as we keep finding some
		if (active)
		{
			auto nextWorkerToStealFrom = m_NextWorkerToStealFrom.load(std::memory_order_relaxed);
			BvJobList* pJobList;
			bool anyWorkDone = false;
			do
			{
				pJobList = nullptr;
				for (auto w = 0; w < m_NumWorkers; w++, nextWorkerToStealFrom++)
				{
					nextWorkerToStealFrom *= u32(nextWorkerToStealFrom < m_NumWorkers);

					for (auto i = 0; i < BvJobSystemWorker::kNumQueues; i++)
					{
						if (m_pWorkers[nextWorkerToStealFrom].GetJobList(pJobList, i, true))
						{
							break;
						}
					}

					if (pJobList)
					{
						break;
					}
				}

				if (pJobList)
				{
					anyWorkDone = true;
					BvJobList::Result result{};
					while (!result.m_ListExhausted)
					{
						result = pJobList->Execute();
						if (result.m_ListDone)
						{
							OnJobListCompleted(result);
						}
					}
				}
			} while (pJobList);

			if (anyWorkDone)
			{
				m_NextWorkerToStealFrom.store(nextWorkerToStealFrom + 1, std::memory_order_relaxed);
			}
		}

		// If there was no work to be stolen, wait for it to finish
		u32 numActiveJobLists = m_ActiveJobLists.load(std::memory_order_acquire);
		while (numActiveJobLists != 0)
		{
			m_ActiveJobLists.wait(numActiveJobLists, std::memory_order_acquire);
			numActiveJobLists = m_ActiveJobLists.load(std::memory_order_acquire);
		}
	}

	BvWorkerStats BvJobSystem::GetWorkerStats(u32 workerIndex)
	{
		return m_pWorkers[workerIndex].GetStats();
	}

	// Private - called by the BvJobSystemHelper class
	void BvJobSystem::OnJobListCompleted(BvJobList::Result& result)
	{
		// If we have child joblists, submit them
		if (result.m_NumChildren)
		{
			Submit(result.m_NumChildren, result.m_ppChildren, true);
		}

		// Then afterwards, we can update the active joblist count
		if (m_ActiveJobLists.fetch_sub(1, std::memory_order_release) == 1)
		{
			m_ActiveJobLists.notify_all();
		}
	}

	// Private - called by both the BvJobSystem and BvJobSystemHelper classes
	void BvJobSystem::Submit(u32 numJobLists, BvJobList** ppJobLists, bool includeChildren)
	{
		if (!includeChildren)
		{
			// If we're skipping children, rearrange the array so that we only account for joblists that are ready to run
			u32 currSwap = numJobLists - 1;
			for (auto i = 0u; i < numJobLists && i <= currSwap;)
			{
				if (ppJobLists[i]->NumParentJobLists() > 0)
				{
					if (i != currSwap)
					{
						std::swap(ppJobLists[i], ppJobLists[currSwap--]);
					}
					numJobLists--;
				}
				else
				{
					i++;
				}
			}
		}

		if (!numJobLists)
		{
			return;
		}

		u32 totalWorkerSlots = 0;

		for (auto i = 0u; i < numJobLists; i++)
		{
			totalWorkerSlots += std::min(ppJobLists[i]->GetMaxConcurrency(), m_NumWorkers);
		}

		auto nextWorkerToUse = m_NextWorkerToUse.fetch_add(totalWorkerSlots, std::memory_order_relaxed);
		
		m_ActiveJobLists.fetch_add(numJobLists, std::memory_order_release);

		for (auto i = 0; i < numJobLists; i++)
		{
			auto pJobList = ppJobLists[i];
			BV_ASSERT(pJobList->GetNumJobs() > 0, "Joblist can't be empty");

			// Reset state data
			pJobList->Reset();

			// We limit concurrency based on the number of worker threads we have,
			// as to not duplicate joblists inside workers' queues
			auto concurrency = std::min(pJobList->GetMaxConcurrency(), m_NumWorkers);
			for (auto w = 0; w < concurrency; w++, nextWorkerToUse++)
			{
				// Keep it in range -> [0..m_NumWorkers)
				nextWorkerToUse *= u32(nextWorkerToUse < m_NumWorkers);

				// Add the joblist to the worker's queue
				m_pWorkers[nextWorkerToUse].EnqueueJobList(pJobList);
			}
		}
	}
}