#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


void* BvVMReserve(size_t size);
void BvVMCommit(void* pAddress, size_t size);
void* BvVMReserveAndCommit(size_t size);
void BvVMDecommit(void* pAddress, size_t size);
void BvVMRelease(void* pAddress);


//class BvVirtualMemory
//{
//	BV_NOCOPY(BvVirtualMemory);
//public:
//	BvVirtualMemory();
//	BvVirtualMemory(size_t maxSize, size_t initialSize, size_t growSize);
//	BvVirtualMemory(BvVirtualMemory&& rhs);
//	BvVirtualMemory& operator=(BvVirtualMemory&& rhs);
//	~BvVirtualMemory();
//
//	BV_INLINE void* GetStart() const { return m_pStart; }
//
//private:
//	void* m_pStart = nullptr;
//	size_t m_MaxSize = 0;
//	size_t m_CurrentSize = 0;
//	size_t m_PageSize = 0;
//};