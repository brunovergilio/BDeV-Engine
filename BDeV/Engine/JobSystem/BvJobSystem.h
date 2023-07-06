#pragma once

#include "BDeV/Utils/BvUtils.h"


namespace JS
{
	using JobFunction = void(*)(void*);

	struct Job
	{
		enum class Priority : u8
		{
			kLow,
			kNormal,
			kHigh,
			kCritical
		};

		JobFunction m_pFunction = nullptr;
		void* m_pData = nullptr;
		Priority m_Priority = Priority::kNormal;
	};

	struct JobCounter
	{
		friend class BvJobSystem;

		JobCounter() = default;
		JobCounter(i32 index, i32 version);
		bool IsDone() const;

	private:
		i32 m_Index = -1;
		i32 m_Version = 0;
	};


#define BV_JOB_DECL(jobName) void jobName(void* pData)
#define BV_JOB_DECL_STATIC(jobName) static void jobName(void* pData) // For class methods
#define BV_JOB_FUNCTION(jobName) void jobName(void* pData)
#define BV_JOB_DATA(dataType) reinterpret_cast<dataType>(pData)

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

	BV_API JobCounter RunJob(const Job& job);
	BV_API JobCounter RunJobs(u32 count, const Job* const pJobs);
	BV_API void WaitForCounter(const JobCounter& jobCounter);
};