#pragma once

#include "BDeV/Utils/BvUtils.h"


namespace JS
{
	struct JobCounterHandle
	{
		friend class BvJobSystem;

		const bool IsDone() const;

	private:
		i32 m_CounterIndex = 0;
		i32 m_Version = 0;
	};


	struct Job
	{
		enum class Priority : u8
		{
			kNormal,
			kLow,
			kHigh,
			kCritical
		};

		void(*m_Function)(void*) = nullptr;
		void* m_pData = nullptr;
		Priority m_Priority = Priority::kNormal;
	};


#define JobFunction(jobName) void jobName(void* pData)
#define GetJobData(dataType) reinterpret_cast<dataType>(pData)


	struct JobSystemDesc
	{
		size_t m_FiberStackSize = 0;
		u32 m_CounterPoolCount = 0;
		u16 m_WorkerThreadCount = 0;
		u16 m_FiberPoolCount = 0;
	};

	void Initialize(const JobSystemDesc& jobSystemDesc = JobSystemDesc());
	void Shutdown();

	JobCounterHandle RunJob(const Job& job);
	JobCounterHandle RunJobs(u32 count, const Job* const pJobs);
	void WaitForCounter(const JobCounterHandle& jobCounter);
};