#include "BDeV/Container/BvFixedVector.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/Container/BvRobinMap.h"
#include "BDeV/System/File/BvAsyncFile.h"
#include "BDeV/System/File/BvFileSystem.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Engine/JobSystem/BvJobSystem.h"
#include "BDeV/Engine/TBJobSystem/BvTBJobSystem.h"
#include <functional>
#include "BDeV/System/Memory/BvMemory.h"
#include "BDeV/Utils/BvHash.h"
#include "BDeV/BvCore.h"
#include "BDeV/Reflection/BvObjectInfo.h"
#include "BDeV/System/File/BvPath.h"
#include "BDeV/System/Window/BvMonitor.h"
#include "BDeV/Reflection/BvRTTI.h"
#include "BDeV/System/Debug/BvDebug.h"
#include "BDeV/Utils/BvTestUnit.h"
#include "BDeV/System/Memory/BvMemory.h"
#include "BDeV/Engine/MemorySystem/BvMemorySystem.h"
#include "BDeV/System/Library/BvSharedLib.h"
#include "BDeV/RenderAPI/BvRenderEngine.h"


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
	BV_RTTI(WE);
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


BV_RENUM() enum abc
{
	kConst1,
	kConst2
};


struct bcd
{
	const char* p1;
};
void bcdf1(bcd& b) { b.p1 = BV_FUNCTION; }
void bcdf2(bcd& b) { b.p1 = BV_FUNCTION; }


struct A
{
	A() {}
	A(const A& a) : b(a.b) {} /*
							  test comm */
	BvRobinMap<int, int> b;
};


template<typename Lambda>
struct Func : public BvDelegateBase
{
	template<typename = typename std::enable_if_t<std::is_same_v<std::invoke_result_t<Lambda>, void>&& std::is_invocable_v<Lambda>>>
	Func(Lambda&& lambda)
		: m_Lambda(std::move(lambda))
	{
	}

	~Func() {}

	void Invoke() const override
	{
	}

	Lambda m_Lambda;
};


BV_JOB_FUNCTION(DoSleep)
{
	auto id = (u32)BV_JOB_DATA;
	auto ms = rand() % 30;
	printf("Job #%u in Thread #%llu sleeping for %d ms\n", id, BvThread::GetCurrentThread().GetId(), ms);
	BvThread::Sleep(ms);
}


struct alignas(64) MyStruct
{
	char c[64];
};


int main()
{
	//BvThread g(DoSleep, nullptr);
	//BvConsole::PrintF(0x1f2b4fff, "Hello\n");
	//for (auto i = 0; i < 12; i++)
	//{
	//	BvConsole::PrintF({ 0, u8(30 + i * 20), u8(i * 10)}, "Testing\n");
	//}

	//BvConsole::PrintF({ 20, 60, 120 }, { 244, 200, 210 }, "Test\n");
	//BvConsole::PrintF({ 100, 240, 20 }, "Test2\n");
	//BvConsole::PrintF("Test3\n");
	//BvConsole::PrintF({ 0, 255, 86 }, "Test4\n");
	//BvConsole::PrintF({ 40, 170, 0 }, "Test5\n");

	//i32* uu = BvNew(i32);
	//MyStruct* my = BvNew(MyStruct);
	//i32* uuc = BvNewN(i32, 10);
	//MyStruct* mcy = BvNewN(MyStruct, 10);

	//BvDelete(uu);
	//BvDelete(my);
	//BvDeleteN(uuc);
	//BvDeleteN(mcy);

	//BvTaskT<24> f;
	//f.Set([](int c, int b) {}, 2, 4);

	//JS::JobSystemDesc desc;
	//desc.m_NumWorkerThreads = 1;
	//JS::Initialize(desc);

	//constexpr auto kJobCount = 129;
	//JS::Job jobs[kJobCount]{};
	//srand(time(nullptr));
	//for (auto i = 0; i < kJobCount; i++)
	//{
	//	jobs[i].m_Job.Set(DoSleep, (void*)i);
	//}

	//JS::Counter* pCounter = nullptr;
	//JS::RunJobs(kJobCount, jobs, pCounter);
	//JS::WaitForCounter(pCounter);

	//getchar();

	//JS::Shutdown();

	//BvRobinMap<int, char> mm;
	//mm.Emplace(1, 'c');
	//mm.Emplace(4, 'd');
	//mm.Emplace(6, 'e');
	//mm.Emplace(12, 'f');

	//const auto i = mm.FindKey(1);

	//mm.Erase(1);
	//mm.Erase(4);


	//BV_RUN_TEST_UNITS();

	//BvVector<int> a(10, 3);
	//const BvVector<BvMonitor*>& monitors = GetMonitors();
	//auto pMonitor = GetMonitorFromPoint(-10, 30);

	BvPath paths[3];
	paths[0] = BvPath::FromCurrentDirectory();
	auto list = paths[0].GetFileList(L"*");
	paths[1] = BvPath(LR"(D:\Bruno\C++\BDeV-Engine\BDeV.sln)");
	paths[2] = BvPath(LR"(main.cpp)");

	for (auto&& path : paths)
	{
		auto name = path.GetName();
		auto ext = path.GetExtension();
		auto nameAndExt = path.GetNameAndExtension();
		auto root = path.GetRoot();
		auto relativeRoot = path.GetRelativePath();
		auto absolute = path.GetAbsolutePath();
		printf("Name: %ls\n", name.CStr());
		printf("Ext: %ls\n", ext.CStr());
		printf("NameAndExt: %ls\n", nameAndExt.CStr());
		printf("Relative Path (With Root): %ls\n", relativeRoot.CStr());
		printf("Absolute Path: %ls\n", absolute.CStr());
		printf("Root: %ls\n", root.CStr());

		printf("\n");
	}

	////BvFileSystem::DeleteDirectory(LR"(C:\Programming\C++\MiscTests\x64)");

	//return 0;



	//BvMemoryAllocator<BvHeapAllocator, BvNoLock, BvNoBoundsChecker, BvNoMemoryMarker, BvFullMemoryTracker> all(stack, stack + 1024);
	//BvMemoryAllocatorBase& p = all;
	//auto pp = BvNewArrayA(u8, 200, p);
	//bcd bc1, bc2;
	//bcdf1(bc1);
	//bcdf2(bc2);
	//BvStackTrace st;
	//GetStackTrace(st);
	//for (auto&& f : st.frames) { printf("0x%llX: %s(%u) in %s(%s)\n", f.m_Address, f.m_Function.CStr(), f.m_Line, f.m_Module.CStr(), f.m_File.CStr()); }

	void* p1 = nullptr;
	void* p2 = nullptr;
	void* p3 = nullptr;

	//BvHeapAllocator alloc(stack, stack + 1024);
	//p1 = alloc.Allocate(100, 32); printf("Size: %llu\n", alloc.GetAllocationSize(p1));//alloc.Debug();
	//p2 = alloc.Allocate(200, 16); printf("Size: %llu\n", alloc.GetAllocationSize(p2));//alloc.Debug();
	//p3 = alloc.Allocate(300, 64); printf("Size: %llu\n", alloc.GetAllocationSize(p3));//alloc.Debug();
	//alloc.Debug();
	////auto p4 = alloc.Allocate(400, 16);
	////alloc.Debug();

	//alloc.Free(p1); alloc.Debug();
	//alloc.Free(p2); alloc.Debug();
	//alloc.Free(p3); alloc.Debug();
	////alloc.Free(p4);

	//BvStackAllocator salloc(stack2, stack2 + 1024);
	//p1 = salloc.Allocate(100, 32);
	//p2 = salloc.Allocate(200, 16);
	//p3 = salloc.Allocate(300, 64);

	//salloc.Debug();

	//salloc.Free(p3);
	//salloc.Free(p2);
	//salloc.Free(p1);

	//BvLinearAllocator lalloc(stack2, stack2 + 1024);
	//p1 = lalloc.Allocate(100, 32);
	//p2 = lalloc.Allocate(200, 16);
	//p3 = lalloc.Allocate(300, 64);
	//
	//lalloc.Debug();

	//void* p4 = nullptr;
	//BvPoolAllocator palloc(stack3, stack3 + 100, 32, 32);
	//p1 = palloc.Allocate();
	//p2 = palloc.Allocate();
	//p3 = palloc.Allocate();
	//p4 = palloc.Allocate();

	//palloc.Free(p3);
	//palloc.Free(p1);
	//palloc.Free(p2);
	//palloc.Debug();

	//p1 = BvMAlloc(1024, 32, 4);
	//BvFree(p1);

	//JobTests();
	//RobinMapTests();

	//JobSystemDesc desc;
	//desc.m_NumThreads = 3;
	//BvJobSystem jobSys;
	//jobSys.Initialize(desc);

	//auto pJobList = jobSys.AllocJobList(30, JobListPriority::kHigh);
	//auto pJobList2 = jobSys.AllocJobList(30);
	//for (auto i = 0u; i < 30; i++)
	//{
	//	if (i == 15)
	//	{
	//		pJobList->AddSyncPoint();
	//		continue;
	//	}
	//	if (i == 20)
	//	{
	//		pJobList->AddDependency(pJobList2);
	//		continue;;
	//	}
	//	pJobList->AddJob([i]() { printf("Job %d from list 1 on thread %lld (core %d)\n",
	//		i, BvThread::GetCurrentThread().GetId(), BvThread::GetCurrentProcessor()); });
	//}
	//for (auto i = 0u; i < 30; i++)
	//{
	//	if (i == 15)
	//	{
	//		pJobList2->AddSyncPoint();
	//		continue;
	//	}
	//	pJobList2->AddJob([i]() { printf("Job %d from list 2 on thread %lld (core %d)\n",
	//		i, BvThread::GetCurrentThread().GetId(), BvThread::GetCurrentProcessor()); });
	//}

	//jobSys.Submit(pJobList);
	//jobSys.Submit(pJobList2);

	//BvThread::Sleep(2000);
	//jobSys.Shutdown();

	return 0;
}