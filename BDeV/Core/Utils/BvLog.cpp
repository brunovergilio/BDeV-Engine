#include "BvLog.h"


class BvLoggerController
{
public:
	static IBvLogger*& GetLoggerHead()
	{
		static IBvLogger* s_pLogger = nullptr;
		return s_pLogger;
	}

	static void RegisterLogger(IBvLogger* pLogger)
	{
		auto& pHead = GetLoggerHead();
		pLogger->SetNext(pHead);
		if (pHead)
		{
			pHead->SetPrev(pLogger);
		}
		pHead = pLogger;
	}

	static void UnregisterLogger(IBvLogger* pLogger)
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

		auto& pHead = GetLoggerHead();
		if (pHead == pLogger)
		{
			pHead = pNext;
		}
	}

	static void Dispatch(const BvLogInfo& logInfo)
	{
		auto& pHead = GetLoggerHead();
		auto pLogger = pHead;
		while (pLogger)
		{
			pLogger->Log(logInfo);
			pLogger = pLogger->GetNext();
		}
	}
};


namespace Logging
{
	void RegisterLogger(IBvLogger* pLogger)
	{
		BvLoggerController::RegisterLogger(pLogger);
	}

	void UnregisterLogger(IBvLogger* pLogger)
	{
		BvLoggerController::UnregisterLogger(pLogger);
	}

	void Dispatch(const char* pChannel, u32 level, u32 verbosity, const BvSourceInfo& sourceInfo, const char* pMessage, ...)
	{
		va_list args1;
		va_start(args1, pMessage);
		va_list args2;
		va_copy(args2, args1);
		auto sizeNeeded = vsnprintf(nullptr, 0, pMessage, args1) + 1;
		va_end(args1);
		char* pBuffer = (char*)BV_STACK_ALLOC(sizeNeeded);
		vsnprintf(pBuffer, sizeNeeded, pMessage, args2);
		va_end(args2);

		const BvLogInfo logInfo{ sourceInfo, pMessage, pChannel, level, verbosity };
		BvLoggerController::Dispatch(logInfo);
	}
}