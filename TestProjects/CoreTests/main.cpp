#include "BDeV/BDeV.h"
#include <iostream>
#include <fstream>
#include "BDeV/Core/Utils/BvRTTI.h"
#include <format>

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


struct alignas(64) MyStruct
{
	char c[64];
};

constexpr BvUUID fa1 = MakeUUIDv4("7b134b6e-b092-465f-9c00-af77a562ba2b");
constexpr BvUUID fa2 = MakeUUIDv4("7b134b6e-b092-465f-9c00-a127a562ba2b");
constexpr BvUUID fa3 = MakeUUIDv4("7b134b6e-b092-465f-9c00-af77a5cdba2b");


#include <iostream>
#include <utility>


void abc(int f)
{
	printf("abc %d\n", f);
}

struct def
{
	def()
	{
		printf("def c\n");
	}
	void abc(int f)
	{
		printf("def %d\n", f);
	}

	void fff(def e)
	{
		e.abc(9);
	}

	void cc(int ff) const
	{
		printf("%d %d\n", ff, ff);
	}
};


int hij(int g)
{
	return g * 2;
}


thread_local int a = 0;
BvFiber* pFb = nullptr;
BvFiber* pFb2 = nullptr;


struct GG
{
	template<typename T = int>
	void PrintMe()
	{
		printf("%s\n", BV_FUNCTION);
	}
};


int main()
{
	GG ff;
	ff.PrintMe();
	ff.PrintMe<f32>();
	//BvFiber& mfb = BvThread::ConvertToFiber();

	//BvSignal s;
	//BvFiber fb;
	//fb = BvFiber([&fb, &mfb]()
	//	{
	//		printf("%d\n", a);
	//		fb.Switch(mfb);

	//		printf("%d\n", a);
	//		fb.Switch(*pFb2);

	//		printf("%d\n", a);
	//		fb.Switch(mfb);
	//	});
	//pFb = &fb;
	//mfb.Switch(fb);

	//MessageBoxW(nullptr, L"afaf", L"fa", MB_OK);

	//BvThread t([]()
	//	{
	//		BvFiber& mfb = BvThread::ConvertToFiber();
	//		a = 1;

	//		pFb2 = &mfb;
	//		mfb.Switch(*pFb);
	//	});
	//t.Wait();

	//mfb.Switch(fb);

	return 0;
}