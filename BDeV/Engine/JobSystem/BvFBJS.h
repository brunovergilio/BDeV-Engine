#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvDelegate.h"
#include <atomic>


struct JobDecl
{
	BvTaskT<32> m_Job;
	class BvJobCounter* m_pCounter;
	std::atomic<u32> m_FetchLock;
};


class BvFBJS
{
public:
	friend class BvFBJSInstance;

	void Initialize();
	void Shutdown();

	bool IsActive() const;

private:
	JobDecl* AcquireJobs(u32 jobCount);
	void Submit(JobDecl* pJobs, u32 jobCount);

	BvFBJS();
	~BvFBJS();

private:
	JobDecl* m_pJobPool = nullptr;
	struct JobList* m_pJobListPool = nullptr;
	struct FiberData* m_pFiberPool = nullptr;
	class BvFBJSWorker* m_pWorkers = nullptr;
	u32 m_JobPoolSize = 0;
	u32 m_WorkerCount = 0;
	u32 m_FiberPoolSize = 0;
	std::atomic<bool> m_Active;
};

extern BvFBJS* g_pJobSys;