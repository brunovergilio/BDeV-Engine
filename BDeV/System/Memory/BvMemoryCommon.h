#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/System/Debug/BvDebug.h"
#include <atomic>


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


namespace BvMemory
{
	BV_INLINE void* AlignMemory(void* pAddress, size_t alignment)
	{
		BvAssert(pAddress != nullptr, "Address has to be valid");
		BvAssert((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

		const size_t mask = alignment - 1;
		return reinterpret_cast<void*>((reinterpret_cast<size_t>(pAddress) + mask) & (~mask));
	}

	BV_INLINE void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0)
	{
		BvAssert(size > 0, "Size can't be 0");
		BvAssert((alignment & (alignment - 1)) == 0, "Alignment has to be a power of 2");

		// Allocate memory using malloc - the total size will be the requested size, plus
		// the alignment and alignment offset, and we also add another kPointerSize bytes
		// in order to store the pointer back to the original address when freeing it
		MemType unalignedMem{ malloc(size + alignment + alignmentOffset + kPointerSize) };

		// We align the memory with the added offset and kPointerSize
		MemType alignedMemWithOffset{ AlignMemory(unalignedMem.pAsCharPtr + alignmentOffset + kPointerSize, alignment) };

		// We move back the offset bytes
		alignedMemWithOffset.pAsCharPtr -= alignmentOffset;
		// And then we set the originally allocated memory address
		alignedMemWithOffset.pAsSizeTPtr[-1] = unalignedMem.asSizeT;

		// return the memory containing the offset bytes
		return alignedMemWithOffset.pAsVoidPtr;
	}


	BV_INLINE void Free(void* pAddress)
	{
		if (pAddress)
		{
			// Note: if the address was allocated with an alignment offset greater than 0,
			// that offset has to be subtracted from the memory address before it is
			// passed to this function

			// We take the returned address and move back size_t* bytes,
			// since that's where we stored our original pointer
			MemType alignedMemWithOffset{ pAddress };

			// We free the address that was stored as a size_t value,
			// so we need to cast it back to void*
			free(reinterpret_cast<void*>(alignedMemWithOffset.pAsSizeTPtr[-1]));
		}
	}

	template<typename Type>
	void* New(size_t alignment, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ Allocate(sizeof(Type), alignment >= alignof(Type) ? alignment : alignof(Type), 0) };

		BvConsole::PrintF(BvColorI::BrightBlue, "New called - 0x%p (%llu bytes)\nFunction: %s, File: %s (%d)\n\n", mem.pAsVoidPtr, sizeof(Type),
			sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		return mem.pAsVoidPtr;
	}

	template<typename Type>
	void Delete(Type* pObj, const BvSourceInfo& sourceInfo)
	{
		// Only call the dtor if needed
		if constexpr (!std::is_trivially_destructible_v<Type>)
		{
			pObj->~Type();
		}

		Free(pObj);

		BvConsole::PrintF(BvColorI::BrightCyan, "Delete called - 0x%p\nFunction: %s, File: %s (%d)\n\n", pObj, sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	}

	template<typename Type>
	Type* NewArray(size_t count, size_t alignment, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ Allocate(count * sizeof(Type), alignment >= alignof(Type) ? alignment : alignof(Type), sizeof(size_t)) };

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

		BvConsole::PrintF(BvColorI::BrightBlue, "New[] called for %llu elements - 0x%p (%llu bytes)\nFunction: %s, File: %s (%d)\n\n", count, pMem, sizeof(Type),
			sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		// Return the pointer to the first element
		return pMem;
	}

	template<typename Type>
	void DeleteArray(Type* pObjs, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ pObjs };

		BvConsole::PrintF(BvColorI::BrightCyan, "Delete[] called - 0x%p\nFunction: %s, File: %s (%d)\n\n", pObjs, sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		mem.pAsSizeTPtr--;

		// Only call the dtor if needed
		if constexpr (!IsPodV<Type>)
		{
			size_t count = *mem.pAsSizeTPtr;
			// Call the dtors in reverse order
			for (auto i = count - 1; i > 0; i--)
			{
				pObjs[i].~Type();
			}
			pObjs[0].~Type();
		}

		Free(mem.pAsVoidPtr);
	}

	template<typename Type, class Allocator>
	void* New(size_t alignment, Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ allocator.Allocate(sizeof(Type), alignment >= alignof(Type) ? alignment : alignof(Type), 0, sourceInfo) };

		return mem.pAsVoidPtr;
	}

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
	Type* NewArray(size_t count, size_t alignment, Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ allocator.Allocate(count * sizeof(Type), alignment >= alignof(Type) ? alignment : alignof(Type), sizeof(size_t), sourceInfo) };

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
			size_t count = *mem.pAsSizeTPtr;
			// Call the dtors in reverse order
			for (auto i = count - 1; i > 0; i--)
			{
				pObjs[i].~Type();
			}
			pObjs[0].~Type();
		}

		allocator.Free(mem.pAsVoidPtr, sourceInfo);
	}
}


// Managed new / delete macros
// Allocate type w/ base alignment
#define BvMNew(allocator, Type, ...) new(BvMemory::New<Type>(alignof(Type), allocator, BV_SOURCE_INFO)) Type(__VA_OPT__(__VA_ARGS__))
// Allocate type w/ custom alignment (>= base alignment)
#define BvMNewA(allocator, Type, alignment, ...) new(BvMemory::New<Type>(alignment, allocator, BV_SOURCE_INFO)) Type(__VA_OPT__(__VA_ARGS__))
// Allocate array of type w/ base alignment
#define BvMNewN(allocator, Type, count) BvMemory::NewArray<Type>(count, alignof(Type), allocator, BV_SOURCE_INFO)
// Allocate array of type w/ custom alignment (>= base alignment)
#define BvMNewNA(allocator, Type, count, alignment) BvMemory::NewArray<Type>(count, alignment, allocator, BV_SOURCE_INFO)
// Free type
#define BvMDelete(allocator, pObj) BvMemory::Delete(pObj, allocator, BV_SOURCE_INFO)
// Free array of type
#define BvMDeleteN(allocator, pObjs) BvMemory::DeleteArray(pObjs, allocator, BV_SOURCE_INFO)


// Unmanaged new / delete macros
// Allocate type w/ base alignment
#define BvNew(Type, ...) new(BvMemory::New<Type>(alignof(Type), BV_SOURCE_INFO)) Type(__VA_OPT__(__VA_ARGS__))
// Allocate type w/ custom alignment (>= base alignment)
#define BvNewA(Type, alignment, ...) new(BvMemory::New<Type>(alignment, BV_SOURCE_INFO)) Type(__VA_OPT__(__VA_ARGS__))
// Allocate array of type w/ base alignment
#define BvNewN(Type, count) BvMemory::NewArray<Type>(count, alignof(Type), BV_SOURCE_INFO)
// Allocate array of type w/ custom alignment (>= base alignment)
#define BvNewNA(Type, count, alignment) BvMemory::NewArray<Type>(count, alignment, BV_SOURCE_INFO)
// Free type
#define BvDelete(pObj) BvMemory::Delete(pObj, BV_SOURCE_INFO)
// Free array of type
#define BvDeleteN(pObjs) BvMemory::DeleteArray(pObjs, BV_SOURCE_INFO)