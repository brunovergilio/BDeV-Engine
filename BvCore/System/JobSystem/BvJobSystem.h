#pragma once


#include "BvCore/Utils/BvDebug.h"
#include "BvCore/System/Threading/BvThread.h"
#include "BvCore/System/Threading/BvFiber.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/System/Threading/BvProcess.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Container/BvVector.h"
#include "BvCore/System/JobSystem/BvJob.h"


class BvJobCounter
{
public:
	friend class BvJobSystem;

	BvJobCounter()
		: m_Counter(kI32Max) {}
	~BvJobCounter() {}

	// All empty stubs, but they have to be here
	BvJobCounter(const BvJobCounter & rhs) : BvJobCounter() {}
	BvJobCounter & operator=(const BvJobCounter & rhs) { return *this; }
	BvJobCounter(BvJobCounter && rhs) noexcept : BvJobCounter() {}
	BvJobCounter & operator=(BvJobCounter && rhs) noexcept { return *this; }

	bool IsDone() { return m_Counter.load() == 0; }

private:
	void Set(const i32 count) { m_Counter.store(count); }
	void Decrement() { m_Counter--; }

private:
	std::atomic<i32> m_Counter;
};


class BvJobList;


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

class BvJobSystemWorker;


class BvJobSystem
{
	BV_NOCOPYMOVE(BvJobSystem);

public:
	BvJobSystem();
	~BvJobSystem();

	void Initialize(const JobSystemDesc desc = JobSystemDesc());
	void Shutdown();

	BvJobList* AllocJobList(const u32 jobListSize, JobListPriority priority = JobListPriority::kNormal);
	void Submit(BvJobList* pJobList);

	BV_INLINE bool IsActive() const { return m_IsActive.load() == true; }

private:
	BvVector<BvJobList*> m_JobListPool;
	BvVector<BvJobSystemWorker *> m_Workers;
	std::atomic<u32> m_JobListCount;
	std::atomic<bool> m_IsActive;
};


class BvJobList
{
	BV_NOCOPYMOVE(BvJobList);

	static constexpr BvJobList* kSyncPoint = (BvJobList*)1;
	static constexpr u32 kMaxJobs = 32;

	friend class BvJobSystem;
	friend class BvJobSystemWorker;

	using JobDataType = std::pair<BvJobT<56>, BvJobList*>;

public:
	template<typename Fn, typename... Args>
	void AddJob(Fn&& fn, Args &&... args)
	{
		BvAssertMsg(m_Jobs.Size() < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		m_Jobs.EmplaceBack(JobDataType());
		m_Jobs[m_Jobs.Size() - 1].first.Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
		m_Jobs[m_Jobs.Size() - 1].second = nullptr;
	}

	void AddSyncPoint()
	{
		BvAssertMsg(m_Jobs.Size() < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
		m_Jobs.EmplaceBack(JobDataType());
		m_Jobs[m_Jobs.Size() - 1].second = kSyncPoint;
	}

	void AddDependency(BvJobList * pDependency)
	{
		BvAssertMsg(m_Jobs.Size() < kMaxJobs, "A BvJobList can't have more than kMaxJobs - increase its value to add more");
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
	BvJobList(BvJobSystem* pJobSystem, const JobListPriority priority)
		: m_pJobSystem(pJobSystem), m_Priority(priority)
	{
		m_Jobs.Reserve(kMaxJobs);
	}
	~BvJobList() {}

private:
	BvJobSystem* m_pJobSystem;
	BvVector<JobDataType> m_Jobs;
	std::atomic<u32> m_CurrJob;
	std::atomic<u32> m_JobsDone;
	BvSpinlock m_Lock;
	JobListPriority m_Priority;
};