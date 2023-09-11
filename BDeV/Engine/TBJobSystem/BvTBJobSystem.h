#pragma once


#include "BDeV/System/Debug/BvDebug.h"
#include "BDeV/System/Threading/BvThread.h"
#include "BDeV/System/Threading/BvFiber.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/System/Threading/BvProcess.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Container/BvVector.h"


class BvTBJobList;


struct JobSystemDesc
{
	enum class Parallelism : u8
	{
		kUseCoreCount,		// Use one thread per core
		kUseThreadCount,	// Use as many threads as the CPU cores support
		kCustom,			// Use the specified amount in m_NumThreads
		kSingleThreaded		// Run all tasks in the same thread
	};

	static constexpr const u32 kDefaultJobListPoolSize = 200;

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


class BvTBJobSystemWorker;


class BvTBJobSystem
{
	BV_NOCOPYMOVE(BvTBJobSystem);

public:
	friend class BvTBJobSystemInstance;

	void Initialize(const JobSystemDesc desc = JobSystemDesc());
	void Shutdown();

	BvTBJobList* AllocJobList(const u32 jobListSize, JobListPriority priority = JobListPriority::kNormal);
	void FreeJobList(BvTBJobList* pJobList);
	void Submit(BvTBJobList* pJobList);
	void Wait();

	BV_INLINE bool IsActive() const { return m_IsActive; }

private:
	BvTBJobSystem() {}
	~BvTBJobSystem() {}

private:
	BvVector<BvTBJobList> m_JobListPool;
	BvVector<BvTBJobSystemWorker *> m_Workers;
	bool m_IsActive{};
};


extern BvTBJobSystem* g_pTBJobSystem;


class BvTBJobList
{
	static constexpr BvTBJobList* kSyncPoint = (BvTBJobList*)1;

	friend class BvTBJobSystem;
	friend class BvTBJobSystemWorker;

	using JobDataType = std::pair<BvTaskT<56>, BvTBJobList*>;

private:
	BvTBJobList() = default;
	//BvTBJobList(const BvTBJobList&) {}
	BvTBJobList(u32 maxJobs, JobListPriority priority = JobListPriority::kNormal)
		: m_Priority(priority)
	{
		m_Jobs.Reserve(maxJobs);
	}
	~BvTBJobList() {}

public:
	template<typename Fn, typename... Args>
	void AddJob(Fn&& fn, Args&&... args)
	{
		BvAssert(m_Jobs.Size() < m_Jobs.Capacity(), "Job list is already full");
		auto& job = m_Jobs.EmplaceBack();
		job.first.Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
		job.second = nullptr;
	}

	void AddSyncPoint()
	{
		BvAssert(m_Jobs.Size() < m_Jobs.Capacity(), "Job list is already full");
		auto& job = m_Jobs.EmplaceBack();
		job.second = kSyncPoint;
	}

	void AddDependency(BvTBJobList* pDependency)
	{
		BvAssert(m_Jobs.Size() < m_Jobs.Capacity(), "Job list is already full");
		auto& job = m_Jobs.EmplaceBack();
		job.second = pDependency;
	}

	void Submit()
	{
		g_pTBJobSystem->Submit(this);
	}

	bool IsDone() const
	{
		return m_JobsDone.load(std::memory_order::relaxed) >= m_Jobs.Size();
	}

	void Wait() const
	{
		while (!IsDone())
		{
			BvProcess::YieldExecution();
		}
	}

	void Reset(bool resetJobs = false)
	{
		if (resetJobs)
		{
			m_Jobs.Clear();
		}

		m_CurrJob.store(0);
		m_JobsDone.store(0);
	}

	JobListPriority GetPriority() const
	{
		return m_Priority;
	}

	bool IsWaitingForDependency() const
	{
		auto index = m_CurrJob.load(std::memory_order::relaxed);
		if (index < m_Jobs.Size())
		{
			return m_Jobs[index].second != nullptr && m_Jobs[index].second != kSyncPoint;
		}

		return false;
	}

private:
	BvVector<JobDataType> m_Jobs;
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_JobsDone;
	JobListPriority m_Priority{};
	std::atomic<bool> m_Lock;
	std::atomic<bool> m_InUse;
};