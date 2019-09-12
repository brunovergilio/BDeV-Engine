#include "BvCore/System/File/FileSystem.h"
#include "BvCore/System/JobSystem/BvJobSystem.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


FileSystem FileSys;


FileSystem::FileSystem()
	: m_WorkSignal(BvEvent(false))
{
}


FileSystem::~FileSystem()
{
}


void FileSystem::Initialize()
{
	m_Active.Store(1);

	m_pRequests = new AsyncIORequest[kMaxAsyncIORequests];
	m_pRequestQueue = new BvSafeQueue<AsyncIORequest *, BvMutex>(kMaxAsyncIORequests);

	m_FileThread.Start(ThreadFunction);
}


void FileSystem::Shutdown()
{
	m_Active.Store(0);

	m_WorkSignal.Set();
	m_FileThread.Wait();

	delete[] m_pRequests;
	delete m_pRequestQueue;
}


BvFile FileSystem::OpenFile(const char * const pFilename, const BvFileAccess mode)
{
	BvAssert(pFilename != nullptr);

	DWORD acccessMode = 0;
	DWORD shareMode = 0;
	DWORD createMode = 0;
	DWORD createFlags = FILE_ATTRIBUTE_NORMAL;

	bool exists = FileSys.FileExists(pFilename);

	switch (mode)
	{
	case BvFileAccess::kRead:
		acccessMode = GENERIC_READ;
		shareMode = FILE_SHARE_READ;
		createMode = OPEN_EXISTING;
		break;
	case BvFileAccess::kWrite:
		acccessMode = GENERIC_WRITE;
		createMode = CREATE_ALWAYS;
		break;
	case BvFileAccess::kReadAndWrite:
		acccessMode = GENERIC_READ | GENERIC_WRITE;
		createMode = exists ? OPEN_EXISTING : CREATE_NEW;
		break;
	default:
		break;
	}

	return BvFile(CreateFile(pFilename, acccessMode, shareMode, nullptr, createMode, createFlags, nullptr));
}


void FileSystem::CloseFile(BvFile & file)
{
	if (file.m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file.m_hFile);
		file.m_hFile = INVALID_HANDLE_VALUE;
	}
}


AsyncIORequest * FileSystem::QueueAsyncRequest(BvFile * const pFile, void * const pBuffer, const u32 bufferSize,
	AsyncIORequest::AsyncOpType opType, pFnAsyncIOCallback pCallback, void * const pData)
{
	for (u32 i = 0; i < kMaxAsyncIORequests; i++)
	{
		if (m_pRequests[i].m_InUse.CompareExchange(1, 0) == 0)
		{
			m_pRequests[i].m_pFile = pFile;
			m_pRequests[i].m_pBuffer = pBuffer;
			m_pRequests[i].m_BufferSize = bufferSize;
			m_pRequests[i].m_pCallback = pCallback;
			m_pRequests[i].m_pData = pData;
			m_pRequests[i].m_OpType = opType;
			m_pRequests[i].m_Signal.Reset();

			m_pRequestQueue->Enqueue(&m_pRequests[i]);

			m_WorkSignal.Set();

			return &m_pRequests[i];
		}
	}

	return nullptr;
}


bool FileSystem::FileExists(const char * const pFileName) const
{
	if (GetFileAttributes(pFileName) == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
		{
			return false;
		}
		BvAssertMsg(error != 0, "Some other error happened");
	}

	return true;
}


bool FileSystem::DirectoryExists(const char * const pDirName) const
{
	return FileExists(pDirName);
}


void FileSystem::Process()
{
	while (m_Active.Load())
	{
		m_WorkSignal.Wait();

		if (!m_Active.Load())
		{
			return;
		}

		AsyncIORequest * pRequest = nullptr;
		while (m_pRequestQueue->Dequeue(pRequest))
		{
			if (pRequest->m_OpType == AsyncIORequest::AsyncOpType::kRead)
			{
				pRequest->m_Result = pRequest->m_pFile->Read(pRequest->m_pBuffer, pRequest->m_BufferSize, &pRequest->m_BytesTransfered);
			}
			else
			{
				pRequest->m_Result = pRequest->m_pFile->Write(pRequest->m_pBuffer, pRequest->m_BufferSize, &pRequest->m_BytesTransfered);
			}
			if ((pRequest->m_Result & BvFileResult::kIOSuccess) > 0)
			{
				pRequest->m_Result |= BvFileResult::kDoneAsync;
			}
			pRequest->m_Signal.Set();

			BvFileResultT result = pRequest->m_Result;
			u32 bytesTransfered = pRequest->m_BytesTransfered;
			void * pData = pRequest->m_pData;

			pRequest->m_InUse.Decrement();
			
			if (pRequest->m_pCallback)
			{
				pRequest->m_pCallback(result, bytesTransfered, pData);
			}
		}
	}
}


void FileSystem::ThreadFunction(void *)
{
	FileSys.Process();
}


#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)