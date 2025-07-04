#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Utils/BvTask.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"


class BvFiber;


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
		const char* m_pName; // Name for the thread
		u32 m_StackSize; // Stack size in bytes for the thread (0 = let system decide)
		Priority m_Priority; // Priority this thread will have
		bool m_CreateSuspended; // Create the thread but don't run it right away
		u32 m_LogicalProcessorIndexCount; // How many processors to set affinity for
		const u32* m_pLogicalProcessorIndices; // Which processors to set affinity for
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
		: m_pTask(new((void*)BV_NEW_ARRAY(u8, sizeof(Internal::BvTaskT<Fn, Args...>))) Internal::BvTaskT<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...))
	{
		Create(createInfo);
	}

	void Start();
	void Wait();
	void SetAffinity(u32 logicalProcessorIndex) const;
	void SetAffinity(u32 logicalProcessorIndexCount, const u32* logicalProcessorIndices) const;
	void SetName(const char* pThreadName) const;
	void SetPriority(Priority priority) const;

	static void Sleep(u32 miliseconds);
	static void Yield();
	static const BvThread& GetCurrentThread();
	static u32 GetCurrentProcessor();

	static BvFiber& ConvertToFiber();
	static void ConvertFromFiber();
	static BvFiber& GetFiber();
	static bool IsFiber();

	BV_INLINE u64 GetId() const { return m_ThreadId; }
	BV_INLINE OSThreadHandle GetHandle() const { return m_hThread; }

private:
	void Create(const CreateInfo& createInfo);
	void Destroy();

private:
	u64 m_ThreadId = 0;
	OSThreadHandle m_hThread = nullptr;
	IBvTask* m_pTask = nullptr;
	bool m_IsRunning = false;
};