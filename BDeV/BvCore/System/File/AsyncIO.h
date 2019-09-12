#pragma once


#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/System/File/BvFileDefs.h"


// void IOCallback(const BvFileResultT result, const u32 bytesTransfered, void * const pData)
typedef void(*pFnAsyncIOCallback)(const BvFileResultT, const u32, void * const);


static constexpr u32 kMaxAsyncIORequests = 16;


class BvFile;


class AsyncIORequest
{
public:
	friend class FileSystem;
	friend class BvFile;
	friend class AsyncIOResult;

	AsyncIORequest(const AsyncIORequest &) = default;
	AsyncIORequest & operator =(const AsyncIORequest &) = default;
	AsyncIORequest(AsyncIORequest && rhs) = default;
	AsyncIORequest & operator =(AsyncIORequest && rhs) = default;
	~AsyncIORequest() {}

private:
	AsyncIORequest()
		: m_Signal(BvEvent(true)) {}

private:
	enum class AsyncOpType
	{
		kRead,
		kWrite
	};

	BvEvent m_Signal;
	BvAtomic m_InUse;
	BvFile * m_pFile = nullptr;
	void * m_pBuffer = nullptr;
	pFnAsyncIOCallback m_pCallback = nullptr;
	void * m_pData = nullptr;
	u32 m_BufferSize = 0;
	u32 m_BytesTransfered = 0;
	BvFileResultT m_Result = EnumVal(BvFileResult::kOk);
	AsyncOpType m_OpType = AsyncOpType::kRead;
};


class AsyncIOResult
{
public:
	friend class FileSystem;
	friend class BvFile;

	AsyncIOResult(const AsyncIOResult & rhs);
	AsyncIOResult & operator =(const AsyncIOResult & rhs);
	AsyncIOResult(AsyncIOResult && rhs);
	AsyncIOResult & operator =(AsyncIOResult && rhs);
	~AsyncIOResult();

	BvFileResultT GetResult(const bool waitForIt = true);
	BV_INLINE u32 GetBytesTransfered() const { return m_BytesTransfered; }

private:
	AsyncIOResult() {}

private:
	AsyncIORequest * m_pIORequest = nullptr;
	u32 m_BytesTransfered = 0;
	BvFileResultT m_Result = EnumVal(BvFileResult::kOk);
};