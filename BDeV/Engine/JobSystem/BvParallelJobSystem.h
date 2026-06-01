#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Threading/BvThread.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvStringId.h"


class BvParallelJobSystem;
class BvParallelJobSystemWorker;


struct alignas(kCacheLineSize) BvParallelJob
{
	BvCMTask<24> m_Job;
	BvStringId m_Name;
};


class alignas(kCacheLineSize) BvParallelJobList
{
	friend BvParallelJobSystem;
	friend BvParallelJobSystemWorker;

public:
	enum class Priority : u8
	{
		kNormal,
		kMedium,
		kHigh
	};

	enum class CategoryFlags : u8
	{
		kGeneral =		BvBit(0),
		kRender =		BvBit(1),
		kGameplay =		BvBit(2),
		kPhysics =		BvBit(3),
		kAI =			BvBit(4),
		kAudio =		BvBit(5),
		kAny =			kU8Max
	};

	template<typename Fn>
	void AddJob(Fn&& fn, const BvStringId& name)
	{
		BV_ASSERT(m_JobCount < m_MaxJobs, "JobList is full");
		auto& job = AddInternal();
		job.m_Job.Set(std::forward<Fn>(fn));
		job.m_Name = name;
	}

	void AddJob(const BvParallelJob& job);
	void AddSyncPoint(BvParallelJobList* pWaitJobList = nullptr);
	void Reset(bool resetJobs = false);

	bool IsDone() const;
	void Wait() const;

	BV_INLINE Priority GetPriority() const { return m_Priority; }
	BV_INLINE CategoryFlags GetCategoryFlags() const { return m_CategoryFlags; }

private:
	struct SyncPoint
	{
		const BvParallelJobList* m_pJobList;
		u32 m_Index;
	};

	BvParallelJobList();
	~BvParallelJobList();

	void Set(BvParallelJob* pJobs, SyncPoint* pSyncPoints, u32 maxJobs, u32 maxSyncPoints, Priority priority, CategoryFlags categoryFlags, bool temp);
	bool IsWaitingForJobList() const;
	BvParallelJob& AddInternal();

private:
	BvParallelJob* m_pJobs = nullptr;
	SyncPoint* m_pSyncPoints = nullptr;
	u32 m_JobCount = 0;
	u32 m_MaxJobs = 0;
	u32 m_SyncPointCount = 0;
	u32 m_MaxSyncPoints = 0;
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_CurrSyncPoint;
	std::atomic<u32> m_JobsDone;
	std::atomic<u32> m_LastSubmittedVersion{ kU32Max };
	std::atomic<u32> m_Version;
	std::atomic_flag m_Lock;
	Priority m_Priority = Priority::kNormal;
	CategoryFlags m_CategoryFlags = CategoryFlags::kAny;
};


BV_USE_ENUM_CLASS_OPERATORS(BvParallelJobList::CategoryFlags);


class BvParallelJobResult
{
	friend class BvParallelJobSystem;

public:
	bool IsDone() const;
	void Wait();

private:
	BvParallelJobResult(BvParallelJobList* pJobList);

private:
	BvParallelJobList* m_pJobList = nullptr;
};


struct JobSystemDesc
{
	static constexpr u32 kDefaultJobListPoolSize = 16;
	static constexpr u32 kDefaultJobPoolSize = 256;
	static constexpr u32 kAutoSelectProcessor = kU32Max;

	struct WorkerThreadDesc
	{
		const char* m_pName = nullptr;
		u32 m_LogicalProcessorIndex = kAutoSelectProcessor;
		BvParallelJobList::CategoryFlags m_CategoryFlags = BvParallelJobList::CategoryFlags::kAny;
	};

	u32 m_NumWorkerThreads = 0;
	WorkerThreadDesc* m_pWorkerThreadDescs = nullptr;
	IBvMemoryArena* m_pMemoryArena = BV_DEFAULT_MEMORY_ARENA;
	u32 m_JobListPoolSize = kDefaultJobListPoolSize;
	u32 m_JobPoolSize = kDefaultJobPoolSize;
};


struct JobSystemWorkerStats
{
	u64 m_TotalRunningTimeUs = 0;
	u64 m_ActiveTimeUs = 0;
	u64 m_TotalJobTimeUs = 0;
	u32 m_JobsRun = 0;
};


class BvParallelJobSystem
{
	BV_NOCOPYMOVE(BvParallelJobSystem);

public:
	BvParallelJobSystem();
	~BvParallelJobSystem();

	void Initialize(const JobSystemDesc& desc = JobSystemDesc());
	void Shutdown();

	BvParallelJobList* AllocJobList(u32 maxJobs, u32 maxSyncs, BvParallelJobList::Priority priority = BvParallelJobList::Priority::kNormal,
		BvParallelJobList::CategoryFlags categoryFlags = BvParallelJobList::CategoryFlags::kAny);
	void FreeJobList(BvParallelJobList*& pJobList);
	
	void Submit(BvParallelJobList* pJobList);
	BvParallelJobResult Submit(const BvParallelJob& job, BvParallelJobList::Priority priority = BvParallelJobList::Priority::kNormal,
		BvParallelJobList::CategoryFlags categoryFlags = BvParallelJobList::CategoryFlags::kAny);
	BvParallelJobResult Submit(u32 jobCount, const BvParallelJob* pJobs, BvParallelJobList::Priority priority = BvParallelJobList::Priority::kNormal,
		BvParallelJobList::CategoryFlags categoryFlags = BvParallelJobList::CategoryFlags::kAny);
	template<typename Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn>>>
	BvParallelJobResult Submit(Fn&& fn, BvParallelJobList::Priority priority = BvParallelJobList::Priority::kNormal,
		BvParallelJobList::CategoryFlags categoryFlags = BvParallelJobList::CategoryFlags::kAny)
	{
		BvParallelJob job;
		job.m_Job.Set(std::forward<Fn>(fn));
		return Submit(job, priority, categoryFlags);
	}

	void Wait();
	const JobSystemWorkerStats& GetStats(u32 workerThreadIndex) const;

private:
	void AcquireJobData(BvParallelJobList*& pJobList, BvParallelJob*& pJobs, u32 jobCount = 1);

private:
	BvParallelJob* m_JobPool = nullptr;
	BvParallelJobList* m_JobListPool = nullptr;
	u32 m_JobPoolSize = 0;
	u32 m_JobListPoolSize = 0;
	std::atomic<u32> m_CurrJobIndex;
	std::atomic<u32> m_CurrJobListIndex;
	BvParallelJobSystemWorker* m_pWorkers = nullptr;
	u32 m_WorkerCount = 0;
	IBvMemoryArena* m_pMemoryArena = nullptr;
	void* m_pMemory = nullptr;
};