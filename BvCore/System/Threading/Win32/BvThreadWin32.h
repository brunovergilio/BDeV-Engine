#pragma once


#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Utils/BvDelegate.h"
#include "BvCore/Utils/BvDebug.h"
#include <Windows.h>


#if defined(Yield)
#undef Yield
#endif


class BvThread
{
	BV_NOCOPY(BvThread);

public:
	BvThread();
	BvThread(BvThread&& rhs) noexcept;
	BvThread& operator=(BvThread&& rhs) noexcept;
	~BvThread();

	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
		BvThread(Fn&& fn, Args &&... args)
		: BvThread(0, std::forward<Fn>(fn), std::forward<Args>(args)...) {}

	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
		BvThread(const u32 stackSize, Fn&& fn, Args &&... args)
		: m_pDelegate(new BvDelegate<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...)) {
		Create(stackSize);
	}

	void Wait();
	void SetAffinity(const u32 affinityMask) const;

	static void Sleep(const u32 miliseconds);
	static void Yield();
	static const BvThread& GetCurrentThread();
	static const u32 GetCurrentProcessor();

	BV_INLINE const u64 GetId() const { return m_ThreadId; }
	BV_INLINE const HANDLE GetHandle() const { return m_hThread; }

private:
	void Create(const u32 stackSize = 0);
	void Destroy();

private:
	BvDelegateBase* m_pDelegate = nullptr;
	u64 m_ThreadId = 0;
	HANDLE m_hThread = nullptr;
};