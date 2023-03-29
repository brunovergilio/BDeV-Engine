#include "BDeV/Container/BvFixedVector.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/Container/BvRobinMap.h"
#include "BDeV/System/File/BvAsyncFile.h"
#include "BDeV/System/File/BvFileSystem.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Engine/TBJobSystem/BvTBJobSystem.h"
#include <functional>
#include "BDeV/System/Memory/BvMemory.h"
#include "BDeV/Utils/Hash.h"
#include "BDeV/BvCore.h"
#include "BDeV/Reflection/BvObjectInfo.h"
#include <Windows.h>
#include "BDeV/System/File/BvPath.h"
#include "BDeV/System/Window/BvMonitor.h"
#include "BDeV/Reflection/RTTI.h"
#include "BDeV/System/Debug/BvDebug.h"


char stack[1024];
char stack2[1024];
char stack3[1024];
char stack4[100];


struct WE
{
	int f();
	int ff() {}
	int a = int(0);
	int (*A)();
};


BV_RSTRUCT() struct TestStruct
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


BV_RSTRUCT() class TestStruct2 : public TestStruct
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


int main()
{
	//const BvVector<BvMonitor*>& monitors = GetMonitors();
	PrintF(ConsoleColor::kLightGreen, "Test\n");
	//auto pMonitor = GetMonitorFromPoint(-10, 30);

	//BvPath paths[3];
	//paths[0] = BvPath::FromCurrentDirectory();
	//auto list = paths[0].GetFileList(L"*");
	//paths[1] = BvPath(LR"(C:\Programming\C++\Graphics\Vulkan\Vulkan.sln)");
	//paths[2] = BvPath(LR"(main.cpp)");

	//for (auto&& path : paths)
	//{
	//	auto name = path.GetName();
	//	auto ext = path.GetExt();
	//	auto nameAndExt = path.GetNameAndExt();
	//	auto root = path.GetRoot();
	//	auto relativeRoot = path.GetRelativeName();
	//	auto relative = path.GetRelativeName(LR"(C:\Programming)");
	//	auto relative2 = path.GetRelativeName(LR"(C:\Programming\Git\BDeV-Engine\TestProjects\CoreTests\CoreTests.vcxproj)");
	//	auto absolute = path.GetAbsoluteName();
	//	printf("Name: %ls\n", name.CStr());
	//	printf("Ext: %ls\n", ext.CStr());
	//	printf("NameAndExt: %ls\n", nameAndExt.CStr());
	//	printf("Relative Path (With Root): %ls\n", relativeRoot.CStr());
	//	printf("Relative Path (With C:\\Programming): %ls\n", relative.CStr());
	//	printf("Relative Path2 : %ls\n", relative2.CStr());
	//	printf("Absolute Path: %ls\n", absolute.CStr());
	//	printf("Root: %ls\n", root.CStr());

	//	printf("\n");
	//}

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

	BvLinearAllocator lalloc(stack2, stack2 + 1024);
	p1 = lalloc.Allocate(100, 32);
	p2 = lalloc.Allocate(200, 16);
	p3 = lalloc.Allocate(300, 64);

	lalloc.Debug();

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