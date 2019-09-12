#pragma once


#include "BvCore/System/JobSystem/SafeQueue.h"
#include "BvCore/BvDebug.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/System/Threading/BvThread.h"
#include "BvCore/System/Threading/BvSync.h"
#include "BvCore/System/File/BvFile.h"


class FileSystem
{
	BV_SINGLETON(FileSystem);
public:
	FileSystem();
	~FileSystem();

	void Initialize();
	void Shutdown();

	BvFile OpenFile(const char * const pFilename, const BvFileAccess mode = BvFileAccess::kReadAndWrite);
	void CloseFile(BvFile & file);

	AsyncIORequest * QueueAsyncRequest(BvFile * const pFile, void * const pBuffer, const u32 bufferSize,
		AsyncIORequest::AsyncOpType opType, pFnAsyncIOCallback pCallback = nullptr, void * const pData = nullptr);

	bool FileExists(const char * const pFileName) const;
	bool DirectoryExists(const char * const pDirName) const;

private:
	void Process();
	static void ThreadFunction(void * pData);

private:
	BvThread m_FileThread;
	BvEvent m_WorkSignal;
	BvAtomic m_Active;
	AsyncIORequest * m_pRequests = nullptr;
	BvSafeQueue<AsyncIORequest *, BvMutex> * m_pRequestQueue = nullptr;
};


extern FileSystem FileSys;