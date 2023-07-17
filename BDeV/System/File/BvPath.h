#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/System/File/BvFile.h"
#include "BDeV/System/File/BvAsyncFile.h"


class BvPath
{
public:
	BvPath();
	BvPath(const char* pPath);
	BvPath(const wchar_t* pPath);
	BvPath(const BvPath& rhs);
	BvPath& operator=(const BvPath& rhs);
	BvPath(BvPath&& rhs) noexcept;
	BvPath& operator=(BvPath&& rhs) noexcept;
	~BvPath();

	static BvPath FromCurrentDirectory();

	//BvPath& Parent();
	//BvPath& Enter(const char* pDirectory);
	//
	BvWString GetAbsoluteName() const;
	BvWString GetRelativeName(const wchar_t* pRelativeToPath = nullptr) const;
	BvWString GetName() const;
	BvWString GetNameAndExt() const;
	BvWString GetExt() const;
	BvWString GetRoot() const;

	bool IsAbsolute() const;
	bool IsRelative() const;
	
	bool IsValid() const;
	
	bool IsFile() const;
	bool IsDirectory() const;
	
	BvFile AsFile(BvFileAccessMode mode = BvFileAccessMode::kReadWrite) const;

	BvVector<BvPath> GetFileList(const char* const pFilter = "*") const;
	BvVector<BvPath> GetFileList(const wchar_t* const pFilter = L"*") const;
	
private:
	void Prepare(const wchar_t* const pPath);

private:
	BvWString m_Path;
	bool m_IsAbsolute = false;
	bool m_IsValid = false;
	bool m_IsFile = false;
};