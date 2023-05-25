#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


template<size_t N>
class BvStackMemory
{
	BV_NOCOPYMOVE(BvStackMemory);
	
public:
	BvStackMemory() {}
	~BvStackMemory() {}

	operator char*() { return m_Data; }
	size_t GetSize() const { return N; }

private:
	char m_Data[N];
};


class BvVirtualMemory
{
public:
	static void* Reserve(size_t size);
	static void Commit(void* pAddress, size_t size);
	static void* ReserveAndCommit(size_t size);
	static void Decommit(void* pAddress, size_t size);
	static void Release(void* pAddress);
};