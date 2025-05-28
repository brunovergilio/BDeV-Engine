#include "BvDiagnostics.h"
#include <stdarg.h>
#include <utility>


void BvLoggerManager::RegisterLogger(IBvLogger* pLogger)
{
	auto& pHead = m_pLoggerList;
	pLogger->SetNext(pHead);
	if (pHead)
	{
		pHead->SetPrev(pLogger);
	}
	pHead = pLogger;
}

void BvLoggerManager::UnregisterLogger(IBvLogger* pLogger)
{
	auto pPrev = pLogger->GetPrev();
	auto pNext = pLogger->GetNext();

	if (pNext)
	{
		pNext->SetPrev(pPrev);
	}

	if (pPrev)
	{
		pPrev->SetNext(pNext);
	}

	auto& pHead = m_pLoggerList;
	if (pHead == pLogger)
	{
		pHead = pNext;
	}
}

void BvLoggerManager::Dispatch(const BvLogInfo& logInfo)
{
	auto pLogger = m_pLoggerList;
	while (pLogger)
	{
		pLogger->Log(logInfo);
		pLogger = pLogger->GetNext();
	}
}


namespace Logging
{
	BvLoggerManager*& GetLoggerManager()
	{
		static BvLoggerManager* pLoggerManager = nullptr;
		return pLoggerManager;
	}

	void SetLoggerManager(BvLoggerManager* pLoggerManager)
	{
		GetLoggerManager() = pLoggerManager;
	}

	void Dispatch(const char* pChannel, u32 level, u32 verbosity, const BvSourceInfo& sourceInfo, const char* pMessage, ...)
	{
		constexpr u32 kMaxLogMessageSize = 2_kb;

		static thread_local char buffer[kMaxLogMessageSize]{};
		va_list args;
		va_start(args, pMessage);
		vsnprintf(buffer, kMaxLogMessageSize, pMessage, args);
		va_end(args);

		if (auto pLoggerManager = GetLoggerManager())
		{
			const BvLogInfo logInfo{ sourceInfo, pMessage, pChannel, level, verbosity };
			pLoggerManager->Dispatch(logInfo);
		}
		else
		{
			// Use default logging mechanism
			constexpr BvColorI kDefaultColors[] =
			{
				BvColorI(255, 255, 255),
				BvColorI(0, 255, 0),
				BvColorI(255, 255, 0),
				BvColorI(255, 165, 0),
				BvColorI(255, 0, 0)
			};

			constexpr const char* kDefaultLevels[] =
			{
				"Trace",
				"Info",
				"Warning",
				"Error",
				"Fatal"
			};

			if (level >= ArraySize(kDefaultColors))
			{
				level = 0;
			}

			Console::Print(kDefaultColors[level], "[%s] [%s]: %s - Function: %s (%s [%d])\n", kDefaultLevels[level], pChannel, pMessage, sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
		}
	}
}