#include "BDeV/BDeV.h"
#include "BDeV/Core/Utils/BvRTTI.h"
#include <BDeV/Engine/JobSystem/BvParallelJobSystem.h>
#include <span>
#include <source_location>
#include "BDeV/Core/Utils/BvUTF.h"
#include "BDeV/Core/Container/BvFlatMap.h"
#include "BDeV/Core/Container/BvFlatHashMap.h"

char stack[1024];
char stack2[1024];
char stack3[1024];
char stack4[100];


//BV_INIT_TEST_UNIT_SYSTEM();
//
//BV_TEST_UNIT(abc)
//{
//	BV_TEST(1 == 3);
//	BV_TEST(2 == 2);
//	BV_TEST(0.0f == 0.0f);
//	BV_TEST(true == 1);
//}
//
//
//BV_TEST_UNIT(def)
//{
//	BV_TEST(1 == 3);
//	BV_TEST(2 == 2);
//	BV_TEST(0.0f == 0.0f);
//	BV_TEST(true == 1);
//}


struct WE
{
	//BV_RTTI(WE);
	int f();
	int ff() {}
	int a = int(0);
	int (*A)();
};


BV_ROBJECT() struct TestStruct
{
	BV_RMETHOD() TestStruct() {}
	BV_RMETHOD() void afsgg(int sedf, char b = '\\');
	BV_RMETHOD() void abc(int sedf);
	BV_RMETHOD() virtual float ff() {}
	BV_RMETHOD() int cc()
	{
		return 0;
	}
	BV_RMETHOD() int (*pfn(int, int))() { return nullptr; }

	BV_RVAR() int a = 0;
	BV_RVAR() int *b = nullptr;
	BV_RVAR() int (*c);
	BV_RVAR() int (*d)(int);
	BV_RVAR() int* (*e)(int);
	BV_RVAR() int (f) = 0;
	BV_RVAR() int g[2];
	BV_RVAR() int (h[2]);
	BV_RVAR() int i[2][3];
};


BV_ROBJECT() class TestStruct2 : public TestStruct
{
	int d()
	{
		return 0;
	}
	int e;
	int f;
};

struct A
{
	A() {}
	A(const A& a) : b(a.b) {} /*
							  test comm */
	BvRobinMap<int, int> b;
};


struct alignas(64) MyStruct
{
	char c[64];
};

constexpr BvUUID fa1 = MakeUUID("7b134b6e-b092-465f-9c00-af77a562ba2b");
constexpr BvUUID fa2 = MakeUUID("7b134b6e-b092-465f-9c00-a127a562ba2b");
constexpr BvUUID fa3 = MakeUUID("7b134b6e-b092-465f-9c00-af77a5cdba2b");


#include <iostream>
#include <utility>



thread_local int a = 0;
BvFiber* pFb = nullptr;
BvFiber* pFb2 = nullptr;

BvAdaptiveMutex am;
BvMutex mm;





int main()
{
	BvFlatMap<u32, u32> fm;
	fm.Emplace(1, 2);
	fm.Emplace(3, 3);
	fm.Emplace(2, 4);
	fm.Insert(std::make_pair(2u, 3u));
	std::pair testu(4u, 3u);
	fm.Insert(testu);
	fm.Erase(1);
	auto res = fm.Emplace(1, 2);
	//printf("%u, %u", res.first->first, res.first->second);
	fm.InsertOrAssign(2, 24);
	fm.InsertOrAssign(testu.first, testu.second);

	testu.first = 10;
	auto res2 = fm.Find(2);
	auto res3 = fm.Find(testu.first);

	BvFlatHashMap<i32, i32> hm;
	hm.Emplace(1, 1);
	hm.Erase(1);
	hm.Emplace(1, 1);
	hm.Erase(1);
	hm.Emplace(1, 1);
	hm.Erase(1);
	hm.Emplace(2, 2);
	hm.Emplace(1, 1);
	hm.Erase(1);
	hm.Emplace(3, 3);
	hm.Emplace(4, 4);
	hm.Insert(BvFlatHashMap<i32, i32>::value_type(5, 5));
	const i32 ci = 6;
	BvFlatHashMap<i32, i32>::value_type ipair(ci, 6);
	hm.Insert(ipair);
	hm.Insert(std::make_pair(5u, 5u));
	hm.InsertOrAssign(5, 15);
	hm.InsertOrAssign(ci, 15);
	hm.InsertOrAssign(5u, 15);

	BvFlatHashMap<i32, i32> hm2(hm);

	for (const auto& it : hm)
	{
		printf("%d, %d\n", it.first, it.second);
	}

	for (auto it = hm2.cbegin(); it != hm2.cend(); ++it)
	{
		printf("%d, %d\n", it->first, it->second);
	}

	//hm.Resize(16);

	return 0;


	//auto p = BV_SALLOC(1_kb, 32);
	//BV_SFREE(p);
	//srand(time(nullptr));
	//int n = rand();
	//BvConsole::Print(BvColorI(0, 100, 182), BvColorI(0, 0, 0), "fasf {}\n", n);
	//BvConsole::Print(BvColorI(0, 255, 0), "fasf {}\n", 123);
	//BvConsole::Println("afasfsf {} {}", 123, 545);
	//std::print("\033[38;2;{};{};{}m", 0, 172, 0);
	//std::println("fabf");
	//std::print("\033[0m");
	//char p[32];
	//scanf("%s", p);
	//char buf[64];
	//auto f = std::format_to_n(buf, 63, "Trying out {1} times to {0}", n, p);
	//*f.out = 0;

	// Use the custom function
	//my_vformat_to(buffer.begin(), "Hello, {}! This is {}", 1, 2);
	//my_vformat_to(bb, "Hello, {}! This is {}", 1, 2);
	//srand(time(nullptr));
	//auto ii = rand();
	//BvDebug::Println("Hello, {}! This is {}", 2, ii);
	//ii = rand();
	//BvDebug::Print("Hello, {}! This is {}", 1, ii);
	//return 0;

	//JobSystemDesc jsDesc;
	//jsDesc.m_NumWorkerThreadDescs = 4;

	//BvParallelJobSystem js;
	//js.Initialize(jsDesc);
	//auto pJobList1 = js.AllocJobList(50, 2);
	//for (auto i = 0; i < 50; ++i)
	//{
	//	pJobList1->AddJob([i]()
	//		{
	//			BvConsole::Print(BvColorI::BrightYellow, "JobList #1's Job %d on processor %u\n", i, GetCurrentProcessorNumber());
	//			//BvRandom32 r;
	//			//BvThread::Sleep(r.Next(50, 100));
	//		});

	//	if (i == 5 || i == 40)
	//	{
	//		pJobList1->AddSyncPoint();
	//	}
	//}

	//auto pJobList2 = js.AllocJobList(20, 1);
	//for (auto i = 0; i < 20; ++i)
	//{
	//	pJobList2->AddJob([i]()
	//		{
	//			BvConsole::Print(BvColorI::BrightMagenta, "JobList #2's Job %d on processor %u\n", i, GetCurrentProcessorNumber());
	//			//BvRandom32 r;
	//			//BvThread::Sleep(r.Next(50, 100));
	//		});

	//	if (i == 10)
	//	{
	//		pJobList2->AddJobListDependency(pJobList1);
	//	}
	//}

	//pJobList1->Submit();
	//pJobList2->Submit();
	//js.Wait();

	//for (auto i = 0; i < jsDesc.m_NumWorkerThreadDescs; ++i)
	//{
	//	auto& stats = js.GetStats(i);
	//	BvConsole::Print("Worker %d -> ", i);
	//	//BvConsole::Print("Running Time: %llu:\n", stats.m_TotalRunningTimeUs);
	//	BvConsole::Print("Active Time [%llu ms] | ", stats.m_ActiveTimeUs);
	//	BvConsole::Print("Job Time [%llu ms, %.2f%%] | ", stats.m_TotalJobTimeUs, 100.0 * double(stats.m_TotalJobTimeUs) / double(stats.m_ActiveTimeUs));
	//	BvConsole::Print("Jobs Run: %u:\n", stats.m_JobsRun);
	//}
	//js.Shutdown();

	//return 0;


	//BvFiber mfb = BvFiber::CreateForThread();

	//BvFiber fb = BvFiber([&mfb]()
	//	{
	//		printf("%d\n", a);
	//		mfb.Yield();

	//		printf("%d\n", a);
	//		BvFiber::GetCurrent().Yield();
	//	});

	//fb.Resume();

	//BvThread t([&fb]()
	//	{
	//		BvFiber::CreateForThread();
	//		a = 1;

	//		fb.Resume();
	//		BvFiber::DestroyForThread();
	//	});
	//t.Wait();

	//fb = nullptr;
	//BvFiber::DestroyForThread();

	//return 0;
}