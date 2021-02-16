#include "BvCore/Container/BvFixedVector.h"
#include "BvCore/Container/BvVector.h"
#include "BvCore/Container/BvRobinMap.h"
#include "BvCore/System/Threading/BvAtomic.h"
#include "BvCore/System/File/BvAsyncFile.h"
#include "BvCore/System/File/BvFileSystem.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/System/JobSystem/BvJobSystem.h"


void RobinMapTests();
void FileTests();
void AtomicTests();
void JobTests();

int main()
{
	//JobTests();
	//RobinMapTests();
	BV_ERROR("Erro no Abc");
	SetLastError(0x4);
	BV_WIN32_ERROR();
	struct A
	{
		A() { BV_ERROR("Erro"); }
	};
	A f;
	return 0;
}


void RobinMapTests()
{
	BvRobinMap<int, int> rm(20);
	rm.Emplace(1, 1);
	rm.Emplace(2, 2);
	rm.Emplace(3, 3);
	rm.Erase(2);
}

void FileTests()
{
	BvFileSystem fs;
	BvAsyncFile file = fs.OpenAsyncFile(R"RAW(C:\Games\Diablo II\PlugY.log)RAW");
	u32 res = 0;
	char buf[4096]{};
	{
		auto request = file.Read(buf, 4096);
		request.GetResult();

		auto b = request;
	}
}


void AtomicTests()
{
	struct f
	{
		char f[9];
	};
	BvAtomic<i8> d;
	BvAtomic<char> c;
	auto s = d.Sub(1);
	s = d.Load();
	auto j = c.Load();
	d -= 2;

	BvAtomic<u32> ff;
	auto e = ff.Sub(3) - 3;
	auto w = ff.CompareExchange(5, e);

	BvAtomic<i64> tt;
	auto z1 = tt.Add(-234);
	auto z2 = tt.CompareExchange(232, 2);
	auto z3 = tt.CompareExchange(232, tt.Load());
	
	// Uncomment for error
	//BvAtomic<f> g;
}

void TestMe(int a)
{
	a;
}

void JobTests()
{
	BvVector<BvJobCounter> c(10);
	int a = 3, k = 2;
	BvJob jj;
	{
		BvJob j([a, k](int g, int c) { printf("%d\n", a + k + g + c); }, 1, 1);
		jj = std::move(j);
	}
	jj.Run();
}