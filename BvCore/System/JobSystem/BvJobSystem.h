#pragma once


// Based on "Parallelizing the Naughty Dog Engine Using Fibers" by Christian Gyrling
// https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine


#include "BvCore/BvDebug.h"
#include "BvCore/System/Threading/BvThread.h"
#include "BvCore/System/Threading/BvFiber.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/System/Threading/BvProcess.h"
#include "BvCore/System/Threading/BvAtomic.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Container/BvVector.h"
#include <utility>


class BvJobCounter
{
public:
	friend class BvJobSystem;

	BvJobCounter()
		: m_Counter(INT32_MAX) {}
	~BvJobCounter() {}

	// All empty stubs, but they have to be here
	BvJobCounter(const BvJobCounter & rhs) : BvJobCounter() {}
	BvJobCounter & operator=(const BvJobCounter & rhs) { return *this; }
	BvJobCounter(BvJobCounter && rhs) : BvJobCounter() {}
	BvJobCounter & operator=(BvJobCounter && rhs) { return *this; }

	bool IsDone() { return m_Counter.Load() == 0; }

private:
	void Set(const i32 count) { m_Counter.Store(count); }
	void Decrement() { m_Counter--; }

private:
	BvAtomic<i32> m_Counter;
};


class BvJob
{
	static constexpr size_t s_JobSize = kCacheLineSize - sizeof(BvJobCounter *);

public:
	BvJob() {}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvJob>
		&& std::is_invocable_v<Fn, Args...>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	explicit BvJob(Fn && fn, Args &&... args)
	{
		BvCompilerAssert(sizeof(BvDelegate<Fn, Args...>) <= s_JobSize, "Job object is too big");
		Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	explicit BvJob(const BvJob & rhs)
	{
		memcpy(m_Data, rhs.m_Data, s_JobSize);
	}

	BvJob & operator=(const BvJob & rhs)
	{
		if (this != &rhs)
		{
			this->~BvJob();
			memcpy(m_Data, rhs.m_Data, s_JobSize);
		}

		return *this;
	}
	
	explicit BvJob(BvJob && rhs)
	{
		*this = std::move(rhs);
	}

	BvJob & operator=(BvJob && rhs)
	{
		if (this != &rhs)
		{
			this->~BvJob();
			memcpy(m_Data, rhs.m_Data, s_JobSize);
			rhs.~BvJob();
		}

		return *this;
	}

	~BvJob()
	{
		if (IsSet())
		{
			reinterpret_cast<BvDelegateBase * const>(m_Data)->~BvDelegateBase();
			Reset();
		}
	}

	void Run()
	{
		BvAssert(IsSet());
		reinterpret_cast<const BvDelegateBase * const>(m_Data)->Invoke();
	}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvJob>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	void Set(Fn && fn, Args &&... args)
	{
		BvCompilerAssert(sizeof(BvDelegate<Fn, Args...>) <= s_JobSize, "Job object is too big");
		new (m_Data) BvDelegate<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	bool IsSet() const
	{
		auto vp = *reinterpret_cast<const u64 * const>(m_Data);
		return vp != 0;
	}

	void Reset()
	{
		auto pVp = reinterpret_cast<u64 * const>(m_Data);
		pVp = 0;
	}

private:
	u8 m_Data[s_JobSize]{};
};


class BvJobSystem
{
	BV_NOCOPYMOVE(BvJobSystem);

public:
	BvJobSystem();
	~BvJobSystem();

	void Initialize();
	void Shutdown();

	void RunJobs(BvJob * const pJobs, const u32 jobCount, BvJobCounter * pCounter);
	void WaitForCounter(BvJobCounter * const pCounter);

private:
	// For TLS values - will reside in a separate cpp
	u32 GetThreadIndex() const;
	void SetThreadIndex(const u32 index);

private:
	struct alignas(kCacheLineSize) JobData
	{
		BvJob job;
		BvJobCounter * pCounter;
	};

	struct ThreadData
	{
		BvJobSystem * pJobSys;
		BvEvent workEvent;
		BvFiber threadFiber;
		u32 threadIndex;
		u32 fiberIndex;
	};

	struct FiberData
	{
		BvJobSystem * pJobSys;
		BvFiber fiber;
	};

	BvVector<JobData> m_Jobs;
	BvVector<ThreadData> m_Threads;
	BvVector<FiberData> m_Fibers;

	BvSpinlock<> m_QueuedJobIndex;
	BvAtomic<u32> m_CurrJob;
};