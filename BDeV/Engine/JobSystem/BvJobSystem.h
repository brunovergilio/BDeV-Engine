#pragma once

#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvDelegate.h"


namespace JS
{
	enum class JobPriority : u8
	{
		kLow,
		kNormal,
		kHigh,
	};

	constexpr auto kMaxJobFunctionSize = kCacheLineSize - (sizeof(const char*) * 3);
	using JobFunction = BvTaskT<kMaxJobFunctionSize>;
	//using JobFunction = void(*)(void*);

	struct Job
	{
		JobFunction m_Job;
		const char* m_pName = nullptr;
		JobPriority m_Priority = JobPriority::kNormal;
	};
	constexpr auto k = sizeof Job;

	class Counter;

#define BV_JOB_FUNCTION(jobName) void jobName(void* pData)
#define BV_JOB_DATA(dataType) static_cast<dataType*>(pData)

	struct JobSystemDesc
	{
		static constexpr size_t kDefaultFiberStackSize = 128 * 1024;
		static constexpr u32 kDefaultJobPoolSize = 250;
		static constexpr u32 kDefaultNumWorkerThreads = 2;
		static constexpr u32 kDefaultFiberPoolSize = 128;

		size_t m_FiberStackSize = kDefaultFiberStackSize;
		u32 m_JobPoolSize = kDefaultJobPoolSize;
		u32 m_NumWorkerThreads = kDefaultNumWorkerThreads;
		u32 m_FiberPoolSize = kDefaultFiberPoolSize;
	};

	BV_API void Initialize(const JobSystemDesc& jobSystemDesc = JobSystemDesc());
	BV_API void Shutdown();

	BV_API void RunJob(const Job& job, Counter*& pCounter);
	BV_API void RunJobs(u32 count, const Job* pJobs, Counter*& pCounter);
	BV_API void WaitForCounter(Counter* pCounter);
	BV_API void FreeCounter(Counter*& pCounter);
};