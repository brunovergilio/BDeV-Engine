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
	constexpr u32 kJobCountPerPool = 400;
	constexpr u32 kSuspendedJobCountPerPool = 32;
	constexpr u32 kCounterPoolCount = 1000;

	class Counter
	{
	public:
		Counter() {}
		//Counter(const Counter&) : m_Value(0), m_Index(0), m_Acquired(false) {}
		//Counter& operator=(const Counter&) { return *this; }

		bool Acquire()
		{
			return !m_Acquired.load(std::memory_order::relaxed) && !m_Acquired.exchange(true);
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

	private:
		std::atomic<u32> m_Value{0};
		std::atomic<bool> m_Acquired{false};
	};

	struct JobData
	{
		Job m_Job;
		Counter* m_pCounter;
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

		JobData* m_pJobData = nullptr;
		Counter* m_pCounter = nullptr;
		ThreadData* m_pThreadData = nullptr;
		BvFiber m_Fiber;
		std::atomic<Status> m_Status;
		bool shouldAddToWaitList{};
	};

	struct SuspendedJobData
	{
		FiberData* m_pFiberData = nullptr;
		BvSpinlock m_Lock;
	};

	struct JobPool
	{
		BvRingBuffer<JobData, kJobCountPerPool> m_Jobs;
		SuspendedJobData m_SuspendedJobs[kSuspendedJobCountPerPool];
	};


	JobPool g_JobPool[3];
	Counter g_CounterPool[kCounterPoolCount];


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
		Counter* AllocCounter();
		void FreeCounter(Counter*& pCounter);

	private:
		void WorkerThreadFunction(u32 index);
		FiberData* GetJob(ThreadData* pThreadData);
		void AddToWaitList(FiberData* pFiberData);
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
		//for (auto& jobPool : m_JobPools)
		//{
		//	jobPool.m_Jobs.Resize(jobSystemDesc.m_JobPoolSize);
		//	jobPool.m_SuspendedJobs.Resize(jobSystemDesc.m_FiberPoolSize);
		//}

		//m_CounterPool.Resize(jobSystemDesc.m_JobPoolSize);

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
		auto pCounter = AllocCounter();
		for (u32 i = 0; i < count; ++i)
		{
			auto jobPoolIndex = 2 - (u32)(*pJobs).m_Priority;
			auto& jobPool = m_JobPools[jobPoolIndex];
			JobData jobData{ pJobs[i], pCounter };
			jobPool.m_Jobs.Push(jobData);
		}
		
		// Notify worker threads
		for (auto& workerThread : m_WorkerThreads)
		{
			workerThread.m_WorkerSignal.Set();
		}
	}


	void BvJobSystem::WaitForCounter(Counter* pCounter)
	{
		if (pCounter->IsDone())
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
			while (!pCounter->IsDone())
			{
				BvProcess::YieldExecution();
			}
		}
	}


	JS::Counter* BvJobSystem::AllocCounter()
	{
		for (auto i = 0; i < kCounterPoolCount; ++i)
		{
			if (g_CounterPool[i].Acquire())
			{
				return &g_CounterPool[i];
			}
		}

		return nullptr;
	}


	void BvJobSystem::FreeCounter(Counter*& pCounter)
	{
		pCounter->Release();
		pCounter = nullptr;
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

			FiberData* pFiberData = nullptr;
			// Run until there are no more jobs
			do
			{
				pFiberData = GetJob(&worker);
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
						// Send the fiber to the wait list
						AddToWaitList(pFiberData);
					}
					else
					{
						// Update the fiber's status
						pFiberData->m_Status.store(FiberData::Status::kAvailable);
					}
				}
			} while (pFiberData != nullptr);
		}

		BvThread::ConvertFromFiber();
	}


	FiberData* BvJobSystem::GetJob(ThreadData* pThreadData)
	{
		JobData* pJobData = nullptr;
		FiberData* pFiberData = nullptr;

		for (auto poolIndex = 0; poolIndex < 3; ++poolIndex)
		{
			if (g_JobPool[poolIndex].m_Jobs.Pop(pJobData))
			{
				// If we found a job in the pool, then we look for a fiber to assign it to
				// TODO: Think of a better way to deal with this when there're no available fibers
				while (true)
				{
					for (auto i = 0; i < m_FiberPool.Size(); ++i)
					{
						auto exp = FiberData::Status::kAvailable;
						if (m_FiberPool[i].m_Status.load(std::memory_order::relaxed) == exp
							&& m_FiberPool[i].m_Status.compare_exchange_strong(exp, FiberData::Status::kInUse))
						{
							pFiberData = &m_FiberPool[i];
							pFiberData->m_pJobData = pJobData;
							pFiberData->m_pThreadData = pThreadData;
							return pFiberData;
						}
					}

					BvProcess::YieldExecution();
				}
			}
			else
			{
				// If we couldn't get a job from the pool, check the wait list
				for (auto suspendedFiberIndex = 0u; suspendedFiberIndex < kSuspendedJobCountPerPool; ++suspendedFiberIndex)
				{
					auto expected = FiberData::Status::kInWaitList;
					pFiberData = g_JobPool[poolIndex].m_SuspendedJobs[suspendedFiberIndex].m_pFiberData;
					if (pFiberData->m_pCounter->IsDone() && pFiberData->m_Status.load(std::memory_order::memory_order_relaxed) == expected
						&& pFiberData->m_Status.compare_exchange_strong(expected, FiberData::Status::kInUse))
					{
						pFiberData->m_pThreadData = pThreadData;
						return pFiberData;
					}
				}
			}
		}

		return nullptr;
	}


	void BvJobSystem::AddToWaitList(FiberData* pFiberData)
	{
		auto poolIndex = 2 - (u32)pFiberData->m_pJobData->m_Job.m_Priority;
		for (auto i = 0; i < kSuspendedJobCountPerPool; i++)
		{
			if (g_JobPool[poolIndex].m_SuspendedJobs[i]->m_WaitListLock.TryLock())
			{

			}
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
			auto pJobData = pFiberData->m_pJobData;
			// Run the job
			pJobData->m_pFunction(pJobData->m_pData);
			// Update the job counter
			pJobData->m_pCounter->Update();
			pFiberData->m_pJobData = nullptr;

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


	void RunJob(const Job& job, JobCounter*& pCounter)
	{

	}


	void RunJobs(u32 count, const Job* pJobs, JobCounter*& pCounter)
	{

	}


	void WaitForCounter(JobCounter* pCounter)
	{

	}


	void WaitForCounterAndFree(JobCounter*& pCounter)
	{

	}
}