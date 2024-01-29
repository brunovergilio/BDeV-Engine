#pragma once


#include "BDeV/System/Debug/BvDebug.h"
#include "BDeV/System/Threading/BvThread.h"
#include "BDeV/System/Threading/BvFiber.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/System/Threading/BvProcess.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Container/BvVector.h"


struct JobSystemDesc
{
	enum class Parallelism : u8
	{
		kUseCoreCount,		// Use one thread per core
		kUseThreadCount,	// Use as many threads as the CPU cores support
		kCustom,			// Use the specified amount in m_NumThreads
		kSingleThreaded		// Run all tasks in the same thread
	};

	static constexpr const u32 kDefaultJobListPoolSize = 32;

	u32 m_JobListPoolSize = kDefaultJobListPoolSize;
	u32 m_NumThreads = 0;
	u32* m_pThreadCores = nullptr;
	Parallelism m_Parallelism = Parallelism::kUseCoreCount;
};


enum class JobListPriority : u8
{
	kLow,
	kNormal,
	kHigh
};

enum class JobListCategory : u8
{
	kGeneral,
	kRender,
	kLogic,
	kAny = kU8Max,
};

class BvTBJobList;
class BvTBJobSystemWorker;


class BvTBJobSystem
{
	BV_NOCOPYMOVE(BvTBJobSystem);

	BvTBJobSystem();
	~BvTBJobSystem();

	friend class BvTBJobSystemInstance;

public:
	void Initialize(const JobSystemDesc& desc = JobSystemDesc());
	void Shutdown();

	BvTBJobList* AllocJobList(u32 maxJobs, u32 maxSyncs, JobListPriority priority = JobListPriority::kNormal);
	void FreeJobList(BvTBJobList*& pJobList);
	void Submit(BvTBJobList* pJobList);
	void Wait();

private:
	BvVector<BvTBJobSystemWorker *> m_Workers;
};


extern BvTBJobSystem* g_pJobSystem;


class BvTBJobList
{
	friend class BvTBJobSystem;
	friend class BvTBJobSystemWorker;

	enum class JobSyncType : u8
	{
		kNone,
		kSyncPoint,
		kJobListDependency,
	};

	struct Job
	{
		BvTaskT<48> m_Job{};
		BvTBJobList* m_pDependency{};
		JobSyncType m_SyncType{};
	};

public:
	template<typename Fn, typename... Args>
	void AddJob(Fn&& fn, Args&&... args)
	{
		BvAssert(m_Jobs.Size() < m_Jobs.Capacity(), "JobList is full");
		auto& job = AddInternal();
		job.m_Job.Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	void AddSyncPoint();
	void AddJobListDependency(BvTBJobList* pDependency);
	void Reset(bool resetJobs = false);
	
	void Submit(bool threaded = true);
	
	bool IsDone() const;
	bool IsWaitingForDependency() const;
	void Wait() const;
	
	JobListPriority GetPriority() const;

private:
	BvTBJobList(u32 maxJobs, u32 maxSyncs, JobListPriority priority);
	~BvTBJobList();

	Job& AddInternal();

private:
	BvVector<Job> m_Jobs;
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_JobsDone;
	BvSpinlock m_Lock;
	JobListPriority m_Priority = JobListPriority::kNormal;
};