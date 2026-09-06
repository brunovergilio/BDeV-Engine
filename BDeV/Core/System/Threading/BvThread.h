#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"


class BvFiber;


class BvThread
{
	BV_NOCOPY(BvThread);
	
	struct TaskData
	{
		void* m_pObj;
		void(*m_pInvokeFn)(void*);
		IBvMemoryArena* m_pArena;
	};

	template<class Fn, typename... Args>
	struct Callable
	{
		Callable(Fn&& fn, Args&&... args)
			: m_Fn(std::forward<Fn>(fn)), m_Args(std::forward<Args>(args)...)
		{}
		~Callable() = default;

		void Invoke()
		{
			std::apply(std::forward<Fn>(m_Fn), m_Args);
		}

		static TaskData* GetTaskData(IBvMemoryArena* pArena, Fn&& fn, Args&&... args)
		{
			auto sizeNeeded = sizeof(TaskData) + alignof(Callable) + sizeof(Callable);
			auto pMem = reinterpret_cast<u8*>(pArena->Allocate(sizeNeeded, alignof(TaskData)));
			auto pTaskData = reinterpret_cast<TaskData*>(pMem);
			pMem = reinterpret_cast<u8*>(BvMemory::AlignMemory(pMem + sizeof(TaskData), alignof(Callable)));
			BV_CHECK_ALIGNMENT(pMem, alignof(Callable));
			auto pCallable = new(pMem) Callable(std::forward<Fn>(fn), std::forward<Args>(args)...);

			pTaskData->m_pObj = pCallable;
			pTaskData->m_pInvokeFn = [](void* pObj)
				{
					reinterpret_cast<Callable*>(pObj)->Invoke();
				};
			pTaskData->m_pArena = pArena;
			
			return pTaskData;
		}

		std::decay_t<Fn> m_Fn;
		std::tuple<std::decay_t<Args>...> m_Args;
	};

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
		IBvMemoryArena* m_pArena; // Default memory arena
	};

	BvThread();
	BvThread(BvThread&& rhs) noexcept;
	BvThread& operator=(BvThread&& rhs) noexcept;
	~BvThread();

	template<class Fn, typename... Args>
	BvThread(Fn&& fn, Args&&... args)
		requires (std::is_invocable_v<Fn, Args...> && !std::is_same_v<std::remove_cvref_t<Fn>, BvThread> && !std::is_same_v<std::remove_cvref_t<Fn>, CreateInfo>)
		: BvThread(CreateInfo{}, std::forward<Fn>(fn), std::forward<Args>(args)...) {}

	template<class Fn, typename... Args>
	BvThread(const CreateInfo& createInfo, Fn&& fn, Args&&... args)
		requires (std::is_invocable_v<Fn, Args...> && !std::is_same_v<std::remove_cvref_t<Fn>, BvThread> && !std::is_same_v<std::remove_cvref_t<Fn>, CreateInfo>)
	{
		m_pTaskData = Callable<Fn, Args...>::GetTaskData(createInfo.m_pArena ? createInfo.m_pArena : BV_DEFAULT_MEMORY_ARENA, std::forward<Fn>(fn), std::forward<Args>(args)...);

		Create(createInfo);
	}

	void Start();
	void Join();
	void SetAffinity(u32 logicalProcessorIndex) const;
	void SetAffinity(u32 logicalProcessorIndexCount, const u32* logicalProcessorIndices) const;
	void SetName(const char* pThreadName) const;
	void SetPriority(Priority priority) const;

	static void Sleep(u32 miliseconds);
	static void Yield();
	static const BvThread& GetCurrentThread();
	static u32 GetCurrentProcessor();

	BV_INLINE u64 GetId() const { return m_ThreadId; }
	BV_INLINE OSThreadHandle GetHandle() const { return m_hThread; }

private:
	void Create(const CreateInfo& createInfo);
	void Destroy();

#if BV_PLATFORM_WIN32
	static u32 CALLBACK ThreadEntryPoint(void* pData);
#endif

private:
	TaskData* m_pTaskData = nullptr;
	u64 m_ThreadId = 0;
	OSThreadHandle m_hThread = nullptr;
	bool m_IsRunning = false;
};