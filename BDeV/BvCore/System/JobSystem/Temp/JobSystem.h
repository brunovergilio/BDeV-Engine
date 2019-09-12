#pragma once


// iD Tech's job system
// I started working on something that would resemble iD Tech's system but in the end it became a copy of it
// Switched to my own Job System


//#include "../System/BvThread.h"
//#include "../System/BvFiber.h"
//#include "../System/BvSync.h"
//#include "../Utils/BvUtils.h"
//
//
//constexpr u32 kMaxJobLists = 32;
//constexpr u32 kMaxJobs = 1024;
//constexpr u32 kMaxCheckpoints = 8;
//constexpr u32 kMaxWaitGuards = 4;
//
//
//typedef void(*pFnJob)(void *);
//
//
//struct Job
//{
//	Job() {}
//	Job(pFnJob pFunction, void * pData = nullptr)
//		: pFunction(pFunction), pData(pData) {}
//
//	pFnJob pFunction = nullptr;
//	void * pData = nullptr;
//};
//
//
//enum class JobPriority
//{
//	kLow,
//	kMedium,
//	kHigh
//};
//
//
//class JobQueue
//{
//public:
//	friend class JobThread;
//	friend class JobSystem;
//
//	~JobQueue() {}
//
//	void Add(const Job & job);
//	void Add(pFnJob pFunction, void * pData = nullptr);
//	void AddCheckpoint();
//	void AddBarrier();
//	void Submit(JobQueue * const pQueueToWaitFor = nullptr);
//	void WaitAndReset();
//
//	BV_INLINE const JobPriority GetPriority() const { return m_Priority; }
//	BV_INLINE const bool WasSubmitted() const { return !m_Done; }
//	BV_INLINE const bool DependsOnAnotherQueue() const { return m_pJobQueueGuardToWait && m_pJobQueueGuardToWait->GetValue() > 0; }
//
//private:
//	explicit JobQueue(const JobPriority priority = JobPriority::kLow)
//		: m_Priority(priority) {}
//	struct ThreadStateData;
//	i32 Run(const u32 threadIndex, ThreadStateData & stateData, const bool runAll);
//	i32 RunInternal(const u32 threadIndex, ThreadStateData & stateData, const bool runAll);
//	BV_INLINE static void Nop(void *) {}
//
//private:
//	enum JobQueueResult
//	{
//		kOk = 0,
//		kInProgress = 1,
//		kStalling = 2,
//		kDone = 4
//	};
//	struct ThreadStateData
//	{
//		JobQueue * pJobQueue = nullptr;
//		i32 lastJob = 0;
//		i32 nextJob = -1;
//		i32 lastSignal = 0;
//		i32 version = 0;
//	};
//
//	BvAtomic m_FetchLock;
//	BvAtomic m_CurrJob;
//	BvAtomic m_NumThreadsRunning;
//	BvAtomic m_Version;
//	Job m_Jobs[kMaxJobs]{};
//	BvAtomic m_Checkpoints[kMaxCheckpoints];
//	BvAtomic m_waitGuards[kMaxWaitGuards];
//	BvAtomic * m_pJobQueueGuardToWait = nullptr;
//	u32 m_JobCount = 0;
//	u32 m_CheckPointCount = 0;
//	u32 m_LastSignal = 0;
//	u32 m_CurrWaitGuard = 0;
//	JobPriority m_Priority = JobPriority::kLow;
//	bool m_HasCheckpoint = false;
//	bool m_Done = true;
//
//	static u32 s_Checkpoint;
//	static u32 s_Barrier;
//	static u32 s_QueueFinished;
//};
//
//
//class JobThread
//{
//public:
//	friend class JobSystem;
//
//	JobThread();
//	~JobThread();
//
//private:
//	void Start(const u32 index);
//	void Stop();
//	void AddJobQueue(JobQueue * const pJobQueue);
//	void Process();
//	static void Thread(void * pData);
//
//private:
//	JobQueue * m_pJobQueues[kMaxJobLists]{};
//	i32 m_JobQueueVersions[kMaxJobLists]{};
//	BvThread m_Thread;
//	BvSpinlock m_JobListLock;
//	BvMutex m_WorkSignalLock;
//	BvEvent m_WorkSignal;
//	u32 m_FirstJobList = 0;
//	u32 m_LastJobList = 0;
//	u32 m_Index = UINT32_MAX;
//	u32 m_TotalJobsRun = 0;
//};
//
//
//class BV_SINGLETON(JobSystem)
//{
//public:
//	JobSystem();
//	~JobSystem();
//
//	void Initialize();
//	void Shutdown();
//	void Submit(JobQueue * const pJobQueue);
//
//	JobQueue * AllocQueue();
//	void FreeJobQueue(JobQueue * & pQueue);
//
//	BV_INLINE bool IsActive() { return m_Active.Load() > 0; }
//	
//	BV_INLINE static JobSystem * const GetInstance() { return s_pJobSystem; }
//
//private:
//	static JobSystem * s_pJobSystem;
//
//	JobThread * m_pJobThreads = nullptr;
//	BvAtomic m_Active;
//};