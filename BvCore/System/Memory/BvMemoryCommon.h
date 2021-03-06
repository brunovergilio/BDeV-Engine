#pragma once


#include "BvCore/BvCore.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Utils/BvDebug.h"


union MemType
{
	void* pAsVoidPtr;
	char* pAsCharPtr;
	size_t* pAsSizeTPtr;
	u32* pAsUIntPtr;
	size_t asSizeT;
};


BV_INLINE void* BvAlign(void* pAddress, size_t alignment)
{
	BvAssert(pAddress != nullptr, "Address has to be valid");
	BvAssert((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

	const size_t mask = alignment - 1;
	return reinterpret_cast<void*>((reinterpret_cast<size_t>(pAddress) + mask) & (~mask));
}



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
				new(mem.pAsVoidPtr) Type;
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


class BvMemoryAllocatorBase
{
public:
	virtual ~BvMemoryAllocatorBase() {}
	virtual void* Allocate(size_t size, size_t alignment, size_t extraBytes, const BvSourceInfo& sourceInfo) = 0;
	virtual void Free(void* pMem, const BvSourceInfo& sourceInfo) = 0;
};


template<typename AllocatorType, typename LockType, typename BoundsCheckingType,
	typename MemoryMarkingType, typename MemoryTrackingType>
class BvMemoryAllocator final : public BvMemoryAllocatorBase
{
public:
	BvMemoryAllocator() {}
	BvMemoryAllocator(void* pStart, void* pEnd)
		: m_Allocator(pStart, pEnd) {}
	~BvMemoryAllocator() {}

	void* Allocate(size_t size, size_t alignment, size_t extraBytes, const BvSourceInfo& sourceInfo) override
	{
		m_Lock.Lock();

		const size_t totalSize = size + extraBytes + BoundsCheckingType::kFrontGuardSize + BoundsCheckingType::kBackGuardSize;
		MemType mem{ m_Allocator.Allocate(totalSize, alignment, extraBytes + BoundsCheckingType::kFrontGuardSize) };

		// We start counting from the aligned address, so the real size will be different
		const size_t usedSize = m_Allocator.GetAllocationSize(mem.pAsVoidPtr);

		m_BoundsChecking.GuardFront(mem.pAsVoidPtr);
		m_MemoryMarking.MarkAllocation(mem.pAsCharPtr + BoundsCheckingType::kFrontGuardSize, usedSize -
			(BoundsCheckingType::kFrontGuardSize + BoundsCheckingType::kBackGuardSize));
		m_BoundsChecking.GuardBack(mem.pAsCharPtr + usedSize - BoundsCheckingType::kBackGuardSize);

		m_MemoryTracking.OnAllocation(mem.pAsVoidPtr, totalSize, alignment, sourceInfo);

		mem.pAsCharPtr += BoundsCheckingType::kFrontGuardSize;

		m_Lock.Unlock();

		return mem.pAsVoidPtr;
	}

	void Free(void* pMem, const BvSourceInfo& sourceInfo) override
	{
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


#define BvNew(Type) BvNewT(Type, GetDefaultAllocator())
#define BvNewArray(Type, count) BvNewArrayT(Type, count, GetDefaultAllocator())
#define BvDelete(pObj) BvDeleteT(pObj, GetDefaultAllocator())
#define BvDeleteArray(pObjs) BvDeleteArrayT(pObjs, GetDefaultAllocator())


#define BvNewA(Type, allocator) new ((allocator).Allocate(sizeof(Type), alignof(Type), 0, { BV_FUNCTION, BV_FILE, BV_LINE })) Type
#define BvNewArrayA(Type, count, allocator) Internal::NewArray<Type>(count, allocator, BV_FUNCTION, BV_FILE, BV_LINE)
#define BvDeleteA(pObj, allocator) Internal::Delete(pObj, allocator, BV_FUNCTION, BV_FILE, BV_LINE)
#define BvDeleteArrayA(pObjs, allocator) Internal::DeleteArray(pObjs, allocator)