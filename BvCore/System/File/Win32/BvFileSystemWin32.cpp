#include "BvCore/System/File/Win32/BvFileSystemWin32.h"
#include "BvCore/System/JobSystem/BvJobSystem.h"


BvFileSystem::BvFileSystem()
{
}


BvFileSystem::~BvFileSystem()
{
}


BvFile BvFileSystem::OpenFile(const char * const pFilename, const BvFileAccess mode)
{
	BvAssert(pFilename != nullptr);

	DWORD acccessMode = 0;
	DWORD shareMode = 0;
	DWORD createMode = 0;
	DWORD createFlags = FILE_ATTRIBUTE_NORMAL;

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
	case BvFileAccess::kReadWrite:
		acccessMode = GENERIC_READ | GENERIC_WRITE;
		createMode = FileExists(pFilename) ? OPEN_EXISTING : CREATE_NEW;
		break;
	}

	auto handle = CreateFile(pFilename, acccessMode, shareMode, nullptr, createMode, createFlags, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// TODO: Handle error
	}

	return BvFile(handle);
}


BvAsyncFile BvFileSystem::OpenAsyncFile(const char * const pFilename, const BvFileAccess mode)
{
	BvAssert(pFilename != nullptr);

	DWORD acccessMode = 0;
	DWORD shareMode = 0;
	DWORD createMode = 0;
	DWORD createFlags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING;

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
	case BvFileAccess::kReadWrite:
		acccessMode = GENERIC_READ | GENERIC_WRITE;
		createMode = FileExists(pFilename) ? OPEN_EXISTING : CREATE_NEW;
		break;
	}

	auto handle = CreateFile(pFilename, acccessMode, shareMode, nullptr, createMode, createFlags, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// TODO: Handle error
	}

	return BvAsyncFile(handle);
}


void BvFileSystem::CloseFile(BvFile & file)
{
	if (file.m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file.m_hFile);
		file.m_hFile = INVALID_HANDLE_VALUE;
	}
}

void BvFileSystem::CloseFile(BvAsyncFile & file)
{
	BvFile tmpFile(file.m_hFile);
	CloseFile(tmpFile);
}


bool BvFileSystem::FileExists(const char * const pFileName) const
{
	if (GetFileAttributes(pFileName) == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
		{
			BvAssertMsg(error != 0, "Some other error happened");
		}
		return false;
	}

	return true;
}


bool BvFileSystem::DirectoryExists(const char * const pDirName) const
{
	return FileExists(pDirName);
}