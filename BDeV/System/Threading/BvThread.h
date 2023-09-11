#pragma once


#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvDelegate.h"


class BvFiber;


enum class ThreadPriority : u8
{
	kNormal,
	kAboveNormal,
	kHighest,
	kTimeCritical,
	kBelowNormal,
	kLowest,
};


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
		: m_pDelegate(new BvDelegate<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...))
	{
		Create(stackSize);
	}

	void Wait();
	void SetAffinityMask(const u64 affinityMask) const;
	void LockToCore(u32 coreIndex) const;
	void SetName(const char* pThreadName) const;
	void SetPriority(ThreadPriority priority) const;

	static void Sleep(const u32 miliseconds);
	static void YieldExecution();
	static const BvThread& GetCurrentThread();
	static u32 GetCurrentProcessor();
	static void ConvertToFiber();
	static void ConvertFromFiber();

	BV_INLINE u64 GetId() const { return m_ThreadId; }
	BV_INLINE const void* GetHandle() const { return m_hThread; }
	const BvFiber& GetThreadFiber() const;
	bool IsFiber() const;

private:
	void Create(const u32 stackSize = 0);
	void Destroy();

private:
	BvDelegateBase* m_pDelegate = nullptr;
	u64 m_ThreadId = 0;
	void* m_hThread = nullptr;
};