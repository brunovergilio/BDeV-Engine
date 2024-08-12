#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvDelegate.h"
#include "BDeV/Core/System/Memory/BvMemory.h"


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
	enum class Priority : u8
	{
		kAuto,
		kNormal,
		kAboveNormal,
		kHighest,
		kTimeCritical,
		kBelowNormal,
		kLowest,
	};

	struct CreateInfo
	{
		u64 m_AffinityMask; // Bitmask determining which cores this thread can run on (0 = all logical processors)
		u32 m_StackSize; // Stack size in bytes for the thread (0 = let system decide)
		Priority m_Priority; // Priority this thread will have
		bool m_CreateSuspended; // Start the thread right away
	};

	BvThread();
	BvThread(BvThread&& rhs) noexcept;
	BvThread& operator=(BvThread&& rhs) noexcept;
	~BvThread();

	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
	BvThread(Fn&& fn, Args&&... args)
		: BvThread(CreateInfo{}, std::forward<Fn>(fn), std::forward<Args>(args)...) {}

	template<class Fn, class... Args,
		typename = typename std::enable_if_t<std::is_invocable_v<Fn, Args...> && !std::is_integral_v<Fn>>>
	BvThread(const CreateInfo& createInfo, Fn&& fn, Args&&... args)
		: m_pDelegate (new((void*)BvNewN(u8, sizeof(BvDelegate<Fn, Args...>))) BvDelegate<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...))
	{
		Create(createInfo);
	}

	void Wait();
	void SetAffinityMask(u64 affinityMask) const;
	void LockToCore(u32 coreIndex) const;
	void SetName(const char* pThreadName) const;
	void SetPriority(Priority priority) const;

	static void Sleep(u32 miliseconds);
	static void YieldExecution();
	static const BvThread& GetCurrentThread();
	static u32 GetCurrentProcessor();
	static void ConvertToFiber();
	static void ConvertFromFiber();

	BV_INLINE u64 GetId() const { return m_ThreadId; }
	BV_INLINE void* GetHandle() const { return m_hThread; }
	const BvFiber& GetThreadFiber() const;
	bool IsFiber() const;

private:
	void Create();
	void Create(const CreateInfo& createInfo);
	void Destroy();

private:
	BvDelegateBase* m_pDelegate = nullptr;
	u64 m_ThreadId = 0;
	void* m_hThread = nullptr;
};