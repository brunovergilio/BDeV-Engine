#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/System/File/BvFile.h"
#include "BDeV/Core/System/File/BvAsyncFile.h"


class BvPath
{
public:
	BvPath();
	explicit BvPath(const char* pPath);
	explicit BvPath(const char* pPath, u32 count);
	explicit BvPath(const BvString& path);
	BvPath(const BvPath& rhs);
	BvPath& operator=(const BvPath& rhs);
	BvPath& operator=(const char* pPath);
	BvPath& operator=(const BvString& path);
	BvPath(BvPath&& rhs) noexcept;
	BvPath& operator=(BvPath&& rhs) noexcept;
	~BvPath();

	static BvPath FromCurrentDirectory();
	static BvPath FromCurrentDrive();

	bool IsAbsolute() const;
	bool IsRelative() const;
	bool IsValid() const;
	bool IsFile() const;
	bool IsDirectory() const;

	BvString GetName() const;
	BvString GetNameAndExtension() const;
	BvString GetExtension() const;
	BvString GetRoot() const;
	BvString GetRootName() const;
	BvString GetRootDirectory() const;

	BvPath GetNormalizedPath() const;
	BvPath GetAbsolutePath() const;
	BvPath GetRelativePath() const;
	
	BvPath GetParentPath() const;

	BvPath GetAppendedPath(const char* pPath) const;
	BvPath GetAppendedPath(const BvString& path) const;
	BvPath GetAppendedPath(const BvPath& path) const;

	BvPath GetPrependedPath(const char* pPath) const;
	BvPath GetPrependedPath(const BvString& path) const;
	BvPath GetPrependedPath(const BvPath& path) const;

	void NormalizePath();
	void ConvertToAbsolutePath();
	void ConvertToRelativePath();

	void MoveToParentPath();

	void AppendPath(const char* pPath);
	void AppendPath(const BvString& path);
	void AppendPath(const BvPath& path);

	void PrependPath(const char* pPath);
	void PrependPath(const BvString& path);
	void PrependPath(const BvPath& path);

	BvPath& operator/=(const char* pPath);
	BvPath& operator/=(const BvString& path);
	BvPath& operator/=(const BvPath& path);

	BvPath& operator--();
	BvPath operator--(i32);

	bool operator==(const BvPath& path) const;
	bool operator!=(const BvPath& path) const;
	
	BvFile AsFile(BvFileAccessMode mode = BvFileAccessMode::kReadWrite) const;
	BvAsyncFile AsAsyncFile(BvFileAccessMode mode = BvFileAccessMode::kReadWrite) const;

	BvVector<BvPath> GetFileList(const char* pFilter = "*") const;

	const BvString& GetStr() const { return m_Path; }

private:
	BvString m_Path;
};