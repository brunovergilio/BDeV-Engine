#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


namespace BvMemory
{
	void* Allocate(size_t size, size_t alignment = kDefaultAlignmentSize, size_t alignmentOffset = 0);
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


namespace Internal
{
	template<typename Type>
	void* New(const BvSourceInfo& sourceInfo)
	{
		MemType mem{ BvMemory::Allocate(sizeof(Type), alignof(Type), 0)};

		BvConsole::Print(BvColorI::BrightBlue, "New called - 0x%p (%llu bytes)\nFunction: %s, File: %s (%d)\n\n", mem.pAsVoidPtr, sizeof(Type),
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

		BvMemory::Free(pObj);

		BvConsole::Print(BvColorI::BrightCyan, "Delete called - 0x%p\nFunction: %s, File: %s (%d)\n\n", pObj, sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	}

	template<typename Type>
	Type* NewArray(size_t count, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ BvMemory::Allocate(count * sizeof(Type), alignof(Type), sizeof(size_t)) };

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

		BvConsole::Print(BvColorI::BrightBlue, "New[] called for %llu elements - 0x%p (%llu bytes)\nFunction: %s, File: %s (%d)\n\n", count, pMem, sizeof(Type),
			sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		// Return the pointer to the first element
		return pMem;
	}

	template<typename Type>
	void DeleteArray(Type* pObjs, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ (void*)pObjs };

		mem.pAsSizeTPtr--;

		size_t count = *mem.pAsSizeTPtr;

		BvConsole::Print(BvColorI::BrightCyan, "Delete[] called for %llu elements - 0x%p\nFunction: %s, File: %s (%d)\n\n", count, pObjs, sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		// Only call the dtor if needed
		if constexpr (!IsPodV<Type>)
		{
			// Call the dtors in reverse order
			for (auto i = count - 1; i > 0; i--)
			{
				pObjs[i].~Type();
			}
			pObjs[0].~Type();
		}

		BvMemory::Free(mem.pAsVoidPtr);
	}

	BV_INLINE void* Alloc(size_t size, size_t alignment, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ BvMemory::Allocate(size, alignment, 0) };

		BvConsole::Print(BvColorI::BrightBlue, "Alloc called - 0x%p (%llu bytes)\nFunction: %s, File: %s (%d)\n\n", mem.pAsVoidPtr, size,
			sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);

		return mem.pAsVoidPtr;
	}

	BV_INLINE void Free(void* pObj, const BvSourceInfo& sourceInfo)
	{
		BvMemory::Free(pObj);

		BvConsole::Print(BvColorI::BrightCyan, "Free called - 0x%p\nFunction: %s, File: %s (%d)\n\n", pObj, sourceInfo.m_pFunction, sourceInfo.m_pFile, sourceInfo.m_Line);
	}

	template<typename Type, class Allocator>
	void* New(Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ allocator.Allocate(sizeof(Type), alignof(Type), 0, sourceInfo) };

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

	template<class Allocator>
	void* Alloc(size_t size, size_t alignment, Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		MemType mem{ allocator.Allocate(size, alignment, 0, sourceInfo) };

		return mem.pAsVoidPtr;
	}

	template<class Allocator>
	void Free(void* pObj, Allocator& allocator, const BvSourceInfo& sourceInfo)
	{
		allocator.Free(pObj, sourceInfo);
	}
}


// Managed new / delete / alloc / free macros

// Allocate type
#define BV_MNEW(allocator, Type) new(Internal::New<Type>(allocator, BV_SOURCE_INFO)) Type
#define BV_MNEW_SI(allocator, sourceInfo, Type) new(Internal::New<Type>(allocator, sourceInfo)) Type
// Free type
#define BV_MDELETE(allocator, pObj) Internal::Delete(pObj, allocator, BV_SOURCE_INFO)
// Allocate array of type
#define BV_MNEW_ARRAY(allocator, Type, count) Internal::NewArray<Type>(count, allocator, BV_SOURCE_INFO)
#define BV_MNEW_ARRAY_SI(allocator, sourceInfo, Type, count) Internal::NewArray<Type>(count, allocator, sourceInfo)
// Free array of type
#define BV_MDELETE_ARRAY(allocator, pObjs) Internal::DeleteArray(pObjs, allocator, BV_SOURCE_INFO)
// Allocate block of bytes with custom alignment
#define BV_MALLOC(allocator, size, alignment) Internal::Alloc(size, alignment, allocator, BV_SOURCE_INFO)
#define BV_MALLOC_SI(allocator, sourceInfo, size, alignment) Internal::Alloc(size, alignment, allocator, sourceInfo)
// Free block of bytes
#define BV_MFREE(allocator, pObj) Internal::Free(pObj, allocator, BV_SOURCE_INFO)


// Unmanaged new / delete / alloc / free macros

// Allocate type
#define BV_NEW(Type) new(Internal::New<Type>(BV_SOURCE_INFO)) Type
#define BV_NEW_SI(sourceInfo, Type) new(Internal::New<Type>(sourceInfo)) Type
// Free type
#define BV_DELETE(pObj) Internal::Delete(pObj, BV_SOURCE_INFO)
// Allocate array of type
#define BV_NEW_ARRAY(Type, count) Internal::NewArray<Type>(count, BV_SOURCE_INFO)
#define BV_NEW_ARRAY_SI(sourceInfo, Type, count) Internal::NewArray<Type>(count, sourceInfo)
// Free array of type
#define BV_DELETE_ARRAY(pObjs) Internal::DeleteArray(pObjs, BV_SOURCE_INFO)
// Allocate block of bytes with custom alignment
#define BV_ALLOC(size, alignment) Internal::Alloc(size, alignment, BV_SOURCE_INFO)
#define BV_ALLOC_SI(sourceInfo, size, alignment) Internal::Alloc(size, alignment, sourceInfo)
// Free block of bytes
#define BV_FREE(pObj) Internal::Free(pObj, BV_SOURCE_INFO)