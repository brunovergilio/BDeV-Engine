#pragma once


#include "BDeV/Core/BvCore.h"


namespace Internal
{
	class BvStackAreaImpl final
	{
	public:
		BvStackAreaImpl(void* pMem, size_t size);
		~BvStackAreaImpl();

		BV_INLINE char* GetStart() const { return m_pStart; }
		BV_INLINE char* GetEnd() const { return m_pEnd; }

	private:
		char* m_pStart;
		char* m_pEnd;
	};
}


template<size_t Size>
class BvFixedStackArea final
{
public:
	BvFixedStackArea() {}
	~BvFixedStackArea() {}

	BV_INLINE char* GetStart() const { return m_Buffer; }
	BV_INLINE char* GetEnd() const { return m_Buffer + Size; }

private:
	char m_Buffer[Size]{};
};


class BvHeapArea final
{
public:
	BvHeapArea(size_t size);
	~BvHeapArea();

	BV_INLINE char* GetStart() const { return m_pStart; }
	BV_INLINE char* GetEnd() const { return m_pEnd; }

private:
	char* m_pStart;
	char* m_pEnd;
};


class BvPageArea final
{
public:
	BvPageArea(u32 maxPageCount, u32 initialCommitPageCount = 1);
	~BvPageArea();

	char* CommitPages(u32 pageCount);

	BV_INLINE char* GetStart() const { return m_pStart; }
	BV_INLINE char* GetEnd() const { return m_pCurr; }

private:
	char* m_pStart;
	char* m_pCurr;
	char* m_pEnd;
};