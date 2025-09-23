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

	template<class Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn> && !std::is_integral_v<Fn>>>
	BvThread(Fn&& fn)
		: BvThread(CreateInfo{}, std::forward<Fn>(fn)) {}

	template<class Fn, typename = typename std::enable_if_t<std::is_invocable_v<Fn> && !std::is_integral_v<Fn>>>
	BvThread(const CreateInfo& createInfo, Fn&& fn)
		: m_Task(std::forward<Fn>(fn))
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

#if BV_PLATFORM == BV_PLATFORM_WIN32
	static u32 CALLBACK ThreadEntryPoint(void* pData);
#endif

private:
	static constexpr auto kTaskSize = 32;

	BvMTask<kTaskSize> m_Task;
	u64 m_ThreadId = 0;
	OSThreadHandle m_hThread = nullptr;
	bool m_IsRunning = false;
};