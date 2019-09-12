#pragma once


// Based on "Parallelizing the Naughty Dog Engine Using Fibers" by Christian Gyrling
// https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine


#include "SafeQueue.h"
#include "BvCore/BvDebug.h"
#include "BvCore/System/Threading/BvThread.h"
#include "BvCore/System/Threading/BvFiber.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/System/Threading/BvProcess.h"
#include "BvCore/Utils/BvUtils.h"
#include <utility>


class BvJobCounter
{
public:
	friend class BvJobSystem;
	friend struct BvJob;

	~BvJobCounter() {}

private:
	BvJobCounter() {}

	void Set(const i32 count) { m_Counter.Store(count); }
	void Decrement() { m_Counter.Decrement(); }
	bool IsDone() { return m_Counter.Load() == 0; }

private:
	BvAtomic m_Counter;
};


enum class BvJobPriority
{
	kLow,
	kMedium,
	kHigh,
};


typedef void(*JobFunction)(void*);


struct BvJob
{
	friend class BvJobSystem;

	BvJob() {}
	BvJob(const JobFunction pFunction, void * const pData = nullptr, BvJobPriority priority = BvJobPriority::kLow)
		: m_pFunction(pFunction), m_pData(pData), m_Priority(priority), m_pCounter(nullptr) {}

	JobFunction m_pFunction = nullptr;
	void * m_pData = nullptr;
	BvJobPriority m_Priority = BvJobPriority::kLow;
	BvJobCounter * m_pCounter = nullptr;

private:
	void Execute()
	{
		BvAssert(m_pFunction != nullptr);
		m_pFunction(m_pData);

		if (m_pCounter)
		{
			m_pCounter->Decrement();
		}
	}
};


class BvJobSystem
{
	BV_SINGLETON(BvJobSystem);
public:
	BvJobSystem();
	~BvJobSystem();

	void Initialize();
	void Shutdown();

	void AddJob(BvJob * const pJob, BvJobCounter * const pCounter = nullptr);
	void AddJobs(BvJob * const pJobs, const u32 count, BvJobCounter * const pCounter = nullptr);

	BvJobCounter * AllocCounter();
	void FreeCounter(BvJobCounter * & pCounter);
	void WaitForCounter(BvJobCounter * & pCounter);

private:
	void ProcessThread(const u32 index);
	void ProcessFiber();
	bool GetJob(BvJob * & pJob);
	u32 GetFreeFiber(const u32 currIndex);
	void ProcessWaitingJobs();

	static void ThreadFunc(void * pData);
	static void FiberFunc(void * pData);

private:
	struct ThreadLocalStorage
	{
		struct WaitFiberData
		{
			BvJobCounter * pCounter = nullptr;
			u32 waitFiberIndex = UINT32_MAX;
		};

		WaitFiberData waitJobs[4]{};
		BvJob * pCurrJob = nullptr;
		u64 threadId = 0;
		u32 currWaitCount = 0;
		u32 fiberIndex = UINT32_MAX;
	};

	BvFiber m_MainThreadFiber;
	BvThread * m_pThreads = nullptr;
	BvFiber * m_pThreadFibers = nullptr;
	BvFiber * m_pFibers = nullptr;
	BvAtomic * m_pFiberStates = nullptr;
	ThreadLocalStorage * m_pThreadData = nullptr;
	BvSafeQueue<BvJob *, BvSpinlock> * m_pJobQueues = nullptr;
	BvAtomic m_Active;
	u32 m_NumCores = 0;
};


extern BvJobSystem JobSys;