#pragma once


#include "BvCore/System/File/BvFile.h"
#include "BvCore/System/File/BvAsyncFile.h"
#include "BvCore/System/File/BvPath.h"


#if defined(CreateFile)
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory
#undef GetCurrentDirectory
#endif


class BvFileSystem
{
public:
	BvFileSystem();
	~BvFileSystem();

	static bool FileExists(const char* pFileName);
	static bool DeleteFile(const char* pFileName);

	static bool FileExists(const wchar_t* pFileName);
	static bool DeleteFile(const wchar_t* pFileName);
	
	static bool DirectoryExists(const char* pDirName);
	static bool CreateDirectory(const char* pDirName);
	static bool DeleteDirectory(const char* pDirName, bool recurse = true);

	static bool DirectoryExists(const wchar_t* pDirName);
	static bool CreateDirectory(const wchar_t* pDirName);
	static bool DeleteDirectory(const wchar_t* pDirName, bool recurse = true);

	// This is used for asynchronous I/O where the buffer has to be aligned to
	// a multiple of the physical sector size
	static u32 GetPhysicalSectorSize();
};