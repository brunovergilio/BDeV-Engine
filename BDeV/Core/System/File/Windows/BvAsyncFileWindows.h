#pragma once


#include "BDeV/Core/System/BvPlatformHeaders.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include <atomic>


class BvAsyncFileRequest
{
	BV_NOCOPYMOVE(BvAsyncFileRequest);

public:
	friend class BvAsyncFile;

	BvAsyncFileRequest();
	~BvAsyncFileRequest();

	bool IsDone();
	u32 GetResult(bool wait = true);
	void Cancel();

private:
	void IsDone(bool done);
	void Reset(HANDLE hFile, u64 position);

private:
	struct AsyncFileData : OVERLAPPED
	{
		HANDLE m_hFile;
		std::atomic<u32> m_TotalBytesProcessed;
		std::atomic<bool> m_IsDone;
		std::atomic<bool> m_Flag;
	};

	AsyncFileData m_AsyncFileData{};
};