#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/System/Debug/BvDebug.h"


union MemType
{
	void* pAsVoidPtr;
	char* pAsCharPtr;
	size_t* pAsSizeTPtr;
	u32* pAsUIntPtr;
	size_t asSizeT;
};


void* BvAlignMemory(void* pAddress, size_t alignment);
void* BvMAlloc(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
void BvFree(void* pAddress);


namespace Internal
{
	template<typename Type, class Allocator>
	void Delete(Type* pObj, Allocator& allocator, const char* const pFunction, const char* const pFile, const u32 line)
	{
		// Only call the dtor if needed
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			pObj->~Type();
		}

		BvSourceInfo sourceInfo{ pFunction, pFile, line };
		allocator.Free(pObj, sourceInfo);
	}

	template<typename Type, class Allocator>
	Type* NewArray(size_t count, Allocator& allocator, const char* const pFunction, const char* const pFile, const u32 line)
	{
		BvSourceInfo sourceInfo{ pFunction, pFile, line };
		MemType mem{ allocator.Allocate(count * sizeof(Type), alignof(Type), sizeof(u32), sourceInfo) };

		// Store the count right before the first element
		*mem.pAsUIntPtr++ = count;

		// Save the pointer to the first element
		Type* pMem = reinterpret_cast<Type*>(mem.pAsVoidPtr);

		// Only call the ctor if needed
		if constexpr (!std::is_trivially_constructible_v<Type>)
		{
			// Construct every element
			for (auto i = 0u; i < count; i++)
			{
				new(mem.pAsVoidPtr) Type();
				mem.pAsCharPtr += sizeof(Type);
			}
		}

		// Return the pointer to the first element
		return pMem;
	}

	template<typename Type, class Allocator>
	void DeleteArray(Type* pObjs, Allocator& allocator, const char* const pFunction, const char* const pFile, const u32 line)
	{
		MemType mem{ pObjs };
		mem.pAsUIntPtr--;

		// Only call the dtor if needed
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			u32 count = *mem.pAsUIntPtr;
			// Call the dtors in reverse order
			u32 i = count - 1;
			for (; i > 0u; i--)
			{
				pObjs[i]->~Type();
			}
			pObjs[i]->~Type();
		}

		BvSourceInfo sourceInfo{ pFunction, pFile, line };
		allocator.Free(mem.pAsVoidPtr, sourceInfo);
	}
}


class IBvMemoryAllocator
{
public:
	virtual ~IBvMemoryAllocator() {}
	virtual void* Allocate(size_t size, size_t alignment, size_t alignmentOffset, const BvSourceInfo& sourceInfo) = 0;
	virtual void Free(void* pMem, const BvSourceInfo& sourceInfo) = 0;
};


template<typename AllocatorType, typename LockType, typename BoundsCheckingType,
	typename MemoryMarkingType, typename MemoryTrackingType>
class BvMemoryAllocator final : public IBvMemoryAllocator
{
public:
	BvMemoryAllocator() {}
	BvMemoryAllocator(void* pStart, void* pEnd)
		: m_Allocator(pStart, pEnd) {}
	~BvMemoryAllocator() {}

	void* Allocate(size_t size, size_t alignment, size_t alignmentOffset, const BvSourceInfo& sourceInfo) override
	{
		m_Lock.Lock();

		size += BoundsCheckingType::kBackGuardSize;
		MemType mem{ m_Allocator.Allocate(size, alignment, alignmentOffset + BoundsCheckingType::kFrontGuardSize) };

		// We start counting from the aligned address, so the real size will be different
		const size_t usedSize = m_Allocator.GetAllocationSize(mem.pAsVoidPtr);

		m_BoundsChecking.GuardFront(mem.pAsVoidPtr);
		m_MemoryMarking.MarkAllocation(mem.pAsCharPtr + BoundsCheckingType::kFrontGuardSize, usedSize -
			(BoundsCheckingType::kFrontGuardSize + BoundsCheckingType::kBackGuardSize));
		m_BoundsChecking.GuardBack(mem.pAsCharPtr + usedSize - BoundsCheckingType::kBackGuardSize);

		m_MemoryTracking.OnAllocation(mem.pAsVoidPtr, size, alignment, sourceInfo);

		mem.pAsCharPtr += BoundsCheckingType::kFrontGuardSize;

		m_Lock.Unlock();

		return mem.pAsVoidPtr;
	}

	void Free(void* pMem, const BvSourceInfo& sourceInfo) override
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

		m_Allocator.Free(pMem);

		m_Lock.Unlock();
	}

private:
	AllocatorType m_Allocator;
	LockType m_Lock;
	BoundsCheckingType m_BoundsChecking;
	MemoryMarkingType m_MemoryMarking;
	MemoryTrackingType m_MemoryTracking;
};


void SetDefaultAllocator(IBvMemoryAllocator* defaultAllocator);
IBvMemoryAllocator* GetDefaultAllocator();


// Replaceable allocation functions
void* operator new  (std::size_t count);
void* operator new[](std::size_t count);
void* operator new  (std::size_t count, std::align_val_t al);
void* operator new[](std::size_t count, std::align_val_t al);

// Replaceable non-throwing allocation functions
void* operator new  (std::size_t count, const std::nothrow_t& tag) noexcept;
void* operator new[](std::size_t count, const std::nothrow_t& tag) noexcept;
void* operator new  (std::size_t count, std::align_val_t al, const std::nothrow_t&) noexcept;
void* operator new[](std::size_t count,	std::align_val_t al, const std::nothrow_t&) noexcept;

void operator delete  (void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete  (void* ptr, std::align_val_t al) noexcept;
void operator delete[](void* ptr, std::align_val_t al) noexcept;

#define BvNew(Type) BvNewA(Type, GetDefaultAllocator())
#define BvNewArray(Type, count) BvNewArrayA(Type, count, GetDefaultAllocator())
#define BvDelete(pObj) BvDeleteA(pObj, GetDefaultAllocator())
#define BvDeleteArray(pObjs) BvDeleteArrayA(pObjs, GetDefaultAllocator())


#define BvNewA(Type, allocator) new ((allocator).Allocate(sizeof(Type), alignof(Type), 0, { BV_FUNCTION, BV_FILE, BV_LINE })) Type
#define BvNewArrayA(Type, count, allocator) Internal::NewArray<Type>(count, allocator, BV_FUNCTION, BV_FILE, BV_LINE)
#define BvDeleteA(pObj, allocator) Internal::Delete(pObj, allocator, BV_FUNCTION, BV_FILE, BV_LINE)
#define BvDeleteArrayA(pObjs, allocator) Internal::DeleteArray(pObjs, allocator, BV_FUNCTION, BV_FILE, BV_LINE)