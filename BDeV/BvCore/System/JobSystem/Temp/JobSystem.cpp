#include "JobSystem.h"


//JobSystem * JobSystem::s_pJobSystem = nullptr;
//
//
//constexpr u32 g_ThreadCount = 4;
//
//
//u32 JobQueue::s_Checkpoint = 0;
//u32 JobQueue::s_Barrier = 1;
//u32 JobQueue::s_QueueFinished = 2;
//
//
//void JobQueue::Add(const Job & job)
//{
//	m_Jobs[m_JobCount++] = job;
//}
//
//
//void JobQueue::Add(pFnJob pFunction, void * pData)
//{
//	m_Jobs[m_JobCount++] = Job(pFunction, pData);
//}
//
//
//void JobQueue::AddCheckpoint()
//{
//	BvAssert(m_JobCount > 0);
//	BvAssert(!m_HasCheckpoint);
//	m_Checkpoints[m_CheckPointCount++] = m_JobCount - m_LastSignal;
//	m_Jobs[m_JobCount] = Job(Nop, &s_Checkpoint);
//	m_LastSignal = m_JobCount;
//	m_JobCount++;
//	m_HasCheckpoint = true;
//}
//
//
//void JobQueue::AddBarrier()
//{
//	if (m_HasCheckpoint)
//	{
//		m_Jobs[m_JobCount++] = Job(Nop, &s_Barrier);
//		m_HasCheckpoint = false;
//	}
//}
//
//
//void JobQueue::Submit(JobQueue * const pQueueToWaitFor)
//{
//	if (m_JobCount == 0)
//	{
//		return;
//	}
//
//	if (pQueueToWaitFor)
//	{
//		m_pJobQueueGuardToWait = &pQueueToWaitFor->m_waitGuards[pQueueToWaitFor->m_CurrWaitGuard];
//	}
//	else
//	{
//		m_pJobQueueGuardToWait = nullptr;
//	}
//
//	m_waitGuards[m_CurrWaitGuard].SetValue(1);
//	m_CurrWaitGuard = (m_CurrWaitGuard + 1) & (kMaxWaitGuards - 1);
//
//	m_Checkpoints[m_CheckPointCount++] = m_JobCount - m_LastSignal;
//	m_Jobs[m_JobCount++] = Job(Nop, &s_QueueFinished);
//
//	JobSystem::GetInstance()->Submit(this);
//}
//
//
//void JobQueue::WaitAndReset()
//{
//	if (m_JobCount > 0 && m_CheckPointCount > 0)
//	{
//		while (m_Checkpoints[m_CheckPointCount - 1].GetValue() > 0)
//		{
//			BvPause();
//		}
//
//		m_Version.Increment();
//
//		while (m_NumThreadsRunning.GetValue() > 0)
//		{
//			BvPause();
//		}
//
//		m_JobCount = 0;
//		m_CheckPointCount = 0;
//		m_LastSignal = 0;
//		m_HasCheckpoint = false;
//	}
//
//	m_Done = true;
//}
//
//
//i32 JobQueue::Run(const u32 threadIndex, ThreadStateData & stateData, const bool runAll)
//{
//	m_NumThreadsRunning.Increment();
//
//	i32 result = RunInternal(threadIndex, stateData, runAll);
//
//	m_NumThreadsRunning.Decrement();
//
//	return result;
//}
//
//
//i32 JobQueue::RunInternal(const u32 threadIndex, ThreadStateData & stateData, const bool runAll)
//{
//	if (stateData.version != m_Version.GetValue())
//	{
//		return JobQueueResult::kDone;
//	}
//
//	threadIndex;
//	i32 result = JobQueueResult::kOk;
//
//	for (; stateData.lastJob < m_CurrJob.GetValue() && stateData.lastJob < m_JobCount; stateData.lastJob++)
//	{
//		if (m_Jobs[stateData.lastJob].pData == &s_Checkpoint)
//		{
//			stateData.lastSignal++;
//		}
//		else if (m_Jobs[stateData.lastJob].pData == &s_Barrier)
//		{
//			if (m_Checkpoints[stateData.lastSignal - 1].GetValue() > 0)
//			{
//				return (result | JobQueueResult::kStalling);
//			}
//		}
//		else if (m_Jobs[stateData.lastJob].pData == &s_QueueFinished)
//		{
//			if (m_Checkpoints[m_CheckPointCount - 1].GetValue() > 0)
//			{
//				return (result | JobQueueResult::kStalling);
//			}
//		}
//	}
//
//	do
//	{
//		if (m_FetchLock.Increment() == 1)
//		{
//			stateData.nextJob = m_CurrJob.Increment() - 1;
//
//			for (; stateData.lastJob <= stateData.nextJob && stateData.lastJob < m_JobCount; stateData.lastJob++)
//			{
//				if (m_Jobs[stateData.lastJob].pData == &s_Checkpoint)
//				{
//					stateData.lastSignal++;
//				}
//				else if (m_Jobs[stateData.lastJob].pData == &s_Barrier)
//				{
//					if (m_Checkpoints[stateData.lastSignal - 1].GetValue() > 0)
//					{
//						m_CurrJob.Decrement();
//
//						m_FetchLock.Decrement();
//
//						return (result | JobQueueResult::kStalling);
//					}
//				}
//				else if (m_Jobs[stateData.lastJob].pData == &s_QueueFinished)
//				{
//					if (m_Checkpoints[m_CheckPointCount - 1].GetValue() > 0)
//					{
//						m_CurrJob.Decrement();
//
//						m_FetchLock.Decrement();
//
//						m_waitGuards[m_CurrWaitGuard].Decrement();
//
//						return (result | JobQueueResult::kStalling);
//					}
//				}
//			}
//
//			m_FetchLock.Decrement();
//		}
//		else
//		{
//			m_FetchLock.Decrement();
//
//			return (result | JobQueueResult::kStalling);
//		}
//
//		if (stateData.nextJob >= m_JobCount)
//		{
//			return (result | JobQueueResult::kDone);
//		}
//
//		m_Jobs[stateData.nextJob].pFunction(m_Jobs[stateData.nextJob].pData);
//
//		result |= JobQueueResult::kInProgress;
//
//		if (m_Checkpoints[stateData.lastSignal].Decrement() == 0)
//		{
//			if (stateData.lastSignal == static_cast<i32>(m_CheckPointCount) - 1)
//			{
//				return (result | JobQueueResult::kDone);
//			}
//		}
//	} while (runAll);
//
//	return result;
//}
//
//
//JobThread::JobThread()
//{
//}
//
//
//JobThread::~JobThread()
//{
//}
//
//
//void JobThread::Start(const u32 index)
//{
//	m_Index = index;
//
//	m_Thread.Start(Thread, this);
//}
//
//
//void JobThread::Stop()
//{
//	{
//		BvScopedLock lock(m_WorkSignalLock);
//		m_WorkSignal.Set();
//	}
//	m_Thread.Wait();
//}
//
//
//void JobThread::AddJobQueue(JobQueue * const pJobQueue)
//{
//	{
//		BvScopedLock lock(m_JobListLock);
//		m_pJobQueues[m_LastJobList & (kMaxJobLists - 1)] = pJobQueue;
//		m_JobQueueVersions[m_LastJobList & (kMaxJobLists - 1)] = pJobQueue->m_Version.GetValue();
//		m_LastJobList++;
//	}
//
//	{
//		BvScopedLock lock(m_WorkSignalLock);
//		m_WorkSignal.Set();
//	}
//}
//
//
//void JobThread::Process()
//{
//	JobQueue::ThreadStateData stateData[kMaxJobLists];
//	i32 numJobLists = 0;
//	i32 lastStalledJobList = -1;
//
//	JobSystem * pSys = JobSystem::GetInstance();
//	while (pSys->IsActive())
//	{
//		m_WorkSignal.Wait();
//
//		while (pSys->IsActive())
//		{
//			if (m_FirstJobList < m_LastJobList && numJobLists < kMaxJobLists)
//			{
//				stateData[numJobLists].pJobQueue = m_pJobQueues[m_FirstJobList & (kMaxJobLists - 1)];
//				stateData[numJobLists].version = m_JobQueueVersions[m_FirstJobList & (kMaxJobLists - 1)];
//
//				numJobLists++;
//				m_FirstJobList++;
//			}
//
//			if (numJobLists == 0)
//			{
//				break;
//			}
//
//			u32 currJobList = 0;
//			JobPriority priority = JobPriority::kLow;
//			if (lastStalledJobList < 0)
//			{
//				for (u32 i = 0; i < numJobLists; i++)
//				{
//					if (stateData[i].pJobQueue->GetPriority() > priority)
//					{
//						currJobList = i;
//						priority = stateData[i].pJobQueue->GetPriority();
//						if (priority == JobPriority::kHigh)
//						{
//							break;
//						}
//					}
//				}
//			}
//			else
//			{
//				currJobList = lastStalledJobList;
//				for (u32 i = 0; i < numJobLists; i++)
//				{
//					if (i != currJobList && stateData[i].pJobQueue->GetPriority() > priority)
//					{
//						currJobList = i;
//						priority = stateData[i].pJobQueue->GetPriority();
//					}
//				}
//			}
//
//			i32 result = stateData[currJobList].pJobQueue->Run(m_Index, stateData[currJobList], priority == JobPriority::kHigh);
//			if (result & JobQueue::JobQueueResult::kDone)
//			{
//				for (u32 i = currJobList; i < numJobLists - 1; i++)
//				{
//					stateData[i] = stateData[i + 1];
//				}
//				numJobLists--;
//				lastStalledJobList = -1;
//			}
//			else if (result & JobQueue::JobQueueResult::kStalling)
//			{
//				if (currJobList == static_cast<u32>(lastStalledJobList) && (result & JobQueue::JobQueueResult::kInProgress) == 0)
//				{
//					BvPause();
//				}
//			}
//			else // if (result & JobQueue::JobQueueResult::kInProgress)
//			{
//				lastStalledJobList = -1;
//			}
//		}
//	}
//}
//
//
//void JobThread::Thread(void * pData)
//{
//	JobThread * pThread = reinterpret_cast<JobThread *>(pData);
//	pThread->Process();
//}
//
//
//JobSystem::JobSystem()
//{
//	s_pJobSystem = this;
//}
//
//
//JobSystem::~JobSystem()
//{
//	s_pJobSystem = nullptr;
//}
//
//
//void JobSystem::Initialize()
//{
//	BvAssert(m_Active.GetValue() == 0);
//
//	m_Active.Store(1);
//
//	m_pJobThreads = new JobThread[g_ThreadCount];
//	for (auto i = 0; i < g_ThreadCount; i++)
//	{
//		m_pJobThreads[i].Start(i);
//	}
//}
//
//
//void JobSystem::Shutdown()
//{
//	BvAssert(m_Active.GetValue() == 1);
//
//	m_Active.Store(0);
//
//	for (auto i = 0; i < g_ThreadCount; i++)
//	{
//		m_pJobThreads[i].Stop();
//	}
//
//	BvDeleteArray(m_pJobThreads);
//}
//
//
//void JobSystem::Submit(JobQueue * const pJobQueue)
//{
//	for (auto i = 0; i < g_ThreadCount; i++)
//	{
//		m_pJobThreads[i].AddJobQueue(pJobQueue);
//	}
//}
//
//
//JobQueue * JobSystem::AllocQueue()
//{
//	return new JobQueue();
//}
//
//
//void JobSystem::FreeJobQueue(JobQueue * & pQueue)
//{
//	delete pQueue;
//}