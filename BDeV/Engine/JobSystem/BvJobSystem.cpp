#include "BvJobSystem.h"
#include "BvJobSystemTLS.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/System/Threading/BvThread.h"
#include "BDeV/System/Threading/BvFiber.h"
#include "BDeV/System/Threading/BvSync.h"


namespace JS
{
	class JobCounter
	{
	public:
		JobCounter() {}
		JobCounter(const JobCounter&) : m_Value(0), m_Version(0) {}
		JobCounter& operator=(const JobCounter&) { return *this; }

		void Update()
		{
			m_Value.fetch_sub(1);
		}

		void Set(i32 value)
		{
			m_Version++;
			m_Value.store(value);
		}

		BV_INLINE const bool IsDone(i32 currVersion) const
		{
			return currVersion != m_Version || m_Value.load() == 0;
		}

	private:
		std::atomic<i32> m_Value{};
		i32 m_Version = 0;
	};


	class BvJobSystem
	{
		BV_NOCOPYMOVE(BvJobSystem);

	public:
		BvJobSystem();
		~BvJobSystem();

		void Initialize(const JobSystemDesc& jobSystemDesc = JobSystemDesc());
		void Shutdown();

		JobCounterHandle RunJob(const Job& job);
		JobCounterHandle RunJobs(u32 count, const Job* const pJobs);
		void WaitForCounter(const JobCounterHandle& jobCounter);

		void WorkerFunction();
		static void FiberFunction(void* pData);

		const BvVector<JobCounter>& GetCounters() const { return m_Counters; }

	private:
		struct JobItem
		{
			class JobCounterData* m_pJobCounter = nullptr;
			Job m_Job{};
		};

		struct ThreadData
		{
			BvJobSystem* m_pJobSystem = nullptr;
			BvThread m_Thread;
			i32 m_CurrFiberIndex = -1;
		};

		struct FiberData
		{
			BvJobSystem* m_pJobSystem = nullptr;
			BvFiber m_Fiber;
		};

		BvVector<JobItem> m_JobItems[4]{};
		BvVector<ThreadData> m_ThreadData;
		BvVector<JobCounter> m_Counters;
		BvSignal m_WorkerSignal;
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
		m_WorkerSignal = BvSignal(false);
		// TODO: become a fiber
		// TODO: spawn worker threads
		m_Active.store(true);
	}


	void BvJobSystem::Shutdown()
	{
		m_Active.store(false);
		//for (auto&& workerThread : m_)
		// TODO: notify worker threads
		// TODO: wait for worker threads to finish
		// TODO: free up fiber / thread pools
	}


	JobCounterHandle BvJobSystem::RunJob(const Job& job)
	{
		return RunJobs(1, &job);
	}


	JobCounterHandle BvJobSystem::RunJobs(u32 count, const Job* const pJobs)
	{
		// TODO: lock the joblist
		// TODO: get a job
		// TODO: update the index
		// TODO: notify worker threads

		return JobCounterHandle();
	}


	void BvJobSystem::WaitForCounter(const JobCounterHandle& jobCounter)
	{
		if (jobCounter.IsDone())
		{
			return;
		}

		if (GetWorkerThreadIndex() >= 0)
		{
			// Main / Worker thread

			// TODO: do other work
		}
		else
		{
			// System-External thread

			// TODO: spin-wait
		}
	}


	void BvJobSystem::WorkerFunction()
	{
		while (m_Active.load())
		{
			// TODO: * wait for new jobs
			// TODO: * run until there are no more jobs
			// TODO: ** get a fiber from the pool if current fiber is null
			// TODO: ** try to get a job
			// TODO: *** if succeeded switch to the fiber and run the job
			// TODO: **** upon returning from the fiber, check result
			// TODO: **** if a wait request was made, put the fiber on wait mode, and set current fiber to null
			// TODO: *** else return fiber to the pool
		}
	}


	void BvJobSystem::FiberFunction(void* pData)
	{
		auto pFiberData = reinterpret_cast<FiberData*>(pData);
		while (pFiberData->m_pJobSystem->m_Active.load())
		{
			// TODO: * run job
			// TODO: * switch back to current thread's fiber
		}
		// TODO: switch back to current thread's fiber
	}


	const bool JobCounterHandle::IsDone() const
	{
		auto& counter = s_JobSystem.GetCounters()[m_CounterIndex];
		return counter.IsDone(m_Version);
	}


	void Initialize(const JobSystemDesc& jobSystemDesc)
	{
		s_JobSystem.Initialize(jobSystemDesc);
	}


	void Shutdown()
	{
		s_JobSystem.Shutdown();
	}


	JobCounterHandle RunJob(const Job& job)
	{
		return s_JobSystem.RunJob(job);
	}


	JobCounterHandle RunJobs(u32 count, const Job* const pJobs)
	{
		return s_JobSystem.RunJobs(count, pJobs);
	}


	void WaitForCounter(const JobCounterHandle& jobCounter)
	{
		s_JobSystem.WaitForCounter(jobCounter);
	}
}