#pragma once


#include "BDeV/Container/BvString.h"
#include "BDeV/Container/BvRobinMap.h"


namespace Internal
{
	struct Test
	{
		const char* pUnitName;
		const char* pName;
		bool result;
	};

	struct TestUnitBase
	{
		virtual ~TestUnitBase() {}
		virtual const char* GetTestUnitName() = 0;
		virtual void Run() = 0;

		BvVector<Test> m_Tests;
	};

	class BV_API TestUnitSystem
	{
	public:
		TestUnitSystem() {}
		~TestUnitSystem() {}

		void AddTestUnit(Internal::TestUnitBase* pTestUnit);
		void Run();

	private:
		BvVector<Internal::TestUnitBase*> m_TestUnits;
		BvRobinMap<const char*, BvVector<Internal::Test*>> m_SuccessfulTests;
		BvRobinMap<const char*, BvVector<Internal::Test*>> m_FailedTests;
	};
}


#define BV_INIT_TEST_UNIT_SYSTEM() Internal::TestUnitSystem g_TestUnitSystem;
#define BV_RUN_TEST_UNITS() g_TestUnitSystem.Run();
#define BV_TEST_UNIT(unitName) void RunTestUnit_##unitName(Internal::TestUnitBase* pTestUnit); \
struct TestUnit_##unitName final : public Internal::TestUnitBase 							   \
{																							   \
	TestUnit_##unitName()																	   \
	{																						   \
		g_TestUnitSystem.AddTestUnit(this);													   \
	}																						   \
	const char* GetTestUnitName() override													   \
	{																						   \
		return #unitName;																	   \
	}																						   \
	void Run() override																		   \
	{																						   \
		RunTestUnit_##unitName(this);														   \
	}																						   \
} g_##unitName;																				   \
void RunTestUnit_##unitName(Internal::TestUnitBase* pTestUnit)

#define BV_TEST(test) pTestUnit->m_Tests.PushBack({ pTestUnit->GetTestUnitName(), #test, (test) });