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
		kUseThreadCount,
		kUseCoreCount,
		kCustom,
		kSingleThreaded // For debugging purposes only
	};

	static constexpr const u32 kDefaultJobListPoolSize = 200;

	u32 m_JobListPoolSize = kDefaultJobListPoolSize;
	u32 m_NumThreads = 0;
	u32* m_pThreadCores = nullptr;
	Parallelism m_Parallelism = Parallelism::kUseThreadCount;
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
	void Submit(BvTBJobList* pJobList);

	BV_INLINE bool IsActive() const { return m_IsActive.load() == true; }

private:
	BvVector<BvTBJobList*> m_JobListPool;
	BvVector<BvTBJobSystemWorker *> m_Workers;
	std::atomic<u32> m_JobListCount;
	std::atomic<bool> m_IsActive;
};


class BvTBJobList
{
	BV_NOCOPYMOVE(BvTBJobList);

	static constexpr BvTBJobList* kSyncPoint = (BvTBJobList*)1;
	static constexpr u32 kMaxJobs = 32;

	friend class BvTBJobSystem;
	friend class BvTBJobSystemWorker;

	using JobDataType = std::pair<BvJobT<56>, BvTBJobList*>;

public:
	template<typename Fn, typename... Args>
	void AddJob(Fn&& fn, Args &&... args)
	{
		BvAssert(m_Jobs.Size() < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		m_Jobs.EmplaceBack(JobDataType());
		m_Jobs[m_Jobs.Size() - 1].first.Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
		m_Jobs[m_Jobs.Size() - 1].second = nullptr;
	}

	void AddSyncPoint()
	{
		BvAssert(m_Jobs.Size() < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		m_Jobs.EmplaceBack(JobDataType());
		m_Jobs[m_Jobs.Size() - 1].second = kSyncPoint;
	}

	void AddDependency(BvTBJobList* pDependency)
	{
		BvAssert(m_Jobs.Size() < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		m_Jobs.EmplaceBack(JobDataType());
		m_Jobs[m_Jobs.Size() - 1].second = pDependency;
	}

	void Submit()
	{
		m_pJobSystem->Submit(this);
	}

	bool TryWait() const
	{
		return m_JobsDone.load() >= m_Jobs.Size();
	}

	void Reset()
	{
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
		if (index < m_Jobs.Size())
		{
			return m_Jobs[index].second != nullptr && m_Jobs[index].second != kSyncPoint;
		}

		return false;
	}
	
private:
	BvTBJobList(BvTBJobSystem* pJobSystem, const JobListPriority priority)
		: m_pJobSystem(pJobSystem), m_Priority(priority)
	{
		m_Jobs.Reserve(kMaxJobs);
	}
	~BvTBJobList() {}

private:
	BvTBJobSystem* m_pJobSystem;
	BvVector<JobDataType> m_Jobs;
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_JobsDone;
	BvSpinlock m_Lock;
	JobListPriority m_Priority;
};