#include "AsyncIO.h"


AsyncIOResult::AsyncIOResult(const AsyncIOResult & rhs)
	: m_BytesTransfered(rhs.m_BytesTransfered), m_Result(rhs.m_Result)
{
	if (m_pIORequest)
	{
		m_pIORequest->m_InUse.Decrement();
	}
	m_pIORequest = rhs.m_pIORequest;
	if (m_pIORequest)
	{
		m_pIORequest->m_InUse.Increment();
	}
}


AsyncIOResult & AsyncIOResult::operator=(const AsyncIOResult & rhs)
{
	if (this != &rhs)
	{
		m_BytesTransfered = rhs.m_BytesTransfered;
		m_Result = rhs.m_Result;

		if (m_pIORequest)
		{
			m_pIORequest->m_InUse.Decrement();
		}
		m_pIORequest = rhs.m_pIORequest;
		if (m_pIORequest)
		{
			m_pIORequest->m_InUse.Increment();
		}
	}

	return *this;
}


AsyncIOResult::AsyncIOResult(AsyncIOResult && rhs)
{
	*this = std::move(rhs);
}


AsyncIOResult & AsyncIOResult::operator=(AsyncIOResult && rhs)
{
	if (this != &rhs)
	{
		if (m_pIORequest)
		{
			m_pIORequest->m_InUse.Decrement();
		}
		m_pIORequest = rhs.m_pIORequest;
		m_BytesTransfered = rhs.m_BytesTransfered;
		m_Result = rhs.m_Result;

		rhs.m_pIORequest = nullptr;
		rhs.m_BytesTransfered = 0;
		rhs.m_Result = EnumVal(BvFileResult::kOk);
	}

	return *this;
}


AsyncIOResult::~AsyncIOResult()
{
	if (m_pIORequest)
	{
		m_pIORequest->m_InUse.Decrement();
	}
}


BvFileResultT AsyncIOResult::GetResult(const bool waitForIt)
{
	if (m_pIORequest)
	{
		if (m_pIORequest->m_Signal.Wait(waitForIt ? UINT32_MAX : 0))
		{
			m_BytesTransfered = m_pIORequest->m_BytesTransfered;
			m_Result = m_pIORequest->m_Result;
		}
	}

	return m_Result;
}