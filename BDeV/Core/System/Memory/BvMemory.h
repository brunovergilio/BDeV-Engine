#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


namespace BvMemory
{
	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t offset = 0);
	void Free(void* pMem);
	size_t GetAllocationSize(void* pMem);
	void* AlignMemory(void* pMem, size_t alignment);
}


namespace BvVirtualMemory
{
	void* Reserve(size_t size);
	bool Commit(void* pAddress, size_t size);
	void* ReserveAndCommit(size_t size, bool useLargePage = false);
	bool Decommit(void* pAddress, size_t size);
	bool Release(void* pAddress);
	size_t GetPageSize();
	size_t GetLargePageSize();
};


// Default allocator, using basic malloc / free functions to allocate memory with custom alignment and offset. Also
// serves as base example for other allocators, some of which may not be copy or move constructible / assignable.
class BvDefaultAllocator
{
public:
	BvDefaultAllocator() {}
	BvDefaultAllocator(const BvDefaultAllocator&) {}
	BvDefaultAllocator(BvDefaultAllocator&&) noexcept {}
	BvDefaultAllocator& operator=(const BvDefaultAllocator&) { return *this; }
	BvDefaultAllocator& operator=(BvDefaultAllocator&&) noexcept { return *this; }
	~BvDefaultAllocator() {}

	BV_INLINE void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0)
	{
		return BvMemory::Allocate(size, alignment, alignmentOffset);
	}

	BV_INLINE void Free(void* pMem)
	{
		BvMemory::Free(pMem);
	}

	BV_INLINE size_t GetAllocationSize(void* pMem) const
	{
		return BvMemory::GetAllocationSize(pMem);
	}
};


// An allocator that does nothing, serving only for debugging purposes.
class BvDummyAllocator
{
public:
	BvDummyAllocator() {}
	BvDummyAllocator(const BvDummyAllocator&) {}
	BvDummyAllocator(BvDummyAllocator&&) noexcept {}
	BvDummyAllocator& operator=(const BvDummyAllocator&) { return *this; }
	BvDummyAllocator& operator=(BvDummyAllocator&&) noexcept { return *this; }
	~BvDummyAllocator() {}

	BV_INLINE void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0)
	{
		return nullptr;
	}

	BV_INLINE void Free(void* pMem)
	{
	}

	BV_INLINE size_t GetAllocationSize(void* pMem) const
	{
		return 0;
	}
};


namespace Internal
{
	template<typename Type>
	BV_INLINE void ConstructArray(Type* pObjs, size_t count)
	{
		// Only call the ctor if needed
		if constexpr (!IsPodV<Type>)
		{
			MemType mem{ pObjs };
			// Construct every element
			for (auto i = 0u; i < count; i++)
			{
				new(mem.pAsVoidPtr) Type();
				mem.pAsCharPtr += sizeof(Type);
			}
		}
	}

	template<typename Type>
	BV_INLINE void DestructArray(Type* pObjs, size_t count)
	{
		// Only call the dtor if needed
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			if (count)
			{
				// Call the dtors in reverse order
				for (auto i = count - 1; i > 0; i--)
				{
					pObjs[i].~Type();
				}
				pObjs[0].~Type();
			}
		}
	}

	template<typename Type>
	void* New()
	{
		MemType mem{ BvMemory::Allocate(sizeof(Type), alignof(Type), 0)};

		return mem.pAsVoidPtr;
	}

	template<typename Type>
	void Delete(Type* pObj)
	{
		[[unlikely]]
		if (!pObj)
		{
			return;
		}

		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			pObj->~Type();
		}

		BvMemory::Free(pObj);
	}

	template<typename Type>
	Type* NewArray(size_t count)
	{
		MemType mem{ BvMemory::Allocate(count * sizeof(Type), alignof(Type), sizeof(size_t)) };

		// Store the count right before the first element
		*mem.pAsSizeTPtr++ = count;

		// Save the pointer to the first element
		Type* pMem = reinterpret_cast<Type*>(mem.pAsVoidPtr);

		ConstructArray(pMem, count);

		// Return the pointer to the first element
		return pMem;
	}

	template<typename Type>
	void DeleteArray(Type* pObjs)
	{
		[[unlikely]]
		if (!pObjs)
		{
			return;
		}

		MemType mem{ (void*)pObjs };

		mem.pAsSizeTPtr--;

		size_t count = *mem.pAsSizeTPtr;

		DestructArray(pObjs, count);

		BvMemory::Free(mem.pAsVoidPtr);
	}

	BV_INLINE void* Alloc(size_t size, size_t alignment = kDefaultAlignmentSize, size_t offset = 0)
	{
		return BvMemory::Allocate(size, alignment, offset);
	}

	BV_INLINE void Free(void* pObj)
	{
		[[unlikely]]
		if (!pObj)
		{
			return;
		}

		BvMemory::Free(pObj);
	}

	template<typename Type, class Allocator>
	void* New(Allocator& allocator, const std::source_location& sourceInfo = std::source_location::current())
	{
		MemType mem{ allocator.Allocate(sizeof(Type), alignof(Type), 0, sourceInfo) };

		return mem.pAsVoidPtr;
	}

	template<typename Type, class Allocator>
	void Delete(Allocator& allocator, Type* pObj, const std::source_location& sourceInfo = std::source_location::current())
	{
		[[unlikely]]
		if (!pObj)
		{
			return;
		}

		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			pObj->~Type();
		}

		allocator.Free(pObj, sourceInfo);
	}

	template<typename Type, class Allocator>
	Type* NewArray(Allocator& allocator, size_t count, const std::source_location& sourceInfo = std::source_location::current())
	{
		MemType mem{ allocator.Allocate(count * sizeof(Type), alignof(Type), sizeof(size_t), sourceInfo) };

		// Store the count right before the first element
		*mem.pAsSizeTPtr++ = count;

		// Save the pointer to the first element
		Type* pMem = reinterpret_cast<Type*>(mem.pAsVoidPtr);

		ConstructArray(pMem, count);

		// Return the pointer to the first element
		return pMem;
	}

	template<typename Type, class Allocator>
	void DeleteArray(Allocator& allocator, Type* pObjs, const std::source_location& sourceInfo = std::source_location::current())
	{
		[[unlikely]]
		if (!pObjs)
		{
			return;
		}

		MemType mem{ (void*)pObjs };

		mem.pAsSizeTPtr--;

		size_t count = *mem.pAsSizeTPtr;

		DestructArray(pObjs, count);

		allocator.Free(mem.pAsVoidPtr, sourceInfo);
	}

	template<class Allocator>
	void* Alloc(Allocator& allocator, size_t size, size_t alignment = kDefaultAlignmentSize, size_t offset = 0, const std::source_location& sourceInfo = std::source_location::current())
	{
		return allocator.Allocate(size, alignment, offset, sourceInfo);
	}

	template<class Allocator>
	void Free(Allocator& allocator, void* pObj, const std::source_location& sourceInfo = std::source_location::current())
	{
		[[unlikely]]
		if (!pObj)
		{
			return;
		}

		allocator.Free(pObj, sourceInfo);
	}
}


// Managed new / delete / alloc / free macros

// Allocate type
#define BV_MNEW(allocator, Type) new(Internal::New<Type>(allocator)) Type
// Allocate type with custom source location
#define BV_MNEW_SI(allocator, sourceInfo, Type) new(Internal::New<Type>(allocator, sourceInfo)) Type
// Free type
#define BV_MDELETE(allocator, pObj) Internal::Delete(allocator, pObj)
// Free type (in-place)
#define BV_MDELETE_IN_PLACE(allocator, pObj) do \
{ \
	if (pObj) \
	{ \
		using ObjType = std::remove_reference_t<decltype(*pObj)>; \
		if constexpr (!std::is_trivially_destructible_v<ObjType>) { pObj->~ObjType(); } \
		Internal::Free(allocator, pObj); \
	} \
} while (0)
// Allocate array of type
#define BV_MNEW_ARRAY(allocator, Type, count) Internal::NewArray<Type>(allocator, count)
// Allocate array of type with custom source location
#define BV_MNEW_ARRAY_SI(allocator, sourceInfo, Type, count) Internal::NewArray<Type>(allocator, count, sourceInfo)
// Free array of type
#define BV_MDELETE_ARRAY(allocator, pObjs) Internal::DeleteArray(allocator, pObjs)
// Free array of type (in-place)
#define BV_MDELETE_ARRAY_IN_PLACE(allocator, pObjs) do \
{ \
	if (pObjs) \
	{ \
		using ObjType = std::remove_reference_t<decltype(*pObjs)>; \
		if constexpr (!IsPodV<ObjType>) \
		{ \
			MemType mem{ (void*)pObjs }; \
			mem.pAsSizeTPtr--; \
			size_t count = *mem.pAsSizeTPtr; \
			for (auto i = count - 1; i > 0; i--) \
			{ \
				pObjs[i].~ObjType(); \
			} \
			pObjs[0].~ObjType(); \
		} \
		Internal::Free(allocator, pObjs); \
	} \
} while (0)
// Allocate block of bytes with custom alignment
#define BV_MALLOC(allocator, size, alignment) Internal::Alloc(allocator, size, alignment)
// Allocate block of bytes with custom alignment and source location
#define BV_MALLOC_SI(allocator, sourceInfo, size, alignment) Internal::Alloc(allocator, size, alignment, 0, sourceInfo)
// Free block of bytes
#define BV_MFREE(allocator, pObj) Internal::Free(allocator, pObj)


// Unmanaged new / delete / alloc / free macros

// Allocate type
#define BV_NEW(Type) new(Internal::New<Type>()) Type
// Free type
#define BV_DELETE(pObj) Internal::Delete(pObj)
// Free type (in-place)
#define BV_DELETE_IN_PLACE(pObj) do \
{ \
	if (pObj) \
	{ \
		using ObjType = std::remove_reference_t<decltype(*pObj)>; \
		if constexpr (!std::is_trivially_destructible_v<ObjType>) { pObj->~ObjType(); } \
		Internal::Free(pObj); \
	} \
} while (0)
// Allocate array of type
#define BV_NEW_ARRAY(Type, count) Internal::NewArray<Type>(count)
// Free array of type
#define BV_DELETE_ARRAY(pObjs) Internal::DeleteArray(pObjs);
// Free array of type (in-place)
#define BV_DELETE_ARRAY_IN_PLACE(pObjs) do \
{ \
	if (pObjs) \
	{ \
		using ObjType = std::remove_reference_t<decltype(*pObjs)>; \
		if constexpr (!IsPodV<ObjType>) \
		{ \
			MemType mem{ (void*)pObjs }; \
			mem.pAsSizeTPtr--; \
			size_t count = *mem.pAsSizeTPtr; \
			for (auto i = count - 1; i > 0; i--) \
			{ \
				pObjs[i].~ObjType(); \
			} \
			pObjs[0].~ObjType(); \
		} \
		Internal::Free(pObjs); \
	} \
} while (0)
// Allocate block of bytes with custom alignment
#define BV_ALLOC(size, alignment) Internal::Alloc(size, alignment)
// Free block of bytes
#define BV_FREE(pObj) Internal::Free(pObj);


// =============================================================
// Memory Arena
// =============================================================


class IBvMemoryArena
{
public:
	virtual ~IBvMemoryArena() {}
	virtual void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0, const std::source_location& sourceInfo = std::source_location::current()) = 0;
	virtual void Free(void* pMem, const std::source_location& sourceInfo = std::source_location::current()) = 0;
	virtual size_t GetAllocationSize(void* pMem) = 0;
	virtual const char* GetName() const = 0;
	virtual void SetName(const char* pName) = 0;
};

#if BV_DEBUG
#define BV_USE_MEMORY_ARENA_NAME
#endif

template<typename AllocatorType, typename LockType, typename BoundsCheckingType, typename MemoryMarkingType, typename MemoryTrackingType, typename MemoryLoggerType>
class BvMemoryArena final : public IBvMemoryArena
{
	static constexpr const char* kDefaultMemoryArenaName = "BDeV Memory Arena";

public:
	BvMemoryArena() {}
	explicit BvMemoryArena([[maybe_unused]] const char* pName)
#ifdef BV_USE_MEMORY_ARENA_NAME
		: m_pName(pName)
#endif
	{}
	~BvMemoryArena() {}

	AllocatorType& GetAllocator()
	{
		return m_Allocator;
	}

	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0, const std::source_location& sourceInfo = std::source_location::current()) override
	{
		m_Lock.Lock();

		size_t newSize = size + BoundsCheckingType::kBackGuardSize;
		MemType mem{ m_Allocator.Allocate(newSize, alignment, alignmentOffset + BoundsCheckingType::kFrontGuardSize) };

		// We start counting from the aligned address, so the real size will be different
		const size_t usedSize = m_Allocator.GetAllocationSize(mem.pAsVoidPtr);

		m_BoundsChecking.GuardFront(mem.pAsVoidPtr);
		m_MemoryMarking.MarkAllocation(mem.pAsCharPtr + BoundsCheckingType::kFrontGuardSize, usedSize -
			(BoundsCheckingType::kFrontGuardSize + BoundsCheckingType::kBackGuardSize));
		m_BoundsChecking.GuardBack(mem.pAsCharPtr + usedSize - BoundsCheckingType::kBackGuardSize);

		m_MemoryTracking.OnAllocation(mem.pAsVoidPtr, size, alignment, sourceInfo);

		m_Lock.Unlock();

		m_MemoryLogger.LogAllocation(GetName(), mem.pAsVoidPtr, size, alignment, sourceInfo);

		return mem.pAsCharPtr + BoundsCheckingType::kFrontGuardSize;
	}

	void Free(void* pMem, const std::source_location& sourceInfo = std::source_location::current()) override
	{
		if (!pMem)
		{
			return;
		}

		MemType mem{ pMem };
		mem.pAsCharPtr -= BoundsCheckingType::kFrontGuardSize;

		m_Lock.Lock();

		// We start counting from the aligned address, so the real size will be different
		const size_t usedSize = m_Allocator.GetAllocationSize(mem.pAsVoidPtr);

		m_BoundsChecking.CheckFrontGuard(mem.pAsVoidPtr);
		m_MemoryMarking.MarkDeallocation(mem.pAsCharPtr + BoundsCheckingType::kFrontGuardSize, usedSize -
			(BoundsCheckingType::kFrontGuardSize + BoundsCheckingType::kBackGuardSize));
		m_BoundsChecking.CheckBackGuard(mem.pAsCharPtr + usedSize - BoundsCheckingType::kBackGuardSize);

		m_MemoryTracking.OnDeallocation(mem.pAsVoidPtr);

		m_Allocator.Free(mem.pAsCharPtr);

		m_Lock.Unlock();

		m_MemoryLogger.LogDeallocation(GetName(), mem.pAsVoidPtr, sourceInfo);
	}

	size_t GetAllocationSize(void* pMem) override
	{
		return m_Allocator.GetAllocationSize(pMem);
	}

	const char* GetName() const override
	{
#ifdef BV_USE_MEMORY_ARENA_NAME
		return m_pName;
#else
		return kDefaultMemoryArenaName;
#endif
	}

	void SetName([[maybe_unused]] const char* pName) override
	{
#ifdef BV_USE_MEMORY_ARENA_NAME
		m_pName = pName;
#endif
	}

private:
#ifdef BV_USE_MEMORY_ARENA_NAME
	const char* m_pName = kDefaultMemoryArenaName;
#endif
	AllocatorType m_Allocator;
	LockType m_Lock;
	BoundsCheckingType m_BoundsChecking;
	MemoryMarkingType m_MemoryMarking;
	MemoryTrackingType m_MemoryTracking;
	MemoryLoggerType m_MemoryLogger;
};


class BvNoMemoryLogger
{
	BV_NOCOPYMOVE(BvNoMemoryLogger);
public:
	BvNoMemoryLogger() {}
	~BvNoMemoryLogger() {}

	BV_INLINE void LogAllocation(const char* pName, void* pMem, size_t size, size_t alignment, const std::source_location& sourceInfo) {}
	BV_INLINE void LogDeallocation(const char* pName, void* pMem, const std::source_location& sourceInfo) {}
};


IBvMemoryArena* GetDefaultMemoryArena();
IBvMemoryArena* SetDefaultMemoryArena(IBvMemoryArena* pArena);

#define BV_DEFAULT_MEMORY_ARENA GetDefaultMemoryArena()


// These are helpers for container classes, so we can broadcast an allocator to another sub-container or object
namespace Internal
{
	template<typename T>
	concept HasGetAllocatorMethod = requires(T obj)
	{
		{ obj.GetAllocator() } -> std::convertible_to<IBvMemoryArena*>;
	};


	template<typename T>
	concept HasSetAllocatorMethod = requires(T obj, IBvMemoryArena* pArena)
	{
		{ obj.SetAllocator(pArena) } -> std::convertible_to<void>;
	};

	template<typename T>
	constexpr bool kCanPropagateAllocatorV = HasGetAllocatorMethod<T> && HasSetAllocatorMethod<T>;

	template<typename T>
	BV_INLINE void PropagateAllocator(T& obj, IBvMemoryArena* pArena)
	{
		if constexpr (kCanPropagateAllocatorV<T>)
		{
			auto pDefaultArena = BV_DEFAULT_MEMORY_ARENA;
			if (pArena != pDefaultArena && obj.GetAllocator() == pDefaultArena)
			{
				obj.SetAllocator(pArena);
			}
		}
	}

	template<typename... Args>
	BV_INLINE void PropagateAllocatorToAll(IBvMemoryArena* pArena, Args&... objs)
	{
		(PropagateAllocator(objs, pArena), ...);
	}
}