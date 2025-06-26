#include "BvTestUnit.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"

namespace Internal
{
	void TestUnitSystem::AddTestUnit(Internal::TestUnitBase* pTestUnit)
	{
		m_TestUnits.EmplaceBack(pTestUnit);
	}


	void TestUnitSystem::Run()
	{
		if (m_TestUnits.Empty())
		{
			BvDebug::Print("Unit testing was requested but there are no tests:\n\n");
			return;
		}

		BvDebug::Print("Running test units...\n\n");
		for (auto&& pTestUnit : m_TestUnits)
		{
			// This queues up all tests in the unit
			pTestUnit->Run();

			for (auto&& test : pTestUnit->m_Tests)
			{
				if (test.result)
				{
					m_SuccessfulTests[pTestUnit->GetTestUnitName()].EmplaceBack(&test);
				}
				else
				{
					m_FailedTests[pTestUnit->GetTestUnitName()].EmplaceBack(&test);
				}
			}
		}

		BvDebug::Print("Successful tests:\n");
		for (auto&& testUnit : m_SuccessfulTests)
		{
			BvDebug::Print("\tUnit: %s\n", testUnit.first);
			for (auto&& test : testUnit.second)
			{
				BvDebug::Print("\t\t%s\n", test->pName);
			}
		}

		BvDebug::Print("\nFailed tests:\n");
		for (auto&& testUnit : m_FailedTests)
		{
			BvDebug::Print("\tUnit: %s\n", testUnit.first);
			for (auto&& test : testUnit.second)
			{
				BvDebug::Print("\t\t%s\n", test->pName);
			}
		}
	}
}