#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/Threading/BvThread.h"
#include "BDeV/Core/System/Threading/BvFiber.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include "BDeV/Core/System/Process/BvProcess.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/System/Memory/Allocators/BvHeapAllocator.h"
#include "BDeV/Core/System/Memory/Utilities/BvBoundsChecker.h"
#include "BDeV/Core/System/Memory/Utilities/BvMemoryMarker.h"
#include "BDeV/Core/System/Memory/Utilities/BvMemoryTracker.h"


class BvParallelJobSystem;
class BvParallelJobSystemWorker;


class alignas(kCacheLineSize) BvParallelJobList
{
	friend BvParallelJobSystem;
	friend BvParallelJobSystemWorker;

	enum class JobSyncType : u8
	{
		kNone,
		kSyncPoint,
		kJobListDependency,
	};

	struct alignas(kCacheLineSize) Job
	{
		BvTask<16> m_Job{};
		BvParallelJobList* m_pDependency{};
		JobSyncType m_SyncType{};
		const char* m_pName = nullptr;
	};

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

	template<typename Fn, typename... Args>
	void AddJob(Fn&& fn, Args&&... args)
	{
		BV_ASSERT(m_JobCount < m_MaxJobCount, "JobList is full");
		auto& job = AddInternal();
		job.m_Job.Set(std::forward<Fn>(fn), std::forward<std::decay_t<Args>>(args)...);
	}

	void AddSyncPoint();
	void AddJobListDependency(BvParallelJobList* pDependency);
	void Reset(bool resetJobs = false);

	void Submit();

	bool IsDone() const;
	bool IsWaitingForDependency() const;
	void Wait() const;

	BV_INLINE Priority GetPriority() const { return m_Priority; }
	BV_INLINE CategoryFlags GetCategoryFlags() const { return m_CategoryFlags; }

private:
	BvParallelJobList(BvParallelJobSystem* pJobSystem, Job* pJobs, u32 maxJobCount, Priority priority, CategoryFlags categoryFlags);
	~BvParallelJobList();

	Job& AddInternal();

private:
	BvParallelJobSystem* m_pJobSystem = nullptr;
	Job* m_pJobs = nullptr;
	u32 m_JobCount = 0;
	u32 m_MaxJobCount = 0;
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_JobsDone;
	std::atomic_flag m_Lock;
	Priority m_Priority = Priority::kNormal;
	CategoryFlags m_CategoryFlags = CategoryFlags::kAny;
};

BV_USE_ENUM_CLASS_OPERATORS(BvParallelJobList::CategoryFlags);


struct JobSystemDesc
{
	static constexpr u32 kDefaultMaxJobLists = 16;
	static constexpr u32 kDefaultMaxJobsAndSyncs = 256;
	static constexpr u32 kAutoSelectProcessor = kU32Max;

	struct WorkerThreadDesc
	{
		const char* m_pName = nullptr;
		u32 m_LogicalProcessorIndex = kAutoSelectProcessor;
		BvParallelJobList::CategoryFlags m_CategoryFlags = BvParallelJobList::CategoryFlags::kAny;
	};

	u32 m_JobListPoolSize = kDefaultMaxJobLists;
	u32 m_JobPoolSize = kDefaultMaxJobsAndSyncs;
	u32 m_NumWorkerThreadDescs = 0;
	WorkerThreadDesc* m_pWorkerThreadDescs = nullptr;
	IBvMemoryArena* m_pMemoryArena = nullptr;
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
	void Wait();
	const JobSystemWorkerStats& GetStats(u32 workerThreadIndex) const;

private:
	BvParallelJobSystemWorker* m_pWorkers = nullptr;
	u32 m_WorkerCount = 0;
	IBvMemoryArena* m_pMemoryArena = nullptr;
	void* m_pMemory = nullptr;
};