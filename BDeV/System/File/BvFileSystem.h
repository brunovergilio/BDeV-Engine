#pragma once


#include "BDeV/System/File/BvFile.h"
#include "BDeV/System/File/BvAsyncFile.h"
#include "BDeV/System/File/BvPath.h"


class BV_API BvFileSystem
{
public:
	static bool FileExists(const char* pFileName);
	static bool DelFile(const char* pFileName);

	static bool FileExists(const wchar_t* pFileName);
	static bool DelFile(const wchar_t* pFileName);

	static bool DirExists(const char* pDirName);
	static bool MakeDir(const char* pDirName);
	static bool DelDir(const char* pDirName, bool recurse = true);

	static bool DirExists(const wchar_t* pDirName);
	static bool MakeDir(const wchar_t* pDirName);
	static bool DelDir(const wchar_t* pDirName, bool recurse = true);

	// This is used for asynchronous I/O where the buffer has to be aligned to
	// a multiple of the physical sector size
	static u32 GetPhysicalSectorSize();
};