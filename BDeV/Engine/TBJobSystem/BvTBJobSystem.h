#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Threading/BvThread.h"
#include "BDeV/Core/System/Threading/BvFiber.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/System/Process/BvProcess.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvVector.h"


class BvTBJobSystem;
class BvTBJobSystemWorker;


class BvTBJobList
{
	friend BvTBJobSystem;
	friend BvTBJobSystemWorker;

	enum class JobSyncType : u8
	{
		kNone,
		kSyncPoint,
		kJobListDependency,
	};

	struct alignas(64) Job
	{
		BvTaskT<48> m_Job{};
		BvTBJobList* m_pDependency{};
		JobSyncType m_SyncType{};
	};

public:
	enum class Priority : u8
	{
		kLow,
		kNormal,
		kHigh
	};

	enum class Category : u8
	{
		kAny,
		kGeneral,
		kRender,
		kGameplay,
		kPhysics,
		kAudio,
	};

	template<typename Fn, typename... Args>
	void AddJob(Fn&& fn, Args&&... args)
	{
		BvAssert(m_Jobs.Size() < m_Jobs.Capacity(), "JobList is full");
		auto& job = AddInternal();
		job.m_Job.Set(std::forward<Fn>(fn), std::forward<std::decay_t<Args>>(args)...);
	}

	void AddSyncPoint();
	void AddJobListDependency(BvTBJobList* pDependency);
	void Reset(bool resetJobs = false);

	void Submit(bool threaded = true);

	bool IsDone() const;
	bool IsWaitingForDependency() const;
	void Wait() const;

	Priority GetPriority() const;
	Category GetCategory() const;

private:
	BvTBJobList(BvTBJobSystem* pJobSystem, u32 maxJobs, u32 maxSyncs, Priority priority);
	~BvTBJobList();

	Job& AddInternal();

private:
	BvTBJobSystem* m_pJobSystem = nullptr;
	BvVector<Job> m_Jobs;
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_JobsDone;
	std::atomic_flag m_Lock;
	Priority m_Priority = Priority::kNormal;
	Category m_Category = Category::kAny;
};

BV_USE_ENUM_CLASS_OPERATORS(BvTBJobList::Category);


struct JobSystemDesc
{
	struct WorkerThreadDesc
	{
		u64 m_CoreIndex = 0;
		BvTBJobList::Category m_Category = BvTBJobList::Category::kAny;
		const char* m_pName = nullptr;
	};

	enum class Parallelism : u8
	{
		kUseCoreCount,		// Use one thread per core
		kUseThreadCount,	// Use as many threads as the CPU cores support
		kCustom,			// Use the specified amount in m_NumThreads
		kSingleThreaded		// Run all tasks in the same thread (for debugging)
	};

	static constexpr u32 kDefaultMaxJobLists = 16;
	static constexpr u32 kDefaultMaxJobs = 256;

	u32 m_JobListPoolSize = kDefaultMaxJobLists;
	u32 m_JobPoolSize = kDefaultMaxJobs;
	Parallelism m_Parallelism = Parallelism::kUseCoreCount;
	u32 m_NumWorkerThreads = 0;
	WorkerThreadDesc* m_pWorkerThreadDescs = nullptr;
};


class BvTBJobSystem
{
	BV_NOCOPYMOVE(BvTBJobSystem);

	BvTBJobSystem();
	~BvTBJobSystem();

	friend class BvTBJobSystemInstance;

public:
	void Initialize(const JobSystemDesc& desc = JobSystemDesc());
	void Shutdown();

	BvTBJobList* AllocJobList(u32 maxJobs, u32 maxSyncs, BvTBJobList::Priority priority = BvTBJobList::Priority::kNormal);
	void FreeJobList(BvTBJobList*& pJobList);
	void Submit(BvTBJobList* pJobList);
	void Wait();

private:
	BvVector<BvTBJobSystemWorker*> m_Workers;
	BvVector<BvTBJobList*> m_JobLists;
};