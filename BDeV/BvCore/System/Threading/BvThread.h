#pragma once


#include "BvCore/BvDefines.h"
#include "BvCore/BvDebug.h"


constexpr u32 kCacheLineSize = 64;


class BvThread
{
public:
	typedef void(*ThreadFunction)(void*);

	BvThread();
	BvThread(BvThread && rhs);
	BvThread & operator=(BvThread && rhs);
	BvThread(const BvThread &) = delete;
	BvThread & operator=(const BvThread &) = delete;
	explicit BvThread(ThreadFunction pFunction, void * pData = nullptr);
	~BvThread();

	void Start();
	void Start(ThreadFunction pFunction, void * pData = nullptr);
	void Wait();
	void SetAffinity(const u32 affinityMask);
	void Destroy();

	BV_INLINE const u64 GetId() const { return m_ThreadId; }
	BV_INLINE const HANDLE GetHandle() const { return m_hThread; }

	static const BvThread & GetThisThread();

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	static unsigned int CALLBACK ThreadEntryPoint(void * pData);
#endif

private:
	ThreadFunction m_pFunction = nullptr;
	u64 m_ThreadId = 0;
	void * m_pData = nullptr;

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HANDLE m_hThread = nullptr;
#endif
};