#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/System/Debug/BvDebug.h"


union MemType
{
	void* pAsVoidPtr;
	char* pAsCharPtr;
	size_t* pAsSizeTPtr;
	size_t asSizeT;
};


BV_INLINE void* AlignMemory(void* pAddress, size_t alignment)
{
	BvAssert(pAddress != nullptr, "Address has to be valid");
	BvAssert((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

	const size_t mask = alignment - 1;
	return reinterpret_cast<void*>((reinterpret_cast<size_t>(pAddress) + mask) & (~mask));
}


namespace Internal
{
	template<typename Type, class Allocator>
	void Delete(Type* pObj, Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		// Only call the dtor if needed
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			pObj->~Type();
		}

		allocator.Free(pObj, sourceInfo);
	}

	template<typename Type, class Allocator>
	Type* NewArray(size_t count, Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ allocator.Allocate(count * sizeof(Type), alignof(Type), sizeof(size_t), sourceInfo) };

		// Store the count right before the first element
		*mem.pAsSizeTPtr++ = count;

		// Save the pointer to the first element
		Type* pMem = reinterpret_cast<Type*>(mem.pAsVoidPtr);

		// Only call the ctor if needed
		if constexpr (!IsPodV<Type>)
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
	void DeleteArray(Type* pObjs, Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ pObjs };
		mem.pAsSizeTPtr--;

		// Only call the dtor if needed
		if constexpr (!IsPodV<Type>)
		{
			u32 count = *mem.pAsSizeTPtr;
			// Call the dtors in reverse order
			u32 i = count - 1;
			for (; i > 0u; i--)
			{
				pObjs[i]->~Type();
			}
			pObjs[i]->~Type();
		}

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

		return mem.pAsCharPtr + BoundsCheckingType::kFrontGuardSize;
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


IBvMemoryAllocator* GetDefaultAllocator();


void* operator new  (std::size_t count);
void* operator new[](std::size_t count);
void* operator new  (std::size_t count, std::align_val_t al);
void* operator new[](std::size_t count, std::align_val_t al);

void operator delete  (void* ptr);
void operator delete[](void* ptr);
void operator delete  (void* ptr, std::align_val_t al);
void operator delete[](void* ptr, std::align_val_t al);


// Custom
void* operator new  (std::size_t count, const BvSourceInfo& sourceInfo);
void* operator new[](std::size_t count, const BvSourceInfo& sourceInfo);
void* operator new  (std::size_t count, std::align_val_t al, const BvSourceInfo& sourceInfo);
void* operator new[](std::size_t count, std::align_val_t al, const BvSourceInfo& sourceInfo);

void operator delete  (void* ptr, const BvSourceInfo& sourceInfo);
void operator delete[](void* ptr, const BvSourceInfo& sourceInfo);
void operator delete  (void* ptr, std::align_val_t al, const BvSourceInfo& sourceInfo);
void operator delete[](void* ptr, std::align_val_t al, const BvSourceInfo& sourceInfo);

#define BV_NEW new(BV_SOURCE_INFO)
#define BV_DELETE PrintF("Default deallocation at %s() (%s), line %lu\n", BV_FUNCTION, BV_FILE, BV_LINE); delete


#define BV_ANEW(Type, allocator) new ((allocator).Allocate(sizeof(Type), alignof(Type), 0, { BV_FUNCTION, BV_FILE, BV_LINE })) Type
#define BV_ANEW_ARRAY(Type, count, allocator) Internal::NewArray<Type>(count, allocator, { BV_FUNCTION, BV_FILE, BV_LINE })
#define BV_ADELETE(pObj, allocator) Internal::Delete(pObj, allocator, { BV_FUNCTION, BV_FILE, BV_LINE })
#define BV_ADELETE_ARRAY(pObjs, allocator) Internal::DeleteArray(pObjs, allocator, { BV_FUNCTION, BV_FILE, BV_LINE })