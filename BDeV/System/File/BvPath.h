#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/System/File/BvFile.h"
#include "BDeV/System/File/BvAsyncFile.h"


class BvPath
{
public:
	using BvPathStringType = 
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	BvWString;
#else
	BvString;
#endif

	BvPath();
	explicit BvPath(const char* pPath);
	explicit BvPath(const char* pPath, u32 count);
	explicit BvPath(const wchar_t* pPath);
	explicit BvPath(const wchar_t* pPath, u32 count);
	explicit BvPath(const BvString& path);
	explicit BvPath(const BvWString& path);
	BvPath(const BvPath& rhs);
	BvPath& operator=(const BvPath& rhs);
	BvPath& operator=(const char* pPath);
	BvPath& operator=(const wchar_t* pPath);
	BvPath& operator=(const BvString& path);
	BvPath& operator=(const BvWString& path);
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

	BvPathStringType GetName() const;
	BvPathStringType GetNameAndExtension() const;
	BvPathStringType GetExtension() const;
	BvPathStringType GetRoot() const;
	BvPathStringType GetRootName() const;
	BvPathStringType GetRootDirectory() const;

	BvPath GetNormalizedPath() const;
	BvPath GetAbsolutePath() const;
	BvPath GetRelativePath() const;
	
	BvPath GetParentPath() const;

	BvPath GetAppendedPath(const char* pPath) const;
	BvPath GetAppendedPath(const wchar_t* pPath) const;
	BvPath GetAppendedPath(const BvString& path) const;
	BvPath GetAppendedPath(const BvWString& path) const;
	BvPath GetAppendedPath(const BvPath& path) const;

	BvPath GetPrependedPath(const char* pPath) const;
	BvPath GetPrependedPath(const wchar_t* pPath) const;
	BvPath GetPrependedPath(const BvString& path) const;
	BvPath GetPrependedPath(const BvWString& path) const;
	BvPath GetPrependedPath(const BvPath& path) const;

	void NormalizePath();
	void ConvertToAbsolutePath();
	void ConvertToRelativePath();

	void MoveToParentPath();

	void AppendPath(const char* pPath);
	void AppendPath(const wchar_t* pPath);
	void AppendPath(const BvString& path);
	void AppendPath(const BvWString& path);
	void AppendPath(const BvPath& path);

	void PrependPath(const char* pPath);
	void PrependPath(const wchar_t* pPath);
	void PrependPath(const BvString& path);
	void PrependPath(const BvWString& path);
	void PrependPath(const BvPath& path);

	BvPath& operator/=(const char* pPath);
	BvPath& operator/=(const wchar_t* pPath);
	BvPath& operator/=(const BvString& path);
	BvPath& operator/=(const BvWString& path);
	BvPath& operator/=(const BvPath& path);

	BvPath& operator--();
	BvPath operator--(i32);

	bool operator==(const BvPath& path) const;
	bool operator!=(const BvPath& path) const;
	
	BvFile AsFile(BvFileAccessMode mode = BvFileAccessMode::kReadWrite) const;
	BvAsyncFile AsAsyncFile(BvFileAccessMode mode = BvFileAccessMode::kReadWrite) const;

	BvVector<BvPath> GetFileList(const char* pFilter = "*") const;
	BvVector<BvPath> GetFileList(const wchar_t* pFilter = L"*") const;

	const BvPathStringType& GetStr() const { return m_Path; }

private:
	BvPathStringType m_Path;
};