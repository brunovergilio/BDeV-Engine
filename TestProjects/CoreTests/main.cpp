#include "BvCore/Container/BvFixedVector.h"
#include "BvCore/Container/BvVector.h"
#include "BvCore/Container/BvRobinMap.h"
#include "BvCore/System/File/BvAsyncFile.h"
#include "BvCore/System/File/BvFileSystem.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/System/JobSystem/BvJobSystem.h"
#include <functional>
#include "BvCore/System/Memory/BvMemory.h"
#include "BvCore/Utils/Hash.h"
#include "BvCore/BvCore.h"


char stack[1024];
char stack2[1024];
char stack3[1024];
char stack4[100];

struct bcd
{
	const char* p1;
};
void bcdf1(bcd& b) { b.p1 = BV_FUNCTION; }
void bcdf2(bcd& b) { b.p1 = BV_FUNCTION; }


int main()
{
	BvMemoryAllocator<BvHeapAllocator, BvNoLock, BvNoBoundsChecker, BvNoMemoryMarker, BvFullMemoryTracker> all(stack, stack + 1024);
	BvMemoryAllocatorBase& p = all;
	auto pp = BvNewArrayA(u8, 200, p);
	bcd bc1, bc2;
	bcdf1(bc1);
	bcdf2(bc2);
	BvStackTrace st;
	GetStackTrace(st);
	for (auto&& f : st.frames) { printf("0x%llX: %s(%u) in %s(%s)\n", f.m_Address, f.m_Function.CStr(), f.m_Line, f.m_Module.CStr(), f.m_File.CStr()); }
	BvHeapAllocator alloc(stack, stack + 1024);

	auto p1 = alloc.Allocate(100, 32); printf("Size: %llu\n", alloc.GetAllocationSize(p1));//alloc.Debug();
	auto p2 = alloc.Allocate(200, 16); printf("Size: %llu\n", alloc.GetAllocationSize(p2));//alloc.Debug();
	auto p3 = alloc.Allocate(300, 64); printf("Size: %llu\n", alloc.GetAllocationSize(p3));//alloc.Debug();
	alloc.Debug();
	//auto p4 = alloc.Allocate(400, 16);
	//alloc.Debug();

	alloc.Free(p1); alloc.Debug();
	alloc.Free(p2); alloc.Debug();
	alloc.Free(p3); alloc.Debug();
	//alloc.Free(p4);

	BvStackAllocator salloc(stack2, stack2 + 1024);
	p1 = salloc.Allocate(100, 32);
	p2 = salloc.Allocate(200, 16);
	p3 = salloc.Allocate(300, 64);

	salloc.Debug();

	salloc.Free(p3);
	salloc.Free(p2);
	salloc.Free(p1);

	BvLinearAllocator lalloc(stack2, stack2 + 1024);
	p1 = lalloc.Allocate(100, 32);
	p2 = lalloc.Allocate(200, 16);
	p3 = lalloc.Allocate(300, 64);

	lalloc.Debug();

	void* p4 = nullptr;
	BvPoolAllocator palloc(stack3, stack3 + 100, 32, 32);
	p1 = palloc.Allocate();
	p2 = palloc.Allocate();
	p3 = palloc.Allocate();
	p4 = palloc.Allocate();

	palloc.Free(p3);
	palloc.Free(p1);
	palloc.Free(p2);
	palloc.Debug();

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