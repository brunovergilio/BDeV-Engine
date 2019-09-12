#include "BvCore/System/File/BvFile.h"
#include "BvCore/BvDebug.h"
#include "BvCore/System/File/FileSystem.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


BvFile::BvFile(const char * const pFilename, const BvFileAccess mode)
{
	Open(pFilename, mode);
}


BvFile::BvFile(BvFile && rhs)
{
	*this = std::move(rhs);
}


BvFile & BvFile::operator =(BvFile && rhs)
{
	if (this != &rhs)
	{
		m_hFile = rhs.m_hFile;
		rhs.m_hFile = INVALID_HANDLE_VALUE;
	}

	return *this;
}


BvFile::~BvFile()
{
	Close();
}


BvFileResult BvFile::Open(const char * const pFilename, const BvFileAccess mode)
{
	*this = FileSys.OpenFile(pFilename, mode);

	return m_hFile != INVALID_HANDLE_VALUE ? BvFileResult::kOk : BvFileResult::kError;
}


void BvFile::Close()
{
	FileSys.CloseFile(*this);
}


BvFileResultT BvFile::Read(void * const pBuffer, const u32 bufferSize, u32 * const pBytesRead)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	BvAssert(pBuffer != nullptr);
	BvAssert(bufferSize > 0);

	u32 bytes = 0;
	if (ReadFile(m_hFile, pBuffer, bufferSize, reinterpret_cast<LPDWORD>(&bytes), nullptr))
	{
		if (pBytesRead)
		{
			*pBytesRead = bytes;
		}

		BvFileResultT result = BvFileResult::kOk | BvFileResult::kDoneSync;
		return bytes > 0 ? result : result | BvFileResult::kEOF;
	}

	return EnumVal(BvFileResult::kError);
}


BvFileResultT BvFile::Write(const void * const pBuffer, const u32 bufferSize, u32 * const pBytesWritten)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	BvAssert(pBuffer != nullptr);
	BvAssert(bufferSize > 0);

	u32 bytes = 0;
	if (WriteFile(m_hFile, pBuffer, bufferSize, reinterpret_cast<LPDWORD>(&bytes), nullptr))
	{
		if (pBytesWritten)
		{
			*pBytesWritten = bytes;
		}

		BvFileResultT result = BvFileResult::kOk | BvFileResult::kDoneSync;
		return bytes > 0 ? result : result | BvFileResult::kEOF;
	}

	return EnumVal(BvFileResult::kError);
}


AsyncIOResult BvFile::ReadAsync(void * const pBuffer, const u32 bufferSize, pFnAsyncIOCallback pCallback, void * const pData)
{
	AsyncIOResult result;

	AsyncIORequest * pIORequest = FileSys.QueueAsyncRequest(this, pBuffer, bufferSize, AsyncIORequest::AsyncOpType::kRead, pCallback, pData);
	if (pIORequest)
	{
		result.m_pIORequest = pIORequest;
		result.m_Result = EnumVal(BvFileResult::kInProgress);

		result.m_pIORequest->m_InUse.Increment();
	}
	else
	{
		result.m_Result = Read(pBuffer, bufferSize, &result.m_BytesTransfered);
	}

	return result;
}


AsyncIOResult BvFile::WriteAsync(void * const pBuffer, const u32 bufferSize, pFnAsyncIOCallback pCallback, void * const pData)
{
	AsyncIOResult result;

	AsyncIORequest * pIORequest = FileSys.QueueAsyncRequest(this, pBuffer, bufferSize, AsyncIORequest::AsyncOpType::kWrite, pCallback, pData);
	if (pIORequest)
	{
		result.m_pIORequest = pIORequest;
		result.m_Result = EnumVal(BvFileResult::kInProgress);

		result.m_pIORequest->m_InUse.Increment();
	}
	else
	{
		result.m_Result = Write(pBuffer, bufferSize, &result.m_BytesTransfered);
	}

	return result;
}


BvFileResult BvFile::Seek(const i64 offset, const BvFileSeek seekFrom)
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	return SetFilePointerEx(m_hFile, *reinterpret_cast<const LARGE_INTEGER *>(&offset), nullptr,
		static_cast<std::underlying_type_t<BvFileSeek>>(seekFrom)) ? BvFileResult::kOk : BvFileResult::kError;
}


BvFileResult BvFile::SkipBytes(const i64 offset)
{
	return Seek(offset, BvFileSeek::kCurrent);
}


BvFileResult BvFile::GoToStart()
{
	return Seek(0, BvFileSeek::kBegin);
}


BvFileResult BvFile::GoToEnd()
{
	return Seek(0, BvFileSeek::kEnd);
}


const u64 BvFile::GetFilePos() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);
	u64 offset = 0; LARGE_INTEGER pos{};
	SetFilePointerEx(m_hFile, pos, reinterpret_cast<LARGE_INTEGER *>(&offset), FILE_CURRENT);

	return offset;
}


const u64 BvFile::GetSize() const
{
	BvAssert(m_hFile != INVALID_HANDLE_VALUE);

	u64 fileSize = 0;
	GetFileSizeEx(m_hFile, reinterpret_cast<PLARGE_INTEGER>(&fileSize));

	return fileSize;
}


const bool BvFile::IsValid() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}


#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)