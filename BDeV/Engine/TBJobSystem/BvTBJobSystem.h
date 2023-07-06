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
		kUseThreadCount,	// Use as many threads as the OS supports
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


class BvTBJobSystemWorker;


class BvTBJobSystem
{
	BV_NOCOPYMOVE(BvTBJobSystem);

public:
	BvTBJobSystem();
	~BvTBJobSystem();

	void Initialize(const JobSystemDesc desc = JobSystemDesc());
	void Shutdown();

	BvTBJobList* AllocJobList(const u32 jobListSize, JobListPriority priority = JobListPriority::kNormal);
	void FreeJobList(BvTBJobList* pJobList);
	void Submit(BvTBJobList* pJobList);

	BV_INLINE bool IsActive() const { return m_IsActive.load() == true; }

private:
	BvVector<BvTBJobList> m_JobListPool;
	BvVector<BvTBJobSystemWorker *> m_Workers;
	std::atomic<u32> m_JobListCount;
	std::atomic<bool> m_IsActive;
};


class BvTBJobList
{
	static constexpr BvTBJobList* kSyncPoint = (BvTBJobList*)1;
	static constexpr u32 kMaxJobs = 32;

	friend class BvTBJobSystem;
	friend class BvTBJobSystemWorker;

	using JobDataType = std::pair<BvTaskT<56>, BvTBJobList*>;

public:
	BvTBJobList() = default;
	BvTBJobList(const BvTBJobList&) {}
	BvTBJobList(BvTBJobSystem* pJobSystem, const JobListPriority priority)
		: m_pJobSystem(pJobSystem), m_Priority(priority) {}
	~BvTBJobList() {}

	template<typename Fn, typename... Args>
	void AddJob(Fn&& fn, Args &&... args)
	{
		BvAssert(m_JobCount < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		u32 jobIndex = m_JobCount++;
		m_Jobs[jobIndex] = JobDataType();
		m_Jobs[jobIndex].first.Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
		m_Jobs[jobIndex].second = nullptr;
	}

	void AddSyncPoint()
	{
		BvAssert(m_JobCount < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		u32 jobIndex = m_JobCount++;
		m_Jobs[jobIndex] = JobDataType();
		m_Jobs[jobIndex].second = kSyncPoint;
	}

	void AddDependency(BvTBJobList* pDependency)
	{
		BvAssert(m_JobCount < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		u32 jobIndex = m_JobCount++;
		m_Jobs[jobIndex] = JobDataType();
		m_Jobs[jobIndex].second = pDependency;
	}

	void Submit()
	{
		m_pJobSystem->Submit(this);
	}

	bool IsDone() const
	{
		return m_JobsDone.load() >= m_JobCount;
	}

	void Wait() const
	{
		while (!IsDone())
		{
			YieldProcessorExecution();
		}
	}

	void Reset()
	{
		m_JobCount = 0;
		m_CurrJob = 0;
		m_JobsDone = 0;
	}

	JobListPriority GetPriority() const
	{
		return m_Priority;
	}

	bool IsWaitingForDependency() const
	{
		auto index = m_CurrJob.load();
		if (index < m_JobCount)
		{
			return m_Jobs[index].second != nullptr && m_Jobs[index].second != kSyncPoint;
		}

		return false;
	}

private:
	BvTBJobSystem* m_pJobSystem{};
	JobDataType m_Jobs[kMaxJobs]{};
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_JobsDone;
	BvSpinlock m_Lock;
	u32 m_JobCount{};
	JobListPriority m_Priority;
	std::atomic_flag m_InUse;
};