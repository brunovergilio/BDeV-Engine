#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Memory/BvMemory.h"
#include "BDeV/Core/System/Threading/BvSync.h"


namespace Internal
{
	class BvMemoryFreeList
	{
	public:
		BvMemoryFreeList() {}
		~BvMemoryFreeList() {}

		BV_INLINE void* Get()
		{
			if (!m_pHead)
			{
				return nullptr;
			}

			auto pBlock = m_pHead;
			m_pHead = *((void**)pBlock);
		}

		BV_INLINE void Return(void* pBlock)
		{
			void** pHead = (void**)pBlock;
			*pHead = m_pHead;
			m_pHead = pBlock;
		}

	private:
		void* m_pHead = nullptr;
	};


	class BvMemoryBlockManager
	{
	public:
		BvMemoryBlockManager() {}
		BvMemoryBlockManager(void* pMem, size_t totalSize, size_t blockSize) { AssignAddressSpace(pMem, totalSize, blockSize); }
		~BvMemoryBlockManager() {}

		void AssignAddressSpace(void* pMem, size_t totalSize, size_t blockSize)
		{
			MemType mem{ pMem };

			m_pStart = mem.pAsCharPtr;
			m_pCurr = mem.pAsCharPtr;
			m_pEnd = mem.pAsCharPtr + totalSize;
			m_BlockSize = blockSize;
		}

		void* GetMemoryBlock()
		{
			BvScopedLock lock(m_Mutex);

			void* pMem = m_FreeList.Get();
			if (!pMem && m_pCurr < m_pEnd)
			{
				pMem = m_pCurr;
				m_pCurr += m_BlockSize;
			}

			return pMem;
		}

		void ReturnMemoryBlock(void* pMem)
		{
			BvScopedLock lock(m_Mutex);

			m_FreeList.Return(pMem);
		}

		BV_INLINE size_t GetBlockSize() const { return m_BlockSize; }

	private:
		char* m_pStart = nullptr;
		char* m_pCurr = nullptr;
		char* m_pEnd = nullptr;
		size_t m_BlockSize = 0;

		BvAdaptiveMutex m_Mutex;
		Internal::BvMemoryFreeList m_FreeList;
	};
}


class BvSmallAllocationModule final
{
public:
	static constexpr size_t kMinAllocationSize = 1;
	static constexpr size_t kMaxAllocationSize = 64;
	static constexpr size_t kPageSize = 16_kb;
	static constexpr size_t kVirtualAddressSpaceSize = 512_mb;

	BvSmallAllocationModule();
	~BvSmallAllocationModule();

	void AssignMemory(void* pMem, size_t totalSize);
	void* Allocate(size_t size);
	void Free(void* pPage, size_t size);

private:
	Internal::BvMemoryBlockManager m_BlockManager;
	Internal::BvMemoryFreeList m_FreeLists[kMaxAllocationSize >> 3];
};


class BvMediumAllocationModule final
{
public:
	static constexpr size_t kMinAllocationSize = BvSmallAllocationModule::kMaxAllocationSize + 1;
	static constexpr size_t kMaxAllocationSize = 64_kb - 1;
	static constexpr size_t kPageSize = 2_mb;
	static constexpr size_t kVirtualAddressSpaceSize = 8_gb;

	BvMediumAllocationModule();
	~BvMediumAllocationModule();

	void AssignMemory(void* pMem, size_t totalSize);
	void* Allocate();
	void Free(void* pPage);

private:
	Internal::BvMemoryBlockManager m_BlockManager;
};


class BvLargeAllocationModule final
{
public:
	static constexpr size_t kMinAllocationSize = 64_kb;
	static constexpr size_t kMaxAllocationSize = 32_mb;
	static constexpr size_t kPageSize = 64_kb;
	static constexpr size_t kVirtualAddressSpaceSize = 160_gb;

	BvLargeAllocationModule();
	~BvLargeAllocationModule();

	void AssignMemory(void* pMem, size_t totalSize);
	void* Allocate(size_t size);
	void Free(void* pPage, size_t size);

private:
	static constexpr size_t kNumAllocationSizes = 9;
	Internal::BvMemoryBlockManager m_BlockManagers[kNumAllocationSizes];
};