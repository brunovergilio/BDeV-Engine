//#include "BvJobSystem.h"
//#include "BvJobSystemTLS.h"
//#include "BDeV/Core/Container/BvVector.h"
//#include "BDeV/Core/System/Threading/BvThread.h"
//#include "BDeV/Core/System/Threading/BvFiber.h"
//#include "BDeV/Core/System/Threading/BvSync.h"
//#include "BDeV/Core/System/Process/BvProcess.h"
//#include "BvRingBuffer.h"
//
//
//namespace JS
//{
//	constexpr u32 kJobCountPerPool = 400;
//	constexpr u32 kSuspendedJobCountPerPool = 32;
//	constexpr u32 kCounterPoolCount = 1000;
//
//	class Counter
//	{
//	public:
//		Counter() {}
//		~Counter() {}
//
//		bool Acquire()
//		{
//			return m_Acquired.load() == false && !m_Acquired.exchange(true);
//		}
//
//		void Release()
//		{
//			m_Acquired.exchange(false);
//		}
//
//		void Decrement()
//		{
//			m_Value.fetch_sub(1);
//		}
//
//		void Increment(u32 val)
//		{
//			m_Value.fetch_add(val);
//		}
//
//		bool IsDone() const
//		{
//			return m_Value.load() == 0;
//		}
//
//	private:
//		std::atomic<u32> m_Value{0};
//		std::atomic<bool> m_Acquired{};
//	};
//
//	struct JobData
//	{
//		Job m_Job;
//		Counter* m_pCounter;
//	};
//
//	struct ThreadData
//	{
//		ThreadData() = default;
//		ThreadData(const ThreadData&) {}
//
//		BvThread m_Thread;
//		BvSignal m_WorkerSignal;
//		struct FiberData* m_pFiberData = nullptr;
//		Counter* m_pMainThreadWaitCounter = nullptr;
//	};
//
//	struct FiberData
//	{
//		FiberData() = default;
//		FiberData(const FiberData&) {}
//
//		enum class Status : u8
//		{
//			kAvailable,
//			kInUse,
//			kInWaitList
//		};
//
//		JobData* m_pJobData = nullptr;
//		Counter* m_pWaitCounter = nullptr;
//		ThreadData* m_pThreadData = nullptr;
//		BvFiber m_Fiber;
//		std::atomic<Status> m_Status;
//	};
//
//	struct SuspendedJobData
//	{
//		FiberData* m_pFiberData = nullptr;
//		BvSpinlock m_Lock;
//	};
//
//	struct JobPool
//	{
//		BvRingBuffer<JobData, kJobCountPerPool> m_Jobs;
//		SuspendedJobData m_SuspendedJobs[kSuspendedJobCountPerPool];
//	};
//
//
//	JobPool g_JobPools[3];
//	Counter g_CounterPool[kCounterPoolCount];
//
//
//	class BvJobSystem
//	{
//		BV_NOCOPYMOVE(BvJobSystem);
//
//	public:
//		BvJobSystem();
//		~BvJobSystem();
//
//		void Initialize(const JobSystemDesc& jobSystemDesc = JobSystemDesc());
//		void Shutdown();
//
//		void RunJobs(u32 count, const Job* pJobs, Counter*& pCounter);
//		void WaitForCounter(Counter*& pCounter);
//		Counter* AllocCounter();
//		void FreeCounter(Counter*& pCounter);
//
//	private:
//		void WorkerThreadFunction(u32 index);
//		FiberData* GetJob(ThreadData* pThreadData);
//		void AddToWaitList(FiberData* pFiberData);
//		void FiberFunction(u32 fiberIndex);
//
//	private:
//		//BvVector<Counter> m_CounterPool;
//		//BvVector<JobPool> m_JobPools;
//		BvVector<ThreadData> m_WorkerThreads;
//		BvVector<FiberData> m_FiberPool;
//		std::atomic<bool> m_Active{};
//	};
//
//
//	BvJobSystem::BvJobSystem()
//	{
//	}
//
//
//	BvJobSystem::~BvJobSystem()
//	{
//	}
//
//
//	void BvJobSystem::Initialize(const JobSystemDesc& jobSystemDesc)
//	{
//		m_Active.store(true);
//
//		// Become a fiber
//		BvThread::ConvertToFiber();
//
//		// Spawn worker threads
//		m_WorkerThreads.Resize(jobSystemDesc.m_NumWorkerThreads + 1);
//		for (auto i = 1u; i <= jobSystemDesc.m_NumWorkerThreads; i++)
//		{
//			m_WorkerThreads[i].m_Thread = BvThread([this, i]()
//				{
//					// Set TLS data
//					SetWorkerThreadIndex(i);
//					
//					// Set processor affinity
//					BvThread::GetCurrentThread().SetAffinityMask(i - 1);
//
//					// Convert to fiber
//					BvThread::ConvertToFiber();
//
//					// Work
//					WorkerThreadFunction(i);
//
//					// Cleanup fiber resources
//					BvThread::ConvertFromFiber();
//				});
//		}
//
//		SetWorkerThreadIndex(0);
//
//		// Initialize obj pools
//		//for (auto& jobPool : m_JobPools)
//		//{
//		//	jobPool.m_Jobs.Resize(jobSystemDesc.m_JobPoolSize);
//		//	jobPool.m_SuspendedJobs.Resize(jobSystemDesc.m_FiberPoolSize);
//		//}
//
//		//m_CounterPool.Resize(jobSystemDesc.m_JobPoolSize);
//
//		m_FiberPool.Resize(jobSystemDesc.m_FiberPoolSize);
//		for (auto i = 0u; i < m_FiberPool.Size(); ++i)
//		{
//			m_FiberPool[i].m_Fiber = BvFiber(jobSystemDesc.m_FiberStackSize, [this, i]()
//				{
//					FiberFunction(i);
//				});
//		}
//	}
//
//
//	void BvJobSystem::Shutdown()
//	{
//		m_Active.store(false);
//		for (auto i = 1u; i < m_WorkerThreads.Size(); i++)
//		{
//			// Notify worker threads
//			m_WorkerThreads[i].m_WorkerSignal.Set();
//			// Wait for worker threads to finish
//			m_WorkerThreads[i].m_Thread.Wait();
//		}
//		m_WorkerThreads.Clear();
//		m_FiberPool.Clear();
//
//		BvThread::ConvertFromFiber();
//	}
//
//
//	void BvJobSystem::RunJobs(u32 count, const Job* pJobs, Counter*& pCounter)
//	{
//		if (pCounter == nullptr)
//		{
//			pCounter = AllocCounter();
//		}
//
//		for (u32 i = 0; i < count; ++i)
//		{
//			auto jobPoolIndex = 2 - (u32)(*pJobs).m_Priority;
//			auto& jobPool = g_JobPools[jobPoolIndex];
//			JobData jobData{ pJobs[i], pCounter };
//			jobPool.m_Jobs.Push(jobData);
//		}
//		pCounter->Increment(count);
//		
//		// Notify worker threads
//		for (auto i = 1u; i < m_WorkerThreads.Size(); i++)
//		{
//			m_WorkerThreads[i].m_WorkerSignal.Set();
//		}
//	}
//
//
//	void BvJobSystem::WaitForCounter(Counter*& pCounter)
//	{
//		if (pCounter->IsDone())
//		{
//			return;
//		}
//
//		auto workerThreadIndex = GetWorkerThreadIndex();
//		if (workerThreadIndex >= 0)
//		{
//			// Worker / Main thread
//			auto pWorkerThread = &m_WorkerThreads[workerThreadIndex];
//			auto pFiberData = pWorkerThread->m_pFiberData;
//
//			// If we have a fiber, that means we're at that fiber's stack currently,
//			// so we switch back. The case we may not have a fiber is if we're in
//			// the thread that created the job system and issue a wait call, so
//			// in that case we just call the worker function.
//			if (pFiberData)
//			{
//				pFiberData->m_pWaitCounter = pCounter;
//				// Switch back to the worker thread fiber
//				pFiberData->m_Fiber.Switch(BvThread::GetCurrentThread().GetThreadFiber());
//			}
//			else
//			{
//				pWorkerThread->m_pMainThreadWaitCounter = pCounter;
//				WorkerThreadFunction(workerThreadIndex);
//				pWorkerThread->m_pMainThreadWaitCounter = nullptr;
//				pWorkerThread->m_pFiberData = nullptr;
//			}
//		}
//		else
//		{
//			// System-External thread
//			while (!pCounter->IsDone())
//			{
//				BvCPU::Yield();
//			}
//		}
//	}
//
//
//	JS::Counter* BvJobSystem::AllocCounter()
//	{
//		for (auto i = 0; i < kCounterPoolCount; ++i)
//		{
//			if (g_CounterPool[i].Acquire())
//			{
//				return &g_CounterPool[i];
//			}
//		}
//
//		return nullptr;
//	}
//
//
//	void BvJobSystem::FreeCounter(Counter*& pCounter)
//	{
//		pCounter->Release();
//		pCounter = nullptr;
//	}
//
//
//	void BvJobSystem::WorkerThreadFunction(u32 workerThreadIndex)
//	{
//		auto& worker = m_WorkerThreads[workerThreadIndex];
//
//		while (m_Active.load())
//		{
//			if (workerThreadIndex > 0)
//			{
//				// Wait for new jobs
//				worker.m_WorkerSignal.Wait();
//			}
//
//			FiberData* pFiberData = nullptr;
//			// Run until there are no more jobs
//			do
//			{
//				pFiberData = GetJob(&worker);
//				if (pFiberData)
//				{
//					// We switch to the fiber and let it execute the job. After finishing the job
//					// or if at any point there's a wait call inside the function, the running
//					// fiber will switch back to the worker thread that activated it.
//					worker.m_Thread.GetThreadFiber().Switch(pFiberData->m_Fiber);
//
//					// When we return from the fiber, we check if it needs to be put on the wait list;
//					// if not, then we send it back to the pool
//					if (pFiberData->m_pWaitCounter)
//					{
//						// Send the fiber to the wait list
//						AddToWaitList(pFiberData);
//					}
//					else
//					{
//						// Update the fiber's status
//						pFiberData->m_Status.store(FiberData::Status::kAvailable);
//					}
//				}
//
//				if (workerThreadIndex == 0 && worker.m_pMainThreadWaitCounter->IsDone())
//				{
//					return;
//				}
//			} while (pFiberData != nullptr);
//		}
//	}
//
//
//	FiberData* BvJobSystem::GetJob(ThreadData* pThreadData)
//	{
//		JobData* pJobData = nullptr;
//		for (auto poolIndex = 0; poolIndex < 3; ++poolIndex)
//		{
//			if (g_JobPools[poolIndex].m_Jobs.Pop(pJobData))
//			{
//				// If we found a job in the pool, then we look for a fiber to assign it to
//				// TODO: Think of a better way to deal with this when there're no available fibers
//				while (true)
//				{
//					for (auto i = 0; i < m_FiberPool.Size(); ++i)
//					{
//						auto pFiberData = &m_FiberPool[i];
//						auto exp = FiberData::Status::kAvailable;
//						if (pFiberData->m_Status.load(std::memory_order::relaxed) == exp
//							&& pFiberData->m_Status.compare_exchange_strong(exp, FiberData::Status::kInUse))
//						{
//							pFiberData->m_pJobData = pJobData;
//							pFiberData->m_pThreadData = pThreadData;
//							pThreadData->m_pFiberData = pFiberData;
//							return pFiberData;
//						}
//					}
//
//					BvCPU::Yield();
//				}
//			}
//			else
//			{
//				// If we couldn't get a job from the pool, check the wait list
//				for (auto suspendedFiberIndex = 0u; suspendedFiberIndex < kSuspendedJobCountPerPool; ++suspendedFiberIndex)
//				{
//					auto& suspendedJob = g_JobPools[poolIndex].m_SuspendedJobs[suspendedFiberIndex];
//					if (suspendedJob.m_Lock.TryLock())
//					{
//						auto pFiberData = suspendedJob.m_pFiberData;
//						if (pFiberData && pFiberData->m_pWaitCounter->IsDone())
//						{
//							pFiberData->m_pThreadData = pThreadData;
//							pThreadData->m_pFiberData = pFiberData;
//
//							suspendedJob.m_pFiberData = nullptr;
//							suspendedJob.m_Lock.Unlock();
//
//							return pFiberData;
//						}
//						suspendedJob.m_Lock.Unlock();
//					}
//				}
//			}
//		}
//
//		return nullptr;
//	}
//
//
//	void BvJobSystem::AddToWaitList(FiberData* pFiberData)
//	{
//		auto poolIndex = 2 - (u32)pFiberData->m_pJobData->m_Job.m_Priority;
//		for (auto i = 0; i < kSuspendedJobCountPerPool; i++)
//		{
//			auto& suspendedJob = g_JobPools[poolIndex].m_SuspendedJobs[i];
//			if (suspendedJob.m_Lock.TryLock())
//			{
//				if (!suspendedJob.m_pFiberData)
//				{
//					suspendedJob.m_pFiberData = pFiberData;
//				}
//				suspendedJob.m_Lock.Unlock();
//			}
//		}
//
//		// Notify worker threads
//		for (auto i = 1u; i < m_WorkerThreads.Size(); i++)
//		{
//			m_WorkerThreads[i].m_WorkerSignal.Set();
//		}
//	}
//
//
//	void BvJobSystem::FiberFunction(u32 fiberIndex)
//	{
//		auto pFiberData = &m_FiberPool[fiberIndex];
//
//		while (m_Active.load())
//		{
//			auto pJobData = pFiberData->m_pJobData;
//			// Run the job
//			pJobData->m_Job.m_Job.Run();
//			// Update the job counter
//			pJobData->m_pCounter->Decrement();
//			pFiberData->m_pJobData = nullptr;
//			pFiberData->m_pWaitCounter = nullptr;
//
//			// Switch back to the worker thread
//			pFiberData->m_Fiber.Switch(pFiberData->m_pThreadData->m_Thread.GetThreadFiber());
//		}
//
//		pFiberData->m_Fiber.Switch(pFiberData->m_pThreadData->m_Thread.GetThreadFiber());
//	}
//
//
//	static BvJobSystem s_JobSystem;
//
//
//	void Initialize(const JobSystemDesc& jobSystemDesc)
//	{
//		s_JobSystem.Initialize(jobSystemDesc);
//	}
//
//
//	void Shutdown()
//	{
//		s_JobSystem.Shutdown();
//	}
//
//
//	void RunJob(const Job& job, Counter*& pCounter)
//	{
//		s_JobSystem.RunJobs(1, &job, pCounter);
//	}
//
//
//	void RunJobs(u32 count, const Job* pJobs, Counter*& pCounter)
//	{
//		s_JobSystem.RunJobs(count, pJobs, pCounter);
//	}
//
//
//	void WaitForCounter(Counter* pCounter)
//	{
//		s_JobSystem.WaitForCounter(pCounter);
//	}
//
//
//	void FreeCounter(Counter*& pCounter)
//	{
//		s_JobSystem.FreeCounter(pCounter);
//	}
//}