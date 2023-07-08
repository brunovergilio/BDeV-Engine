#include "BvJobSystem.h"
#include "BvJobSystemTLS.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/System/Threading/BvThread.h"
#include "BDeV/System/Threading/BvFiber.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/System/Threading/BvProcess.h"


namespace JS
{
	class Counter
	{
	public:
		Counter() {}
		Counter(const Counter&) : m_Value(0), m_Index(0), m_Acquired(false) {}
		//Counter& operator=(const Counter&) { return *this; }

		bool Acquire()
		{
			return !m_Acquired.exchange(true);
		}

		void Release()
		{
			m_Acquired.exchange(false);
		}

		void Decrement()
		{
			m_Value.fetch_sub(1);
		}

		void Increment()
		{
			m_Value.fetch_add(1);
		}

		bool IsDone() const
		{
			return m_Value == 0;
		}

		void SetIndex(u32 index)
		{
			m_Index = index;
		}

	private:
		std::atomic<u32> m_Value{0};
		u32 m_Index{0};
		std::atomic<bool> m_Acquired{false};
	};


	struct JobPool
	{
		JobPool() = default;
		JobPool(const JobPool& rhs) {}

		BvVector<Job> m_Jobs;
		std::atomic<u32> m_CurrJobIndex{};
		std::atomic<u32> m_LastJobIndex{};
		BvSpinlock m_Lock;
	};


	class BvJobSystem
	{
		BV_NOCOPYMOVE(BvJobSystem);

	public:
		BvJobSystem();
		~BvJobSystem();

		void Initialize(const JobSystemDesc& jobSystemDesc = JobSystemDesc());
		void Shutdown();

		void RunJobs(u32 count, const Job* pJobs);
		void WaitForCounter(Counter* pCounter);
		bool IsDone(Counter* pCounter);
		Counter* AllocCounter();
		void FreeCounter(Counter* pCounter);

	private:
		struct ThreadData
		{
			ThreadData() = default;
			ThreadData(const ThreadData&) {}

			BvThread m_Thread;
			BvSignal m_WorkerSignal;
			u32 m_FiberIndex{};
			u32 m_CurrJobIndices[4]{};
			u32 m_CurrWaitListIndices[4]{};
		};

		struct FiberData
		{
			FiberData() = default;
			FiberData(const FiberData&) {}

			enum class Status : u8
			{
				kAvailable,
				kInUse,
				kInWaitList
			};

			Job* m_pJob = nullptr;
			ThreadData* m_pThreadData = nullptr;
			BvFiber m_Fiber;
			std::atomic<Status> m_Status;
			bool shouldAddToWaitList{};
		};

		void WorkerThreadFunction(u32 index);
		FiberData* GetJob(u32 workerThreadIndex, u32 poolIndex);
		bool AcquireFiber(u32& fiberIndex);
		bool AcquireJob(u32 poolIndex, u32& jobIndex, u32 lastJobIndex);
		u32 GetNextJobIndex(u32 poolIndex, u32 currJobIndex);
		static void FiberFunction(void* pData);

	private:
		// TODO: Maybe these should be global and regular arrays with specific sizes
		JobPool m_JobPools[4]{};
		BvVector<Counter> m_CounterPool;
		BvVector<u32> m_WaitLists[4]{};

		BvVector<ThreadData> m_WorkerThreads;
		BvVector<FiberData> m_FiberPool;
		BvSpinlock m_JobLock[4]{};
		BvSpinlock m_WaitListLock[4]{};
		std::atomic<u32> m_LastJobIndices[4]{};
		std::atomic<u32> m_LastWaitListIndices[4]{};
		std::atomic<bool> m_Active{};
	};


	static BvJobSystem s_JobSystem;


	BvJobSystem::BvJobSystem()
	{
	}


	BvJobSystem::~BvJobSystem()
	{
	}


	void BvJobSystem::Initialize(const JobSystemDesc& jobSystemDesc)
	{
		m_Active.store(true);

		// Become a fiber
		BvThread::ConvertToFiber();

		// Spawn worker threads
		m_WorkerThreads.Resize(jobSystemDesc.m_NumWorkerThreads + 1);
		for (auto i = 1u; i <= jobSystemDesc.m_NumWorkerThreads; i++)
		{
			m_WorkerThreads[i].m_WorkerSignal = BvSignal(false);
			m_WorkerThreads[i].m_Thread = BvThread([this, i]()
				{
					// Set TLS data
					SetWorkerThreadIndex(i);
					// Work
					WorkerThreadFunction(i);
				});
		}

		SetWorkerThreadIndex(0);

		// Initialize obj pools
		for (auto& jobPool : m_JobPools)
		{
			jobPool.m_Jobs.Resize(jobSystemDesc.m_JobPoolSize);
		}

		m_CounterPool.Resize(jobSystemDesc.m_JobPoolSize);
		for (auto i = 0u; i < m_CounterPool.Size(); ++i)
		{
			m_CounterPool[i].SetIndex(i);
		}

		m_FiberPool.Resize(jobSystemDesc.m_FiberPoolSize);
		for (auto& fiberData : m_FiberPool)
		{
			fiberData.m_Fiber = BvFiber(FiberFunction, &fiberData, jobSystemDesc.m_FiberStackSize);
		}
	}


	void BvJobSystem::Shutdown()
	{
		m_Active.store(false);
		for (auto& workerThread : m_WorkerThreads)
		{
			// Notify worker threads
			workerThread.m_WorkerSignal.Set();
			// Wait for worker threads to finish
			workerThread.m_Thread.Wait();
		}
		m_WorkerThreads.Clear();
		m_FiberPool.Clear();

		BvThread::ConvertFromFiber();
	}


	void BvJobSystem::RunJobs(u32 count, const Job* pJobs)
	{
		for (u32 i = 0; i < count; ++i, ++pJobs)
		{
			auto jobPoolIndex = 3 - (u32)(*pJobs).m_Priority;
			
			// Lock the joblist
			auto& jobPool = m_JobPools[jobPoolIndex];
			BvScopedLock lock(jobPool.m_Lock);
			auto lastJobIndex = jobPool.m_LastJobIndex.load(std::memory_order::relaxed);
			auto nextJobIndex = (lastJobIndex + 1) % jobPool.m_Jobs.Size();

			// Add the job
			jobPool.m_Jobs[lastJobIndex] = *pJobs;

			// Update the index
			jobPool.m_LastJobIndex.store(nextJobIndex, std::memory_order::release);
		}
		
		// Notify worker threads
		for (auto& workerThread : m_WorkerThreads)
		{
			workerThread.m_WorkerSignal.Set();
		}
	}


	void BvJobSystem::WaitForCounter(const JobCounter& jobCounter)
	{
		if (jobCounter.IsDone())
		{
			return;
		}

		auto workerThreadIndex = GetWorkerThreadIndex();
		if (workerThreadIndex > 0)
		{
			// Worker thread
			auto pWorkerThread = &m_WorkerThreads[workerThreadIndex];
			auto pFiber = &m_FiberPool[pWorkerThread->m_FiberIndex];
			pFiber->shouldAddToWaitList = true;
			// Switch back to the worker thread fiber
			pFiber->m_Fiber.Switch(pWorkerThread->m_Thread.GetThreadFiber());
		}
		else if (workerThreadIndex == 0)
		{
			// Main thread
			// ProcessJobs(jobCounter)
		}
		else
		{
			// System-External thread
			while (!jobCounter.IsDone())
			{
				YieldProcessorExecution();
			}
		}
	}


	void BvJobSystem::WorkerThreadFunction(u32 workerThreadIndex)
	{
		BvThread::ConvertToFiber();
		BvThread::GetCurrentThread().SetAffinity(workerThreadIndex - 1);
		auto& worker = m_WorkerThreads[workerThreadIndex];

		while (m_Active.load())
		{
			// Wait for new jobs
			worker.m_WorkerSignal.Wait();

			bool fiberAcquired = false;
			bool jobAcquired = false;

			// Process each pool by priority
			for (auto poolIndex = 0; poolIndex < 4; ++poolIndex)
			{
				u32 lastJobIndex = m_LastJobIndices[poolIndex].load(std::memory_order::acquire);
				u32& currJobIndex = worker.m_CurrJobIndices[poolIndex];
				u32 fiberIndex = 0;

				// Run until there are no more jobs
				while (currJobIndex != lastJobIndex)
				{
					auto pFiberData = GetJob(workerThreadIndex, poolIndex);
					if (pFiberData)
					{
						// We switch to the fiber and let it execute the job. After finishing the job
						// or if at any point there's a wait call inside the function, the running
						// fiber will switch back to the worker thread that activated it.
						worker.m_Thread.GetThreadFiber().Switch(pFiberData->m_Fiber);

						// When we return from the fiber, we check if it needs to be put on the wait list;
						// if not, then we send it back to the pool
						auto fiberStatus = FiberData::Status::kAvailable;
						if (pFiberData->shouldAddToWaitList)
						{
							// Reset this flag
							pFiberData->shouldAddToWaitList = false;
							fiberStatus = FiberData::Status::kInWaitList;
						}
						// Update the fiber's status
						pFiberData->m_Status.store(fiberStatus);
					}

					// Try to get a fiber from the pool
					if (!fiberAcquired)
					{
						fiberAcquired = AcquireFiber(fiberIndex);
					}

					// Try to get a job from the pool
					if (!jobAcquired)
					{
						jobAcquired = AcquireJob(poolIndex, currJobIndex, lastJobIndex);
					}

					if (fiberAcquired && jobAcquired)
					{
						auto pFiberData = &m_FiberPool[fiberIndex];
						pFiberData->m_pJob = &m_JobPools[poolIndex][currJobIndex];
						pFiberData->m_pThreadData = &worker;

						// We switch to the fiber and let it execute the job. After finishing the job
						// or if at any point there's a wait call inside the function, the running
						// fiber will switch back to the worker thread that activated it.
						worker.m_Thread.GetThreadFiber().Switch(pFiberData->m_Fiber);

						// When we return from the fiber, we check if it needs to be put on the wait list;
						// if not, then we send it back to the pool
						auto fiberStatus = FiberData::Status::kAvailable;
						if (pFiberData->shouldAddToWaitList)
						{
							// Reset this flag
							pFiberData->shouldAddToWaitList = false;
							fiberStatus = FiberData::Status::kInWaitList;
						}
						// Update the fiber's status
						pFiberData->m_Status.store(fiberStatus);

						fiberAcquired = false;
						jobAcquired = false;
					}
				}
			}
		}

		BvThread::ConvertFromFiber();
	}


	BvJobSystem::FiberData* BvJobSystem::GetJob(u32 workerThreadIndex, u32 poolIndex)
	{
		auto& jobPool = m_JobPools[poolIndex];
		auto lastJobIndex = jobPool.m_LastJobIndex.load(std::memory_order::relaxed);
		auto expected = jobPool.m_CurrJobIndex.load(std::memory_order::relaxed);
		auto desired = 0;
		do
		{
			desired = GetNextJobIndex(poolIndex, expected);
		} while (expected != lastJobIndex && !jobPool.m_CurrJobIndex.compare_exchange_weak(expected, desired));

		FiberData* pFiberData = nullptr;
		// If we found a job in the pool, then we look for a fiber to assign it to,
		// otherwise we go to the wait list and look for a job / fiber that's ready to
		// be resumed
		if (expected != lastJobIndex)
		{
			for (auto i = 0; i < m_FiberPool.Size(); ++i)
			{
				auto exp = FiberData::Status::kAvailable;
				if (m_FiberPool[i].m_Status.compare_exchange_strong(exp, FiberData::Status::kInUse))
				{
					pFiberData = &m_FiberPool[i];
					pFiberData->m_pJob = &jobPool.m_Jobs[expected];
					pFiberData->m_pThreadData = &m_WorkerThreads[workerThreadIndex];
					break;
				}
			}
		}
		else
		{
			// TODO: Look for a job in the wait list
		}


		return pFiberData;
	}


	bool BvJobSystem::AcquireJob(u32 poolIndex, u32& jobIndex, u32 lastJobIndex)
	{
		bool result = false;
		auto expectedStatus = JobData::Status::kInPool;
		// Look for a job in the active pool
		while (jobIndex != lastJobIndex)
		{
			result = m_JobPools[poolIndex][jobIndex].m_Status.compare_exchange_strong(expectedStatus, JobData::Status::kAcquired);
			if (result)
			{
				break;
			}
			else
			{
				jobIndex = GetNextJobIndex(poolIndex, jobIndex);
				expectedStatus = JobData::Status::kInPool;
			}
		}

		// If we couldn't find a job in the active pool, then look in the wait pool
		if (!result)
		{
			// TODO: Loop through the wait pool
		}
		
		return result;
	}


	u32 BvJobSystem::GetNextJobIndex(u32 poolIndex, u32 currJobIndex)
	{
		return (currJobIndex + 1) % (u32)m_JobPools[poolIndex].m_Jobs.Size();
	}


	bool BvJobSystem::AcquireFiber(u32& fiberIndex)
	{
		for (auto i = fiberIndex; i < m_FiberPool.Size(); i++)
		{
			auto expectedStatus = FiberData::Status::kAvailable;
			if (m_FiberPool[i].m_Status.compare_exchange_strong(expectedStatus, FiberData::Status::kInUse))
			{
				fiberIndex = i;
				return true;
			}
		}

		for (auto i = 0; i < fiberIndex; i++)
		{
			auto expectedStatus = FiberData::Status::kAvailable;
			if (m_FiberPool[i].m_Status.compare_exchange_strong(expectedStatus, FiberData::Status::kInUse))
			{
				fiberIndex = i;
				return true;
			}
		}

		BvAssert(false, "Failed to get a fiber! Increase fiber pool size!");
		return false;
	}


	void BvJobSystem::FiberFunction(void* pData)
	{
		auto pFiberData = reinterpret_cast<FiberData*>(pData);

		while (s_JobSystem.m_Active.load())
		{
			auto pJobData = pFiberData->m_pJob;
			// Run the job
			pJobData->m_pFunction(pJobData->m_pData);
			// Update the job counter
			pJobData->m_pJobCounter->Update();
			// Return the job to the pool
			pJobData->m_Status.store(JobData::Status::kAvailable);
			pFiberData->m_pJob = nullptr;

			// Switch back to the worker thread
			pFiberData->m_Fiber.Switch(pFiberData->m_pThreadData->m_Thread.GetThreadFiber());
		}

		pFiberData->m_Fiber.Switch(pFiberData->m_pThreadData->m_Thread.GetThreadFiber());
	}


	std::pair<JobCounterImpl*, i32> BvJobSystem::GetUnusedCounter()
	{
		for (auto i = 0u; i < m_CounterPool.Size(); ++i)
		{
			if (m_CounterPool[i].Acquire())
			{
				return std::make_pair(&m_CounterPool[i], (i32)i);
			}
		}

		return std::make_pair(nullptr, -1);
	}


	void Initialize(const JobSystemDesc& jobSystemDesc)
	{
		s_JobSystem.Initialize(jobSystemDesc);
	}


	void Shutdown()
	{
		s_JobSystem.Shutdown();
	}


	JobCounter RunJob(const Job& job)
	{
		return s_JobSystem.RunJobs(1, &job);
	}


	JobCounter RunJobs(u32 count, const Job* const pJobs)
	{
		return s_JobSystem.RunJobs(count, pJobs);
	}


	void WaitForCounter(const JobCounter& jobCounter)
	{
		s_JobSystem.WaitForCounter(jobCounter);
	}


	JobCounter::JobCounter(i32 index, i32 version)
		: m_Index(index), m_Version(version)
	{
	}


	bool JobCounter::IsDone() const
	{
		auto& counter = s_JobSystem.GetCounters()[m_Index];
		return counter.IsDone(m_Version);
	}
}