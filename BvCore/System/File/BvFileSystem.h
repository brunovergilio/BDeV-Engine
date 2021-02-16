#pragma once


#include "BvCore/System/File/BvFile.h"
#include "BvCore/System/File/BvAsyncFile.h"


class BvFileSystem
{
public:
	BvFileSystem();
	~BvFileSystem();

	BvFile OpenFile(const char * const pFilename, const BvFileAccess mode = BvFileAccess::kReadWrite);
	BvAsyncFile OpenAsyncFile(const char * const pFilename, const BvFileAccess mode = BvFileAccess::kReadWrite);

	void CloseFile(BvFile & file);
	void CloseFile(BvAsyncFile & file);

	bool FileExists(const char * const pFileName) const;
	bool DirectoryExists(const char * const pDirName) const;

private:
};