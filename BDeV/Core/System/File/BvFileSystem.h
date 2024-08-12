#pragma once


#include "BDeV/Core/System/File/BvFile.h"
#include "BDeV/Core/System/File/BvAsyncFile.h"
#include "BDeV/Core/System/File/BvPath.h"


class BvFileSystem
{
public:
	static bool FileExists(const char* pFilename);
	static bool DeleteFile(const char* pFilename);

	static bool DirectoryExists(const char* pDirName);
	static bool CreateDirectory(const char* pDirName);
	static bool DeleteDirectory(const char* pDirName, bool recurse = true);

	// This is used for asynchronous I/O where the buffer has to be aligned to
	// as well as have a size that is a multiple of the physical sector size
	static u32 GetPhysicalSectorSize();
};