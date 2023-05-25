#include "BDeV/System/File/BvPath.h"
#include "BDeV/System/File/BvFileCommon.h"
#include "BDeV/System/File/Windows/BvFileUtilsWindows.h"
#include <windows.h>
#include <Shlwapi.h>
#include <PathCch.h>


#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "pathcch.lib")


BvPath::BvPath()
{
}


BvPath::BvPath(const char* pPath)
{
	wchar_t path[kMaxPathSize];
	ConvertToWidePath(path, pPath);

	Prepare(path);
}


BvPath::BvPath(const wchar_t* pPath)
{
	Prepare(pPath);
}


BvPath::BvPath(const BvPath& rhs)
	: m_Path(rhs.m_Path), m_IsValid(rhs.IsValid()),
	m_IsAbsolute(rhs.m_IsAbsolute), m_IsFile(rhs.m_IsFile)
{
}


BvPath& BvPath::operator=(const BvPath& rhs)
{
	if (this != &rhs)
	{
		m_Path = rhs.m_Path;
		m_IsValid = rhs.m_IsValid;
		m_IsAbsolute = rhs.m_IsAbsolute;
		m_IsFile = rhs.m_IsFile;
	}
	return *this;
}


BvPath::BvPath(BvPath&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvPath& BvPath::operator=(BvPath&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_Path = std::move(rhs.m_Path);

		m_IsValid = rhs.m_IsValid;
		m_IsAbsolute = rhs.m_IsAbsolute;
		m_IsFile = rhs.m_IsFile;
	}
	return *this;
}


BvPath::~BvPath()
{
}


BvPath BvPath::FromCurrentDirectory()
{
	wchar_t widePath[kMaxPathSize]{};
	if (!GetCurrentDirectoryW(kMaxPathSize - 1, widePath))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return BvPath();
	}

	return BvPath(widePath);
}


BvWString BvPath::GetAbsoluteName() const
{
	if (!m_IsValid)
	{
		return BvWString();
	}

	if (m_IsAbsolute)
	{
		if (!wcsncmp(m_Path.CStr(), kWideFilePrefix, kWideFilePrefixSize))
		{
			return BvWString(m_Path.CStr() + kWideFilePrefixSize);
		}

		return m_Path;
	}
	else
	{
		wchar_t path[kMaxPathSize];
		GetFullPathNameW(m_Path.CStr(), kMaxPathSize - 1, path, nullptr);

		return BvWString(path);
	}
}


BvWString BvPath::GetRelativeName(const wchar_t* pRelativeToPath) const
{
	if (!m_IsValid)
	{
		return BvWString();
	}

	wchar_t relativePath[kMaxPathSize]{};
	if (!m_IsAbsolute)
	{
		wchar_t fullPath[kMaxPathSize];
		if (!pRelativeToPath || pRelativeToPath[0] == 0)
		{
			GetFullPathNameW(m_Path.CStr(), kMaxPathSize - 1, fullPath, nullptr);
			wchar_t root[4];
			auto pAfterRoot = PathSkipRootW(fullPath);
			auto count = (size_t)(pAfterRoot - fullPath);
			wcsncpy_s(root, fullPath, count);
			PathRelativePathToW(relativePath, m_Path.CStr() + kWideFilePrefixSize,
				m_IsFile ? 0 : FILE_ATTRIBUTE_DIRECTORY, root, FILE_ATTRIBUTE_DIRECTORY);
		}
		else
		{
			GetFullPathNameW(m_Path.CStr(), kMaxPathSize - 1, fullPath, nullptr);
			auto attributes = GetFileAttributesW(pRelativeToPath);
			PathRelativePathToW(relativePath, fullPath, m_IsFile ? 0 : FILE_ATTRIBUTE_DIRECTORY,
				pRelativeToPath, (attributes & FILE_ATTRIBUTE_DIRECTORY));
		}
	}
	else
	{
		if (!pRelativeToPath || pRelativeToPath[0] == 0)
		{
			wchar_t root[4]{};
			auto pAfterRoot = PathSkipRootW(m_Path.CStr());
			auto count = (size_t)(pAfterRoot - m_Path.CStr());
			wcsncpy_s(root, m_Path.CStr() + kWideFilePrefixSize,
				count - kWideFilePrefixSize);
			PathRelativePathToW(relativePath, m_Path.CStr() + kWideFilePrefixSize,
				m_IsFile ? 0 : FILE_ATTRIBUTE_DIRECTORY, root, FILE_ATTRIBUTE_DIRECTORY);
		}
		else
		{
			auto attributes = GetFileAttributesW(pRelativeToPath);
			PathRelativePathToW(relativePath, m_Path.CStr() + kWideFilePrefixSize, m_IsFile ? 0 : FILE_ATTRIBUTE_DIRECTORY,
				pRelativeToPath, (attributes & FILE_ATTRIBUTE_DIRECTORY));
		}

	}
	
	return BvWString(relativePath);
}


BvWString BvPath::GetName() const
{
	if (!m_IsValid)
	{
		return BvWString();
	}

	wchar_t path[kMaxPathSize];
	wcsncpy_s(path, m_Path.CStr(), kMaxPathSize - 1);
	PathStripPathW(path);
	auto pExt = PathFindExtensionW(path);
	if (pExt)
	{
		*pExt = 0;
	}

	return BvWString(path);
}


BvWString BvPath::GetNameAndExt() const
{
	if (!m_IsValid)
	{
		return BvWString();
	}

	wchar_t path[kMaxPathSize];
	wcsncpy_s(path, m_Path.CStr(), kMaxPathSize - 1);
	PathStripPathW(path);

	return BvWString(path);
}


BvWString BvPath::GetExt() const
{
	if (!m_IsValid)
	{
		return BvWString();
	}

	auto pExt = PathFindExtensionW(m_Path.CStr());
	if (pExt)
	{
		return BvWString(pExt);
	}
	else
	{
		return BvWString();
	}
}


BvWString BvPath::GetRoot() const
{
	if (!m_IsValid)
	{
		return BvWString();
	}

	wchar_t root[kMaxPathSize];
	if (m_IsAbsolute)
	{
		wcsncpy_s(root, m_Path.CStr(), kMaxPathSize - 1);
	}
	else
	{
		GetFullPathNameW(m_Path.CStr(), kMaxPathSize - 1, root, nullptr);
	}
	PathCchStripToRoot(root, kMaxPathSize);

	return BvWString(root + (!wcsncmp(root, kWideFilePrefix, kWideFilePrefixSize) ? kWideFilePrefixSize : 0));
}


bool BvPath::IsAbsolute() const
{
	return m_IsValid && m_IsAbsolute;
}


bool BvPath::IsRelative() const
{
	return m_IsValid && !m_IsAbsolute;
}


bool BvPath::IsValid() const
{
	return m_IsValid;
}


bool BvPath::IsFile() const
{
	return m_IsValid && m_IsFile;
}


bool BvPath::IsDirectory() const
{
	return m_IsValid && !m_IsFile;
}


BvFile BvPath::AsFile(BvFileAccessMode mode) const
{
	return m_IsFile ? BvFile(m_Path.CStr(), mode, BvFileAction::kOpen) : BvFile();
}


BvVector<BvPath> BvPath::GetFileList(const char* const pFilter) const
{
	wchar_t filter[kMaxPathSize];
	ConvertToWidePath(filter, pFilter);
	return GetFileList(filter);
}


BvVector<BvPath> BvPath::GetFileList(const wchar_t* const pFilter) const
{
	WIN32_FIND_DATAW findData;
	wchar_t pathWithFilter[kMaxPathSize]{};
	wcsncpy_s(pathWithFilter, m_Path.CStr(), kMaxPathSize - 1);
	wcsncat_s(pathWithFilter, L"\\", kMaxPathSize - 1);
	wcsncat_s(pathWithFilter, pFilter, kMaxPathSize - 1);

	BvVector<BvPath> fileList;

	auto hFind = FindFirstFileW(pathWithFilter, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		// TODO: Handle error
		return fileList;
	}

	do
	{
		BvWString filename(findData.cFileName);
		if (filename != L"." && filename != L"..")
		{
			filename.Insert(m_Path, 0);
			filename.Insert(L'\\', m_Path.Size());
			fileList.EmplaceBack(filename);
		}
	} while (FindNextFileW(hFind, &findData) != FALSE);

	DWORD error = GetLastError();
	if (error != ERROR_NO_MORE_FILES)
	{
		// TODO: Handle error
		return fileList;
	}

	if (!FindClose(hFind))
	{
		// TODO: Handle error
		return fileList;
	}

	return fileList;
}


void BvPath::Prepare(const wchar_t* const pPath)
{
	m_IsAbsolute = PathIsRelativeW(pPath) == FALSE;

	wchar_t path[kMaxPathSize]{};
	DWORD attributes = 0;
	if (m_IsAbsolute)
	{
		if (wcsncmp(L"\\\\\?\\", pPath, kWideFilePrefixSize) != 0)
		{
			AddPrefixForMaxPathLimit(path, pPath);
		}
		else
		{
			wcsncpy_s(path, pPath, kMaxPathSize - 1);
		}
	}
	else
	{
		// The "\\?\" prefix won't work for relative paths
		wcsncpy_s(path, pPath, kMaxPathSize - 1);
	}

	attributes = GetFileAttributesW(path);
	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		// TODO: Handle error
	}
	else
	{
		m_Path = path;
		m_IsFile = (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
		m_IsValid = true;
	}
}