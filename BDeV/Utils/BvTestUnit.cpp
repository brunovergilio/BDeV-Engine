#include "BvTestUnit.h"
#include "BDeV/System/Debug/BvDebug.h"

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
			DPrintF("Unit testing was requested but there are no tests:\n\n");
			return;
		}

		DPrintF("Running test units...\n\n");
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

		DPrintF("Successful tests:\n");
		for (auto&& testUnit : m_SuccessfulTests)
		{
			DPrintF("\tUnit: %s\n", testUnit.first);
			for (auto&& test : testUnit.second)
			{
				DPrintF("\t\t%s\n", test->pName);
			}
		}

		DPrintF("\nFailed tests:\n");
		for (auto&& testUnit : m_FailedTests)
		{
			DPrintF("\tUnit: %s\n", testUnit.first);
			for (auto&& test : testUnit.second)
			{
				DPrintF("\t\t%s\n", test->pName);
			}
		}
	}
}