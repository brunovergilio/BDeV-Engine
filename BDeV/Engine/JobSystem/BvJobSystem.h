#pragma once


#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Utils/BvTask.h"


namespace JobSystem
{
	struct BvJob
	{
		BvTask<16> m_Job;
	};

	class alignas(kCacheLineSize) BvJobList
	{
	public:
		enum class Priority : u8
		{
			kNormal,
			kMedium,
			kHigh
		};

		struct Result
		{
			u32 m_NumJobsExecuted; // How many jobs were executed in this batch
			u32 m_NumChildren; // Number of child joblists ready to be scheduled
			BvJobList** m_ppChildren; // Pointer to the child joblists
			u64 m_BatchCycles; // Time this batch took, in cycles
			bool m_ListExhausted; // Tells whether or not we still have work to do on this list (but not if all jobs are finished)
			bool m_ListDone; // True only when all jobs have finished
		};

		BvJobList() = default;
		~BvJobList() = default;

		// Sets the name for debugging purposes - [CALL BEFORE SCHEDULING]
		void SetName(const char* pName);

		// Sets the underlying storages with uninitialized data - [CALL BEFORE SCHEDULING]
		void SetData(u32 maxJobs, BvJob* pJobs, u32 maxChildren = 0, BvJobList** ppChildren = nullptr);

		// Resets jobs / children / parents, but doesn't destroy the underlying storage - [CALL BEFORE SCHEDULING]
		void ClearJobsAndDependencies();

		// Adds a job to the list - [CALL BEFORE SCHEDULING]
		template<typename Fn>
		void AddJob(Fn&& fn)
		{
			BV_ASSERT(m_NumJobs < m_MaxJobs, "JobList is full");
			auto& job = m_pJobs[m_NumJobs++];
			job.m_Job.Set(std::forward<Fn>(fn));
		}

		// Adds a child joblist - [CALL BEFORE SCHEDULING]
		void AddChild(BvJobList* pJobList);

		// Determines how many jobs to execute in one fetch - [CALL BEFORE SCHEDULING]
		void SetBatchSize(u32 batchSize);

		// Sets the joblist priority - [CALL BEFORE SCHEDULING]
		void SetPriority(BvJobList::Priority priority);

		// Resets the joblist state
		void Reset();

		// Tries to run a batch of jobs - returns a 'Result' object with information about the execution
		Result Execute();

		// Waits until the joblist finishes executing - avoid calling this from a worker thread
		void Wait();

		// Returns the current number of valid jobs
		u32 GetNumJobs() const { return m_NumJobs; }

		// Returns the maximum number of job slots available
		u32 GetMaxJobs() const { return m_MaxJobs; }

		// Returns the max number of batches this joblist can be split into
		u32 GetMaxConcurrency() const { return m_NumJobs / m_BatchSize + u32(m_NumJobs % m_BatchSize != 0); }

		// Returns the number of parent joblists
		u32 NumParentJobLists() const { return m_NumParentJobLists; }

		// Returns the joblist priority
		BvJobList::Priority GetPriority() const { return m_Priority; }

		// Checks if a list has finished running
		bool IsDone() const { return m_JobsDone.load(std::memory_order_acquire) == m_NumJobs; }

		// Returns the name - for debugging purposes
		const char* GetName() const { return m_pName; }

	private:
		BvJob* m_pJobs = nullptr; // Jobs
		u32 m_NumJobs = 0;  // How many jobs we currently have
		u32 m_MaxJobs = 0; // Maximum number of jobs we can have

		BvJobList** m_ppChildren = nullptr; // Child joblists
		u32 m_NumChildren = 0; // How many children (dependent) joblists we currently have
		u32 m_MaxChildren = 0; // Maximum number of children (dependent) joblists we can have

		u32 m_NumParentJobLists = 0; // Number of parent joblists we have
		u32 m_BatchSize = 1; // How many jobs a worker acquire in one attempt

		Priority m_Priority = Priority::kNormal; // Priority

		const char* m_pName = nullptr; // For debugging

		alignas(kCacheLineSize) std::atomic<u32> m_NextJobIndex; // Next job index (may overshoot, expected)
		std::atomic<i64> m_TotalCycles; // Total execution time, in cycles
		std::atomic<u32> m_JobsDone; // Number of jobs done so far
		std::atomic<u32> m_NumExecutedParents; // Current number of parent joblists that have finished
	};


	struct BvJobSystemDesc
	{
		struct WorkerThreadDesc
		{
			const char* m_pName;
			u32 m_LogicalProcessorIndex;
		};

		struct PoolSizes
		{
			u32 m_JobListPoolSize;
			u32 m_JobPoolSize;
			u32 m_ChildJobListPoolSize;
		};

		IBvMemoryArena* m_pMemoryArena = BV_DEFAULT_MEMORY_ARENA;
		WorkerThreadDesc* m_pWorkerThreadDescs = nullptr;
		u32 m_NumWorkerThreads = 0;
		PoolSizes m_PersistentPoolSizes{};
		PoolSizes m_DynamicPoolSizes{};
		u32 m_WorkerQueueSize = 0;
	};


	// Forward declaration
	class BvJobSystemWorker;


	struct BvWorkerStats
	{
		u64 m_NumSchedulerCycles;
		u64 m_NumJobExecutionCycles;
		u64 m_NumJobsExecuted;
		u64 m_NumPausedCycles;
		u64 m_NumPauses;
		u64 m_NumSearchCycles;
		u64 m_NumWorkSearchFailures;
		u64 m_NumStealAttempts;
		u64 m_NumStealQueueProbes;
		u64 m_NumJobListsStolenByWorker;
		u64 m_NumJobListsStolenFromWorker;
		u64 m_NumJobListsSubmitted;
	};


	class BvJobSystem
	{
	public:
		friend class BvJobSystemController;

		BvJobSystem();
		~BvJobSystem();

		void Initialize(const BvJobSystemDesc& jsd = BvJobSystemDesc());
		void Shutdown();

		BvJobList* AllocJobList(u32 maxJobs = 0, u32 maxChildren = 0);
		BvJobList* AcquireJobList(u32 maxJobs, u32 maxChildren = 0);
		//BvJobList* RegisterJobList(u32 maxJobs = 0, u32 maxChildren = 0);

		void ResetAllJoblists();
		void ResetStats();
		void Submit(BvJobList* pJobList) { Submit(1, &pJobList, false); }
		void Submit(u32 numJobLists, BvJobList** ppJobLists) { Submit(numJobLists, ppJobLists, false); }
		template<typename Fn>
		BvJobList* Submit(Fn&& fn)
			requires (!std::is_same_v<Fn, BvJobList*>) && std::is_invocable_v<Fn>
		{
			auto pJobList = AcquireJobList(1);
			pJobList->AddJob(std::forward<Fn>(fn));
			Submit(1, &pJobList, false);
			
			return pJobList;
		}
		template<typename Fn>
		BvJobList* ParallelFor(Fn&& fn, u32 count, u32 batchSize = 1)
			requires std::is_invocable_v<Fn, u32>
		{
			auto pJobList = AcquireJobList(count / batchSize + count % batchSize);
			for (auto i = 0; i < count; i += batchSize)
			{
				u32 numJobs = i + batchSize > count ? count - i : batchSize;
				pJobList->AddJob([fn, i, numJobs]()
					{
						for (auto job = 0; job < numJobs; job++)
						{
							fn(job + i);
						}
					});
			}
			Submit(1, &pJobList, false);

			return pJobList;
		}

		void Wait(bool active = false);

		BV_INLINE u32 GetNumWorkers() const { return m_NumWorkers; }
		BvWorkerStats GetWorkerStats(u32 workerIndex);

	private:
		void OnJobListCompleted(BvJobList::Result& result);
		void Submit(u32 numJobLists, BvJobList** ppJobLists, bool includeChildren);

	private:
		IBvMemoryArena* m_pArena = BV_DEFAULT_MEMORY_ARENA; // Memory provider
		BvJobList* m_pJobListPool = nullptr;
		BvJob* m_pJobPool = nullptr;
		BvJobList** m_ppChildJobListPool = nullptr;
		BvJobSystemWorker* m_pWorkers = nullptr;
		BvJobSystemDesc::PoolSizes m_PersistentPoolSizes{};
		BvJobSystemDesc::PoolSizes m_DynamicPoolSizes{};
		u32 m_NumWorkers = 0;
		u32 m_NumFrames = 0;
		std::atomic<u32> m_JobListPoolIndex;
		std::atomic<u32> m_DynamicJobListPoolIndex;
		std::atomic<u32> m_JobPoolIndex;
		std::atomic<u32> m_DynamicJobPoolIndex;
		std::atomic<u32> m_ChildJobListPoolIndex;
		std::atomic<u32> m_DynamicChildJobListPoolIndex;
		std::atomic<u32> m_NextWorkerToUse;
		std::atomic<u32> m_NextWorkerToStealFrom;
		alignas(kCacheLineSize) std::atomic<u32> m_ActiveJobLists;
	};

#if defined(BV_ENABLE_JOBSYSTEM_TESTS)
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/System/Threading/BvThread.h"
#include "BDeV/Core/Utils/BvTime.h"

	// ============================================================
	// JOB SYSTEM STRESS TESTS
	// ============================================================
	//
	// These tests are deliberately synthetic. They are not intended
	// to represent a particular real workload; instead they exercise
	// different characteristics of the scheduler:
	//
	//   1. Many independent joblists
	//   2. Small jobs / high scheduler overhead
	//   3. Large jobs / low scheduler overhead
	//   4. Uneven workloads / stealing
	//   5. DAG workloads / dependency propagation
	//   6. Mixed priorities
	//
	// Adjust the constants at the bottom to scale the test.
	// ============================================================

	namespace Tests
	{
		// --------------------------------------------------------
		// A small piece of deterministic CPU work.
		//
		// IMPORTANT:
		// Don't let the compiler optimize the whole thing away.
		// --------------------------------------------------------

		static std::atomic<u64> g_StressSink{ 0 };

		BV_INLINE void DoCpuWork(u32 iterations)
		{
			u64 value = 0x123456789abcdef0ull;

			for (u32 i = 0; i < iterations; i++)
			{
				value ^= value >> 7;
				value *= 0x9e3779b97f4a7c15ull;
				value ^= value << 9;
				value += i;
			}

			g_StressSink.fetch_xor(value, std::memory_order_relaxed);
		}


		// --------------------------------------------------------
		// Workload configuration
		// --------------------------------------------------------

		struct WorkloadDesc
		{
			u32 m_NumJobLists = 1;
			u32 m_JobsPerList = 256;

			// Number of CPU iterations performed by each job.
			u32 m_WorkPerJob = 1000;

			// How many jobs each worker claims at once.
			u32 m_BatchSize = 8;

			BvJobList::Priority m_Priority =
				BvJobList::Priority::kNormal;
		};


		// --------------------------------------------------------
		// Create a set of independent joblists.
		//
		// These are useful for testing pure scheduler throughput,
		// queue distribution and stealing.
		// --------------------------------------------------------

		inline void CreateIndependentWorkload(
			BvJobSystem& jobSystem,
			const WorkloadDesc& desc)
		{
			for (u32 listIndex = 0;
				listIndex < desc.m_NumJobLists;
				listIndex++)
			{
				auto* pList =
					jobSystem.AllocJobList(desc.m_JobsPerList);

				pList->SetName("Stress - Independent");
				pList->SetBatchSize(desc.m_BatchSize);
				pList->SetPriority(desc.m_Priority);

				for (u32 jobIndex = 0;
					jobIndex < desc.m_JobsPerList;
					jobIndex++)
				{
					// Adapt this line to however BvJob exposes its
					// callable in your header.
					pList->AddJob(
						[work = desc.m_WorkPerJob]()
						{
							DoCpuWork(work);
						});
				}

				jobSystem.Submit(pList);
			}
		}


		// --------------------------------------------------------
		// One very large joblist.
		//
		// This should produce relatively little stealing and a lot
		// of parallel execution of a single list.
		// --------------------------------------------------------

		inline void TestLargeJobList(BvJobSystem& jobSystem)
		{
			WorkloadDesc desc;
			desc.m_NumJobLists = 1;
			desc.m_JobsPerList = 100000;
			desc.m_WorkPerJob = 1000;
			desc.m_BatchSize = 32;

			CreateIndependentWorkload(jobSystem, desc);
			jobSystem.Wait();
		}


		// --------------------------------------------------------
		// Lots of tiny joblists.
		//
		// This intentionally increases scheduler overhead.
		// It should exercise:
		//
		//   queue operations
		//   semaphore signaling
		//   joblist acquisition
		//   search failures
		//   worker distribution
		//
		// This is probably the most useful "scheduler overhead"
		// benchmark.
		// --------------------------------------------------------

		inline void TestManyTinyJobLists(BvJobSystem& jobSystem)
		{
			WorkloadDesc desc;
			desc.m_NumJobLists = 4096;
			desc.m_JobsPerList = 8;
			desc.m_WorkPerJob = 1000;
			desc.m_BatchSize = 1;

			CreateIndependentWorkload(jobSystem, desc);
			jobSystem.Wait();
		}


		// --------------------------------------------------------
		// Many medium-sized lists.
		//
		// This is closer to the sort of workload I'd expect from a
		// real engine: many independent pieces of work, but enough
		// work per list to amortize scheduler overhead.
		// --------------------------------------------------------

		inline void TestMediumWorkload(BvJobSystem& jobSystem)
		{
			WorkloadDesc desc;
			desc.m_NumJobLists = 256;
			desc.m_JobsPerList = 256;
			desc.m_WorkPerJob = 5000;
			desc.m_BatchSize = 8;

			CreateIndependentWorkload(jobSystem, desc);
			jobSystem.Wait();
		}


		// --------------------------------------------------------
		// Uneven workload.
		//
		// Some lists are deliberately much more expensive than
		// others. This is where stealing becomes interesting.
		// --------------------------------------------------------

		inline void TestUnevenWorkload(BvJobSystem& jobSystem)
		{
			constexpr u32 numLists = 128;
			constexpr u32 jobsPerList = 128;

			for (u32 listIndex = 0;
				listIndex < numLists;
				listIndex++)
			{
				auto* pList =
					jobSystem.AllocJobList(jobsPerList);

				pList->SetName("Stress - Uneven");
				pList->SetBatchSize(4);
				pList->SetPriority(
					BvJobList::Priority::kNormal);

				// Every few lists are deliberately expensive.
				u32 workPerJob =
					(listIndex % 8 == 0)
					? 50000
					: 1000;

				for (u32 jobIndex = 0;
					jobIndex < jobsPerList;
					jobIndex++)
				{
					pList->AddJob(
						[workPerJob]()
						{
							DoCpuWork(workPerJob);
						});
				}

				jobSystem.Submit(pList);
			}

			jobSystem.Wait();
		}


		// --------------------------------------------------------
		// DAG workload
		//
		//      A   B   C   D
		//       \ / \ / \ /
		//        E   F   G
		//         \  |  /
		//            H
		//
		// This stresses:
		//
		//   parent completion
		//   dependency counters
		//   child submission
		//   dynamically appearing work
		//   stealing after dependencies become runnable
		// --------------------------------------------------------

		inline void TestDependencyGraph(BvJobSystem& jobSystem)
		{
			constexpr u32 leafJobs = 128;
			constexpr u32 middleJobs = 128;
			constexpr u32 finalJobs = 256;

			auto* A = jobSystem.AllocJobList(leafJobs, 1);
			auto* B = jobSystem.AllocJobList(leafJobs, 2);
			auto* C = jobSystem.AllocJobList(leafJobs, 2);
			auto* D = jobSystem.AllocJobList(leafJobs, 1);

			auto* E = jobSystem.AllocJobList(middleJobs, 1);
			auto* F = jobSystem.AllocJobList(middleJobs, 1);
			auto* G = jobSystem.AllocJobList(middleJobs, 1);

			auto* H = jobSystem.AllocJobList(finalJobs);

			auto configure =
				[](BvJobList* pList,
					const char* pName,
					u32 batchSize,
					u32 work)
				{
					pList->SetName(pName);
					pList->SetBatchSize(batchSize);
					pList->SetPriority(
						BvJobList::Priority::kNormal);

					// Again, adapt GetJob() to your actual BvJob API.
					// The important part is the workload shape.
					//
					// This assumes the jobs have already been assigned
					// through the same mechanism used above.
					for (u32 i = 0;
						i < pList->GetMaxJobs();
						i++)
					{
						pList->AddJob(
							[work]()
							{
								DoCpuWork(work);
							});
					}
				};

			configure(A, "Stress - DAG A", 8, 5000);
			configure(B, "Stress - DAG B", 8, 5000);
			configure(C, "Stress - DAG C", 8, 5000);
			configure(D, "Stress - DAG D", 8, 5000);

			configure(E, "Stress - DAG E", 8, 5000);
			configure(F, "Stress - DAG F", 8, 5000);
			configure(G, "Stress - DAG G", 8, 5000);

			configure(H, "Stress - DAG H", 8, 10000);

			A->AddChild(E);
			B->AddChild(E);

			B->AddChild(F);
			C->AddChild(F);

			C->AddChild(G);
			D->AddChild(G);

			E->AddChild(H);
			F->AddChild(H);
			G->AddChild(H);

			jobSystem.Submit(A);
			jobSystem.Submit(B);
			jobSystem.Submit(C);
			jobSystem.Submit(D);

			jobSystem.Wait();
		}


		// --------------------------------------------------------
		// Priority workload.
		//
		// Flood the system with low-priority work while periodically
		// inserting high-priority work.
		//
		// This is useful for checking whether your priority queues
		// actually behave as intended under pressure.
		// --------------------------------------------------------

		inline void TestPriorities(BvJobSystem& jobSystem)
		{
			constexpr u32 lowPriorityLists = 256;
			constexpr u32 highPriorityLists = 32;

			for (u32 i = 0; i < lowPriorityLists; i++)
			{
				WorkloadDesc desc;
				desc.m_NumJobLists = 1;
				desc.m_JobsPerList = 256;
				desc.m_WorkPerJob = 5000;
				desc.m_BatchSize = 8;
				desc.m_Priority =
					BvJobList::Priority::kMedium;

				CreateIndependentWorkload(jobSystem, desc);

				// Periodically inject high-priority work.
				if ((i % 8) == 7)
				{
					WorkloadDesc high;
					high.m_NumJobLists = highPriorityLists / 8;
					high.m_JobsPerList = 64;
					high.m_WorkPerJob = 10000;
					high.m_BatchSize = 4;
					high.m_Priority =
						BvJobList::Priority::kHigh;

					CreateIndependentWorkload(jobSystem, high);
				}
			}

			jobSystem.Wait();
		}


		// --------------------------------------------------------
		// Concurrent submission stress test
		//
		// THIS ONE IS IMPORTANT FOR YOUR m_NextWorkerToUse CHANGE.
		//
		// Several producer threads submit independent joblists at
		// the same time.
		//
		// This should be run after changing Submit() to reserve
		// worker slots using fetch_add().
		// --------------------------------------------------------

		inline void TestConcurrentSubmission(BvJobSystem& jobSystem)
		{
			constexpr u32 producerCount = 8;
			constexpr u32 listsPerProducer = 256;
			constexpr u32 jobsPerList = 64;

			BvVector<BvThread> producers;
			producers.reserve(producerCount);

			for (u32 producerIndex = 0;
				producerIndex < producerCount;
				producerIndex++)
			{
				producers.emplace_back(
					[&jobSystem]()
					{
						for (u32 listIndex = 0;
							listIndex < listsPerProducer;
							listIndex++)
						{
							auto* pList =
								jobSystem.AllocJobList(jobsPerList);

							pList->SetName(
								"Stress - Concurrent Submit");

							pList->SetBatchSize(4);
							pList->SetPriority(
								BvJobList::Priority::kNormal);

							for (u32 jobIndex = 0;
								jobIndex < jobsPerList;
								jobIndex++)
							{
								pList->AddJob(
									[]()
									{
										DoCpuWork(5000);
									});
							}

							jobSystem.Submit(pList);
						}
					});
			}

			for (auto& producer : producers)
			{
				producer.Join();
			}

			jobSystem.Wait();
		}


		// --------------------------------------------------------
		// Run the complete benchmark suite.
		//
		// IMPORTANT:
		// ResetStats() should happen between tests, and ideally you
		// should print the statistics immediately after Wait().
		// --------------------------------------------------------

		inline void RunAll(BvJobSystem& jobSystem)
		{
			auto RunTest =
				[&jobSystem](const char* pName,
					auto&& test)
				{
					printf("\n");
					printf("==================================================\n");
					printf("%s\n", pName);
					printf("==================================================\n");

					jobSystem.ResetStats();
					jobSystem.ResetAllJoblists();

					auto start = BvTime::GetClockCyclesSerial();

					test();

					auto elapsed = BvTime::GetClockCyclesSerial() - start;

					printf("Total cycles: %llu\n", elapsed);

					for (auto i = 0; i < jobSystem.GetNumWorkers(); i++)
					{
						JobSystem::BvWorkerStats stats = jobSystem.GetWorkerStats(i);
						BvConsole::Println("Worker {} Stats:", i);
						BvConsole::Println("\tNum Scheduler Cycles: {}", stats.m_NumSchedulerCycles);
						BvConsole::Println("\tNum Paused Cycles: {}", stats.m_NumPausedCycles);
						BvConsole::Println("\tNum Pauses: {}", stats.m_NumPauses);
						BvConsole::Println("\tNum Search Cycles: {}", stats.m_NumSearchCycles);
						BvConsole::Println("\tNum Work Search Failures: {}", stats.m_NumWorkSearchFailures);
						BvConsole::Println("\tNum Jobs Executed: {}", stats.m_NumJobsExecuted);
						BvConsole::Println("\tNum Job Execution Cycles: {}", stats.m_NumJobExecutionCycles);
						BvConsole::Println("\tNum Steal Attempts: {}", stats.m_NumStealAttempts);
						BvConsole::Println("\tNum Steal Queue Probes: {}", stats.m_NumStealQueueProbes);
						BvConsole::Println("\tNum JobLists Stolen By Worker: {}", stats.m_NumJobListsStolenByWorker);
						BvConsole::Println("\tNum JobLists Stolen From Worker: {}", stats.m_NumJobListsStolenFromWorker);
						BvConsole::Println("\tNum JobLists Submitted: {}", stats.m_NumJobListsSubmitted);
						BvConsole::Println("\tOther ratios:");
						if (stats.m_NumJobsExecuted)
						{
							BvConsole::Println("\tAverage cycles per job: {:.2f}", double(stats.m_NumJobExecutionCycles) / double(stats.m_NumJobsExecuted));
						}
						if (stats.m_NumStealQueueProbes)
						{
							BvConsole::Println("\tSteal success rate: {:.2f}%", 100.0 * double(stats.m_NumJobListsStolenByWorker) / double(stats.m_NumStealQueueProbes));
						}
						auto totalTime = double(stats.m_NumSchedulerCycles);
						BvConsole::Println("\tSearch time rate: {:.2f}%", 100.0 * double(stats.m_NumSearchCycles) / totalTime);
						BvConsole::Println("\tJob execution time rate: {:.2f}%", 100.0 * double(stats.m_NumJobExecutionCycles) / totalTime);
						BvConsole::Println("\tPaused time rate: {:.2f}%", 100.0 * double(stats.m_NumPausedCycles) / totalTime);
						BvConsole::Println("");
					}
				};

			RunTest(
				"Large Job List",
				[&]()
				{
					TestLargeJobList(jobSystem);
				});

			RunTest(
				"Many Tiny Job Lists",
				[&]()
				{
					TestManyTinyJobLists(jobSystem);
				});

			RunTest(
				"Medium Workload",
				[&]()
				{
					TestMediumWorkload(jobSystem);
				});

			RunTest(
				"Uneven Workload",
				[&]()
				{
					TestUnevenWorkload(jobSystem);
				});

			RunTest(
				"Dependency Graph",
				[&]()
				{
					TestDependencyGraph(jobSystem);
				});

			RunTest(
				"Priority Workload",
				[&]()
				{
					TestPriorities(jobSystem);
				});

			RunTest(
				"Concurrent Submission",
				[&]()
				{
					TestConcurrentSubmission(jobSystem);
				});
		}
	}
#endif
}