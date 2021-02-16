#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/BvDebug.h"
#include "BvCore/Utils/BvUtils.h"


constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;


class BvThread
{
	BV_NOCOPY(BvThread);

public:
	typedef void(*ThreadFunction)(void*);

	BvThread();
	BvThread(BvThread && rhs);
	BvThread & operator=(BvThread && rhs);
	explicit BvThread(ThreadFunction pFunction, void * pData = nullptr);
	~BvThread();

	void Start();
	void Start(ThreadFunction pFunction, void * pData = nullptr);
	void Wait();
	void SetAffinity(const u32 affinityMask) const;
	void Destroy();

	static void Sleep(const u32 miliseconds);
	static const BvThread & GetThisThread();
	static const u32 GetCurrentProcessor();

	BV_INLINE const u64 GetId() const { return m_ThreadId; }
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	BV_INLINE const HANDLE GetHandle() const { return m_hThread; }
#endif

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