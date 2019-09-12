#include "BvCore/System/Threading/BvThread.h"
#include "BvCore/BvDebug.h"
#include <utility>


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


#include <process.h>


BvThread::BvThread()
{
}


BvThread::BvThread(BvThread && rhs)
{
	*this = std::move(rhs);
}


BvThread & BvThread::operator =(BvThread && rhs)
{
	if (this != &rhs)
	{
		m_ThreadId = rhs.m_ThreadId;
		m_hThread = rhs.m_hThread;
		m_pFunction = rhs.m_pFunction;
		m_pData = rhs.m_pData;

		rhs.m_ThreadId = 0;
		rhs.m_hThread = nullptr;
		rhs.m_pFunction = nullptr;
		rhs.m_pData = nullptr;
	}

	return *this;
}


BvThread::BvThread(ThreadFunction pFunction, void * pData)
	: m_pFunction(pFunction), m_pData(pData)
{
}


BvThread::~BvThread()
{
	Destroy();
}


void BvThread::Start()
{
	BvAssertMsg(m_hThread == nullptr, "Thread already started");
	BvAssertMsg(m_pFunction != nullptr, "Thread doesn't have a function specified");

	m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0U, ThreadEntryPoint, this, 0U, reinterpret_cast<u32 *>(&m_ThreadId)));
}


void BvThread::Start(ThreadFunction pFunction, void * pData)
{
	m_pFunction = pFunction;
	m_pData = pData;

	Start();
}


void BvThread::Wait()
{
	BvAssertMsg(m_hThread != nullptr, "Thread handle is invalid");

	WaitForSingleObject(m_hThread, INFINITE);
}


void BvThread::SetAffinity(const u32 affinityMask)
{
	BvAssertMsg(m_hThread != nullptr, "Thread handle is invalid");

	SetThreadAffinityMask(m_hThread, 1ull << static_cast<DWORD_PTR>(affinityMask));
}


void BvThread::Destroy()
{
	if (m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = nullptr;
	}
}


const BvThread & BvThread::GetThisThread()
{
	static BvThreadLocal BvThread thisThread;
	if (!thisThread.m_hThread)
	{
		thisThread.m_hThread = GetCurrentThread();
		thisThread.m_ThreadId = GetThreadId(thisThread.m_hThread);
	}

	return thisThread;
}


unsigned int BvThread::ThreadEntryPoint(void * pData)
{
	BvThread * pThread = reinterpret_cast<BvThread *>(pData);
	pThread->m_pFunction(pThread->m_pData);

	return 0;
}


#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)