#include "BvJobSystem.h"
#include "BvJobSystemTLS.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/System/Threading/BvThread.h"
#include "BDeV/System/Threading/BvFiber.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/System/Threading/BvProcess.h"
#include "BvRingBuffer.h"


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

	struct JobPool
	{
		JobPool() = default;
		JobPool(const JobPool& rhs) {}

		BvRingBuffer<Job> m_Jobs;
		BvRingBuffer<FiberData*> m_SuspendedJobs;
	};

	//struct JobPool
	//{
	//	JobPool() = default;
	//	JobPool(const JobPool& rhs) {}

	//	BvVector<Job> m_Jobs;
	//	BvVector<u32> m_WaitFiberIndices;
	//	std::atomic<u32> m_CurrJobIndex{};
	//	std::atomic<u32> m_LastJobIndex{};
	//	std::atomic<u32> m_CurrWaitFiberIndex{};
	//	std::atomic<u32> m_LastWaitFiberIndex{};
	//	BvSpinlock m_JobLock;
	//	BvSpinlock m_WaitFiberLock;
	//};


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
		void WorkerThreadFunction(u32 index);
		FiberData* GetJob(ThreadData* pThreadData, JobPool& jobPool);
		void AddToWaitList(JobPool& jobPool);
		u32 GetNextIndex(u32 currIndex, u32 size);
		void FiberFunction(u32 fiberIndex);

	private:
		BvVector<Counter> m_CounterPool;
		BvVector<JobPool> m_JobPools;
		BvVector<ThreadData> m_WorkerThreads;
		BvVector<FiberData> m_FiberPool;
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
			jobPool.m_SuspendedJobs.Resize(jobSystemDesc.m_FiberPoolSize);
		}

		m_CounterPool.Resize(jobSystemDesc.m_JobPoolSize);
		for (auto i = 0u; i < m_CounterPool.Size(); ++i)
		{
			m_CounterPool[i].SetIndex(i);
		}

		m_FiberPool.Resize(jobSystemDesc.m_FiberPoolSize);
		for (auto i = 0u; i < m_FiberPool.Size(); ++i)
		{
			m_FiberPool[i].m_Fiber = BvFiber(jobSystemDesc.m_FiberStackSize, [this, i]()
				{
					FiberFunction(i);
				});
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
			jobPool.m_Jobs.Push(*pJobs);
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

			// Process each pool by priority
			for (auto poolIndex = 0; poolIndex < 4; ++poolIndex)
			{
				FiberData* pFiberData = nullptr;
				// Run until there are no more jobs
				do
				{
					pFiberData = GetJob(&worker, m_JobPools[poolIndex]);
					if (pFiberData)
					{
						// We switch to the fiber and let it execute the job. After finishing the job
						// or if at any point there's a wait call inside the function, the running
						// fiber will switch back to the worker thread that activated it.
						worker.m_Thread.GetThreadFiber().Switch(pFiberData->m_Fiber);

						// When we return from the fiber, we check if it needs to be put on the wait list;
						// if not, then we send it back to the pool
						if (pFiberData->shouldAddToWaitList)
						{
							// Reset this flag
							pFiberData->shouldAddToWaitList = false;
							// Update the fiber's status
							pFiberData->m_Status.store(FiberData::Status::kInWaitList);
							AddToWaitList(m_JobPools[poolIndex]);
						}
						else
						{
							// Update the fiber's status
							pFiberData->m_Status.store(FiberData::Status::kAvailable);
						}
					}
				} while (pFiberData != nullptr);
			}
		}

		BvThread::ConvertFromFiber();
	}


	FiberData* BvJobSystem::GetJob(ThreadData* pThreadData, JobPool& jobPool)
	{
		//auto lastJobIndex = jobPool.m_LastJobIndex.load();
		//auto currJobIndex = jobPool.m_CurrJobIndex.load();
		//auto nextJobIndex = GetNextIndex(currJobIndex, (u32)jobPool.m_Jobs.Size());
		//while (currJobIndex != lastJobIndex && !jobPool.m_CurrJobIndex.compare_exchange_weak(currJobIndex, nextJobIndex))
		//{
		//	nextJobIndex = GetNextIndex(currJobIndex, (u32)jobPool.m_Jobs.Size());
		//}
		Job* pJob = nullptr;

		FiberData* pFiberData = nullptr;
		// If we found a job in the pool, then we look for a fiber to assign it to,
		// otherwise we go to the wait list and look for a job / fiber that's ready to
		// be resumed
		if (jobPool.m_Jobs.Pop(pJob))
		{
			while (!pFiberData)
			{
				for (auto i = 0; i < m_FiberPool.Size(); ++i)
				{
					auto exp = FiberData::Status::kAvailable;
					if (m_FiberPool[i].m_Status.compare_exchange_strong(exp, FiberData::Status::kInUse))
					{
						pFiberData = &m_FiberPool[i];
						pFiberData->m_pJob = pJob;
						pFiberData->m_pThreadData = pThreadData;
						break;
					}
				}

				if (!pFiberData)
				{
					YieldProcessorExecution();
				}
			}
		}
		else
		{
			//auto lastWaitFiberIndex = jobPool.m_LastWaitFiberIndex.load();
			//auto currWaitFiberIndex = jobPool.m_CurrWaitFiberIndex.load();
			//auto nextWaitFiberIndex = GetNextIndex(currWaitFiberIndex, (u32)jobPool.m_WaitFiberIndices.Size());
			//while (currWaitFiberIndex != lastWaitFiberIndex && !jobPool.m_CurrWaitFiberIndex.compare_exchange_weak(currWaitFiberIndex, nextWaitFiberIndex))
			//{
			//	nextWaitFiberIndex = GetNextIndex(currWaitFiberIndex, (u32)jobPool.m_WaitFiberIndices.Size());
			//}

			FiberData** ppFiberData = nullptr;
			if (jobPool.m_SuspendedJobs.Pop(ppFiberData))
			{
				pFiberData = *ppFiberData;
				pFiberData->m_pThreadData = pThreadData;
				pFiberData->m_Status.exchange(FiberData::Status::kInUse);
			}
		}

		return pFiberData;
	}


	void BvJobSystem::AddToWaitList(JobPool& jobPool)
	{
		{
			// Lock the joblist
			BvScopedLock lock(jobPool.m_WaitFiberLock);
			auto lastWaitFiberIndex = jobPool.m_LastWaitFiberIndex.load();
			auto nextWaitFiberIndex = (lastWaitFiberIndex + 1) % jobPool.m_WaitFiberIndices.Size();

			jobPool.m_WaitFiberIndices[lastWaitFiberIndex] = *pJobs;

			// Update the index
			jobPool.m_LastWaitFiberIndex.store(nextWaitFiberIndex);
		}

		// Notify worker threads
		for (auto& workerThread : m_WorkerThreads)
		{
			workerThread.m_WorkerSignal.Set();
		}
	}


	u32 BvJobSystem::GetNextIndex(u32 currIndex, u32 size)
	{
		return (currIndex + 1) % size;
	}


	void BvJobSystem::FiberFunction(u32 fiberIndex)
	{
		auto pFiberData = &m_FiberPool[fiberIndex];

		while (s_JobSystem.m_Active.load())
		{
			auto pJobData = pFiberData->m_pJob;
			// Run the job
			pJobData->m_pFunction(pJobData->m_pData);
			// Update the job counter
			pJobData->m_pCounter->Update();
			pFiberData->m_pJob = nullptr;

			// Switch back to the worker thread
			pFiberData->m_Fiber.Switch(pFiberData->m_pThreadData->m_Thread.GetThreadFiber());
		}

		pFiberData->m_Fiber.Switch(pFiberData->m_pThreadData->m_Thread.GetThreadFiber());
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