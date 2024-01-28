#include "BDeV/System/File/BvPath.h"
#include "BDeV/System/File/BvFileCommon.h"
#include "BDeV/System/Windows/BvWindowsHeader.h"
#include <algorithm>


namespace BvPathUtils
{
	void ConvertToWidePath(BvWString& result, const char* pSrcBegin, const char* pSrcEnd, const std::locale& loc = std::locale());
	bool IsSlash(wchar_t ch);
	u32 GetEndOfRoot(const wchar_t* pPath, u32 size);
	bool HasRootName(const wchar_t* pPath, u32 size);
	std::wstring_view GetRootName(const wchar_t* pPath, u32 size);
	bool HasRootSlash(const wchar_t* pPath, u32 size);
	bool HasDriveLetter(const wchar_t* pPath, u32 size);
	bool IsPathAbsolute(const wchar_t* pPath, u32 size);
	u32 FindRelativePath(const wchar_t* pPath, u32 size);
	u32 FindName(const wchar_t* pPath, u32 size);
	u32 FindExtension(const wchar_t* pPath, u32 size);
	u32 GetFileAttributeFlags(const BvWString& path);
	bool HasUNCPathAndDrive(const wchar_t* pPath, u32 size);
	bool IsRelativePathWithRootName(const wchar_t* pPath, u32 size);
	void NormalizePath(const BvWString& path, BvWString& normalizedPath);
	u32 GetParentPathEnd(const wchar_t* pPath, u32 size);
	void AppendPath(BvWString& result, const wchar_t* pPath);
	void AppendPath(BvWString& result, const BvWString& path);
}


BvPath::BvPath()
{
}


BvPath::BvPath(const char* pPath)
{
	BvPathUtils::ConvertToWidePath(m_Path, pPath, pPath + strlen(pPath));
}


BvPath::BvPath(const char* pPath, u32 count)
{
	BvPathUtils::ConvertToWidePath(m_Path, pPath, pPath + count);
}


BvPath::BvPath(const wchar_t* pPath)
	: m_Path(pPath)
{
}


BvPath::BvPath(const wchar_t* pPath, u32 count)
	: m_Path(pPath, 0, count)
{
}


BvPath::BvPath(const BvString& path)
{
	BvPathUtils::ConvertToWidePath(m_Path, path.CStr(), path.CStr() + path.Size());
}


BvPath::BvPath(const BvWString& path)
	: m_Path(path)
{
}


BvPath::BvPath(const BvPath& rhs)
	: m_Path(rhs.m_Path)
{
}


BvPath& BvPath::operator=(const BvPath& rhs)
{
	if (this != &rhs)
	{
		m_Path = rhs.m_Path;
	}
	return *this;
}


BvPath& BvPath::operator=(const char* pPath)
{
	BvPathUtils::ConvertToWidePath(m_Path, pPath, pPath + strlen(pPath));

	return *this;
}


BvPath& BvPath::operator=(const wchar_t* pPath)
{
	m_Path = pPath;

	return *this;
}


BvPath& BvPath::operator=(const BvString& path)
{
	BvPathUtils::ConvertToWidePath(m_Path, path.CStr(), path.CStr() + path.Size());

	return *this;
}


BvPath& BvPath::operator=(const BvWString& path)
{
	m_Path = path;

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
	}
	return *this;
}


BvPath::~BvPath()
{
}


BvPath BvPath::FromCurrentDirectory()
{
	auto sizeNeeded = GetCurrentDirectoryW(0, nullptr);
	if (!sizeNeeded)
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return BvPath();
	}
	BvPath currentPath;
	currentPath.m_Path.Resize(sizeNeeded - 1);
	if (!GetCurrentDirectoryW(sizeNeeded, &currentPath.m_Path[0]))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return BvPath();
	}

	return currentPath;
}


BvPath BvPath::FromCurrentDrive()
{
	auto sizeNeeded = GetCurrentDirectoryW(0, nullptr);
	if (!sizeNeeded)
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return BvPath();
	}
	BvPath currentPath;
	currentPath.m_Path.Resize(sizeNeeded - 1);
	if (!GetCurrentDirectoryW(sizeNeeded, &currentPath.m_Path[0]))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return BvPath();
	}
	currentPath.m_Path.Erase(0, BvPathUtils::GetEndOfRoot(currentPath.m_Path.CStr(), currentPath.m_Path.Size()) + 1);

	return currentPath;
}


bool BvPath::IsAbsolute() const
{
	return BvPathUtils::IsPathAbsolute(m_Path.CStr(), m_Path.Size());
}


bool BvPath::IsRelative() const
{
	return !IsAbsolute();
}


bool BvPath::IsValid() const
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	return GetFileAttributesExW(m_Path.CStr(), GetFileExInfoStandard, &fad);
}


bool BvPath::IsFile() const
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	return GetFileAttributesExW(m_Path.CStr(), GetFileExInfoStandard, &fad) ? (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 : false;
}


bool BvPath::IsDirectory() const
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	return GetFileAttributesExW(m_Path.CStr(), GetFileExInfoStandard, &fad) ? (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 : false;
}


BvWString BvPath::GetName() const
{
	auto nameIndex = BvPathUtils::FindName(m_Path.CStr(), m_Path.Size());
	auto extIndex = BvPathUtils::FindExtension(m_Path.CStr(), m_Path.Size());
	auto count = extIndex - nameIndex;

	return BvWString(m_Path.CStr(), nameIndex, count);
}


BvWString BvPath::GetNameAndExtension() const
{
	auto nameIndex = BvPathUtils::FindName(m_Path.CStr(), m_Path.Size());
	return BvWString(m_Path.CStr(), nameIndex, m_Path.Size() - nameIndex);
}


BvWString BvPath::GetExtension() const
{
	auto extIndex = BvPathUtils::FindExtension(m_Path.CStr(), m_Path.Size());
	return BvWString(m_Path.CStr(), extIndex, m_Path.Size() - extIndex);
}


BvWString BvPath::GetRoot() const
{
	auto rootDirEnd = BvPathUtils::GetEndOfRoot(m_Path.CStr(), m_Path.Size());
	return BvWString(m_Path, 0, rootDirEnd + (BvPathUtils::IsSlash(m_Path[rootDirEnd]) ? 1 : 0));
}


BvWString BvPath::GetRootName() const
{
	auto rootNameView = BvPathUtils::GetRootName(m_Path.CStr(), m_Path.Size());
	return BvWString(rootNameView.data(), 0, rootNameView.size());
}


BvWString BvPath::GetRootDirectory() const
{
	auto pRootEnd = m_Path.CStr() + BvPathUtils::GetEndOfRoot(m_Path.CStr(), m_Path.Size());
	auto pRelativePath = std::find_if_not(pRootEnd, m_Path.LCStr(), BvPathUtils::IsSlash);
	return BvWString(pRootEnd, 0, static_cast<u32>(pRelativePath - pRootEnd));
}


BvPath BvPath::GetNormalizedPath() const
{
	BvPath normalizedPath;
	BvPathUtils::NormalizePath(m_Path, normalizedPath.m_Path);

	return normalizedPath;
}


BvPath BvPath::GetAbsolutePath() const
{
	if (IsAbsolute())
	{
		return *this;
	}

	u32 sizeNeeded = GetFullPathNameW(m_Path.CStr(), 0, nullptr, nullptr);
	if (sizeNeeded == 0)
	{
		// TODO: Handle error
		return BvPath();
	}

	BvPath path;
	path.m_Path.Resize(sizeNeeded);
	GetFullPathNameW(m_Path.CStr(), path.m_Path.Size(), &path.m_Path[0], nullptr);
	if (sizeNeeded == 0)
	{
		// TODO: Handle error
		return BvPath();
	}

	return path;
}


BvPath BvPath::GetRelativePath() const
{
	auto index = BvPathUtils::FindRelativePath(m_Path.CStr(), m_Path.Size());
	return BvPath(m_Path.CStr() + index, m_Path.Size() - index);
}


BvPath BvPath::GetParentPath() const
{
	return BvPath(m_Path.CStr(), BvPathUtils::GetParentPathEnd(m_Path.CStr(), m_Path.Size()));
}


BvPath BvPath::GetAppendedPath(const char* pPath) const
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvPath appendedPath(m_Path);
	BvWString path;
	BvPathUtils::ConvertToWidePath(path, pPath, pPath + strlen(pPath));
	BvPathUtils::AppendPath(appendedPath.m_Path, path);

	return appendedPath;
}


BvPath BvPath::GetAppendedPath(const wchar_t* pPath) const
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvPath appendedPath(m_Path);
	BvPathUtils::AppendPath(appendedPath.m_Path, pPath);

	return appendedPath;
}


BvPath BvPath::GetAppendedPath(const BvString& path) const
{
	BvPath appendedPath(m_Path);
	BvWString pathW;
	BvPathUtils::ConvertToWidePath(pathW, path.CStr(), path.LCStr());
	BvPathUtils::AppendPath(appendedPath.m_Path, pathW);

	return appendedPath;
}


BvPath BvPath::GetAppendedPath(const BvWString& path) const
{
	BvPath appendedPath(m_Path);
	BvPathUtils::AppendPath(appendedPath.m_Path, path);

	return appendedPath;
}


BvPath BvPath::GetAppendedPath(const BvPath& path) const
{
	BvPath appendedPath(m_Path);
	BvPathUtils::AppendPath(appendedPath.m_Path, path.m_Path);

	return appendedPath;
}


BvPath BvPath::GetPrependedPath(const char* pPath) const
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvWString path;
	BvPathUtils::ConvertToWidePath(path, pPath, pPath + strlen(pPath));
	BvPath prependedPath;
	prependedPath.m_Path = std::move(path);
	BvPathUtils::AppendPath(prependedPath.m_Path, m_Path);

	return prependedPath;
}


BvPath BvPath::GetPrependedPath(const wchar_t* pPath) const
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvPath prependedPath(pPath);
	BvPathUtils::AppendPath(prependedPath.m_Path, m_Path);

	return prependedPath;
}


BvPath BvPath::GetPrependedPath(const BvString& path) const
{
	BvWString pathW;
	BvPathUtils::ConvertToWidePath(pathW, path.CStr(), path.LCStr());
	BvPath prependedPath;
	prependedPath.m_Path = std::move(pathW);
	BvPathUtils::AppendPath(prependedPath.m_Path, m_Path);

	return prependedPath;
}


BvPath BvPath::GetPrependedPath(const BvWString& path) const
{
	BvPath prependedPath(path);
	BvPathUtils::AppendPath(prependedPath.m_Path, m_Path);

	return prependedPath;
}


BvPath BvPath::GetPrependedPath(const BvPath& path) const
{
	BvPath prependedPath(path);
	BvPathUtils::AppendPath(prependedPath.m_Path, m_Path);

	return prependedPath;
}


void BvPath::NormalizePath()
{
	BvWString normalizedPath;
	BvPathUtils::NormalizePath(m_Path, normalizedPath);
	std::swap(m_Path, normalizedPath);
}


void BvPath::ConvertToAbsolutePath()
{
	if (IsAbsolute())
	{
		return;
	}

	u32 sizeNeeded = GetFullPathNameW(m_Path.CStr(), 0, nullptr, nullptr);
	if (sizeNeeded == 0)
	{
		// TODO: Handle error
		return;
	}

	BvWString path(sizeNeeded);
	GetFullPathNameW(m_Path.CStr(), path.Size(), &path[0], nullptr);
	if (sizeNeeded == 0)
	{
		// TODO: Handle error
		return;
	}

	std::swap(m_Path, path);
}


void BvPath::ConvertToRelativePath()
{
	auto index = BvPathUtils::FindRelativePath(m_Path.CStr(), m_Path.Size());
	if (index != 0)
	{
		m_Path.Erase(0, index - 0);
	}
}


void BvPath::MoveToParentPath()
{
	auto parentEndIndex = BvPathUtils::GetParentPathEnd(m_Path.CStr(), m_Path.Size());
	m_Path.Erase(parentEndIndex, m_Path.Size() - parentEndIndex);
}


void BvPath::AppendPath(const char* pPath)
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvWString path;
	BvPathUtils::ConvertToWidePath(path, pPath, pPath + strlen(pPath));
	BvPathUtils::AppendPath(m_Path, path);
}


void BvPath::AppendPath(const wchar_t* pPath)
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvPathUtils::AppendPath(m_Path, pPath);
}


void BvPath::AppendPath(const BvString& path)
{
	BvWString pathW;
	BvPathUtils::ConvertToWidePath(pathW, path.CStr(), path.LCStr());
	BvPathUtils::AppendPath(m_Path, pathW);
}


void BvPath::AppendPath(const BvWString& path)
{
	BvPathUtils::AppendPath(m_Path, path);
}


void BvPath::AppendPath(const BvPath& path)
{
	BvPathUtils::AppendPath(m_Path, path.m_Path);
}


void BvPath::PrependPath(const char* pPath)
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvWString path;
	BvPathUtils::ConvertToWidePath(path, pPath, pPath + strlen(pPath));
	BvPathUtils::AppendPath(path, m_Path);
	m_Path = std::move(path);
}


void BvPath::PrependPath(const wchar_t* pPath)
{
	BvAssert(pPath != nullptr, "Invalid path string");

	BvWString path(pPath);
	BvPathUtils::AppendPath(path, m_Path);
	m_Path = std::move(path);
}


void BvPath::PrependPath(const BvString& path)
{
	BvWString pathW;
	BvPathUtils::ConvertToWidePath(pathW, path.CStr(), path.LCStr());
	BvPathUtils::AppendPath(pathW, m_Path);
	m_Path = std::move(pathW);
}


void BvPath::PrependPath(const BvWString& path)
{
	BvWString prependedPath(path);
	BvPathUtils::AppendPath(prependedPath, m_Path);
	m_Path = std::move(prependedPath);
}


void BvPath::PrependPath(const BvPath& path)
{
	BvWString prependedPath(path.m_Path);
	BvPathUtils::AppendPath(prependedPath, m_Path);
	m_Path = std::move(prependedPath);
}


BvPath& BvPath::operator/=(const char* pPath)
{
	AppendPath(pPath);

	return *this;
}


BvPath& BvPath::operator/=(const wchar_t* pPath)
{
	AppendPath(pPath);

	return *this;
}


BvPath& BvPath::operator/=(const BvString& path)
{
	AppendPath(path);

	return *this;
}


BvPath& BvPath::operator/=(const BvWString& path)
{
	AppendPath(path);

	return *this;
}


BvPath& BvPath::operator/=(const BvPath& path)
{
	AppendPath(path);

	return *this;
}


BvPath& BvPath::operator--()
{
	MoveToParentPath();
	return *this;
}


BvPath BvPath::operator--(i32)
{
	BvPath path(*this);
	MoveToParentPath();

	return path;
}


bool BvPath::operator==(const BvPath& path) const
{
	return m_Path == path.m_Path;
}


bool BvPath::operator!=(const BvPath& path) const
{
	return m_Path != path.m_Path;
}


BvFile BvPath::AsFile(BvFileAccessMode mode) const
{
	return IsFile() ? BvFile(m_Path.CStr(), mode, BvFileAction::kOpen) : BvFile();
}


BvAsyncFile BvPath::AsAsyncFile(BvFileAccessMode mode) const
{
	return IsFile() ? BvAsyncFile(m_Path.CStr(), mode, BvFileAction::kOpen) : BvAsyncFile();
}


void GetFileListFromPathWithFilter(BvVector<BvPath>& fileList, const BvWString& path, const BvWString& pathWithFilter)
{
	WIN32_FIND_DATAW findData;
	auto hFind = FindFirstFileW(pathWithFilter.CStr(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		// TODO: Handle error
		return;
	}

	do
	{
		BvWString filename(findData.cFileName);
		if (filename != L"." && filename != L"..")
		{
			filename.Insert(path, 0);
			filename.Insert(L'\\', path.Size());
			fileList.EmplaceBack(filename);
		}
	} while (FindNextFileW(hFind, &findData) != FALSE);

	DWORD error = GetLastError();
	if (error != ERROR_NO_MORE_FILES)
	{
		// TODO: Handle error
		return;
	}

	if (!FindClose(hFind))
	{
		// TODO: Handle error
	}
}


BvVector<BvPath> BvPath::GetFileList(const char* pFilter) const
{
	BvVector<BvPath> fileList;
	if (!IsDirectory())
	{
		return fileList;
	}

	BvWString pathWithFilter;
	BvPathUtils::ConvertToWidePath(pathWithFilter, pFilter, pFilter + strlen(pFilter));
	if (m_Path[m_Path.Size() - 1] != L'\\')
	{
		pathWithFilter.Insert(L'\\', 0);
	}
	pathWithFilter.Insert(m_Path, 0, m_Path.Size(), 0);

	GetFileListFromPathWithFilter(fileList, m_Path, pathWithFilter);

	return fileList;
}


BvVector<BvPath> BvPath::GetFileList(const wchar_t* pFilter) const
{
	BvVector<BvPath> fileList;
	if (!IsDirectory())
	{
		return fileList;
	}

	BvWString pathWithFilter(m_Path);
	if (m_Path[m_Path.Size() - 1] != L'\\')
	{
		pathWithFilter.Append(L'\\');
	}
	pathWithFilter.Append(pFilter);

	GetFileListFromPathWithFilter(fileList, m_Path, pathWithFilter);

	return fileList;
}


namespace BvPathUtils
{
	void ConvertToWidePath(BvWString& dst, const char* pSrcBegin, const char* pSrcEnd, const std::locale& loc)
	{
		auto const& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);

		dst.Resize(static_cast<u32>(pSrcEnd - pSrcBegin));

		std::codecvt_base::result result = std::codecvt_base::ok;
		do 
		{
			mbstate_t state{};
			wchar_t* pDstBegin = &dst[0];
			wchar_t* pDstEnd = pDstBegin + dst.Size();
			wchar_t* pDstNext = nullptr;
			const char* pSrcNext = nullptr;

			result = facet.in(state, pSrcBegin, pSrcEnd, pSrcNext, pDstBegin, pDstEnd, pDstNext);

			if (pSrcNext < pSrcBegin || pSrcNext > pSrcEnd || pDstNext < pDstBegin || pDstNext > pDstEnd)
			{
				BvAssert(false, "Invalid string conversion");
				return;
			}

			switch (result)
			{
			case std::codecvt_base::partial:
				if (pSrcNext == pSrcEnd && pDstNext != pDstEnd)
				{
					BvAssert(false, "Invalid string conversion");
					return;
				}

				dst.Resize(dst.Size() * 2);
				break;
			case std::codecvt_base::error:
			case std::codecvt_base::noconv:
				BvAssert(false, "Invalid string conversion");
				return;
			default:
				break;
			}

		} while (result != std::codecvt_base::ok);
	}


	bool IsSlash(wchar_t ch)
	{
		return ch == L'\\' || ch == L'/';
	}


	u32 GetEndOfRoot(const wchar_t* pPath, u32 size)
	{
		if (size < 2)
		{
			return 0;
		}

		if (HasDriveLetter(pPath, size))
		{
			return 2;
		}

		// The following paths all begin with a slash
		if (!IsSlash(pPath[0]))
		{
			return 0;
		}

		// Check for \\?\, \??\ or \\.\ 
		if (size >= 4 && IsSlash(pPath[3]) && (size == 4 || !IsSlash(pPath[4])) // \xx\$
			&& ((IsSlash(pPath[1]) && (pPath[2] == L'?' || pPath[2] == L'.')) // \\?\$ or \\.\$
				|| (pPath[1] == L'?' && pPath[2] == L'?'))) // \??\$
		{
			return 3;
		}

		// Check for \\server
		if (size >= 3 && IsSlash(pPath[1]) && !IsSlash(pPath[2]))
		{
			return static_cast<u32>(std::find_if(pPath, pPath + size, IsSlash) - pPath);
		}

		return 0;
	}


	bool HasRootName(const wchar_t* pPath, u32 size)
	{
		return !GetRootName(pPath, size).empty();
	}


	std::wstring_view GetRootName(const wchar_t* pPath, u32 size)
	{
		return std::wstring_view(pPath, GetEndOfRoot(pPath, size));
	}


	bool HasRootSlash(const wchar_t* pPath, u32 size)
	{
		auto pEndOfRoot = pPath + GetEndOfRoot(pPath, size);
		auto pRelPath = std::find_if_not(pEndOfRoot, pPath + size, IsSlash);

		return static_cast<u32>(pRelPath - pEndOfRoot) > 0;
	}


	bool HasDriveLetter(const wchar_t* pPath, u32 size)
	{
		return size >= 2 && iswalpha(pPath[0]) && pPath[1] == L':';
	}


	bool IsPathAbsolute(const wchar_t* pPath, u32 size)
	{
		if (HasDriveLetter(pPath, size))
		{
			return size >= 3 && IsSlash(pPath[2]);
		}

		return GetEndOfRoot(pPath, size) != 0;
	}


	u32 FindRelativePath(const wchar_t* pPath, u32 size)
	{
		auto pFirst = pPath + GetEndOfRoot(pPath, size);
		return static_cast<u32>(std::find_if_not(pFirst, pPath + size, IsSlash) - pPath);
	}


	u32 FindName(const wchar_t* pPath, u32 size)
	{
		if (size == 0)
		{
			return 0;
		}

		auto pFirst = pPath + FindRelativePath(pPath, size);
		auto pLast = pPath + size;
		while (pLast != pFirst && !IsSlash(pLast[-1]))
		{
			--pLast;
		}

		return static_cast<u32>(pLast - pPath);
	}


	u32 FindExtension(const wchar_t* pPath, u32 size)
	{
		if (size <= 1)
		{
			size;
		}

		auto pExtension = pPath + size - 1;
		if (*pExtension == L'.')
		{
			if (size == 2 && pExtension[-1] == L'.') // ..
			{
				return size;
			}
			else
			{
				return static_cast<u32>(pExtension - pPath);
			}
		}

		while (pPath != --pExtension)
		{
			if (*pExtension == L'.')
			{
				return static_cast<u32>(pExtension - pPath);
			}
		}

		return size;
	}


	u32 GetFileAttributeFlags(const BvWString& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA fad;
		return GetFileAttributesExW(path.CStr(), GetFileExInfoStandard, &fad) ? fad.dwFileAttributes : 0;
	}


	bool HasUNCPathAndDrive(const wchar_t* pPath, u32 size)
	{
		std::wstring_view view(pPath, size);
		return size >= 6 && view.starts_with(L"\\\\?\\") && iswalpha(pPath[4]) && pPath[5] == L':';
	}


	bool IsRelativePathWithRootName(const wchar_t* pPath, u32 size)
	{
		auto pFirst = pPath + FindRelativePath(pPath, size);
		auto pLast = pPath + size;

		while (pFirst != pLast)
		{
			auto pNext = std::find_if(pFirst, pLast, IsSlash);
			if (GetEndOfRoot(pFirst, static_cast<u32>(pNext - pFirst)) != 0)
			{
				return true;
			}

			pFirst = std::find_if_not(pNext, pLast, IsSlash);
		}

		return false;
	}

	void NormalizePath(const BvWString& path, BvWString& normalizedPath)
	{
		if (path.Empty())
		{
			return;
		}

		BvVector<std::wstring_view> parts;
		parts.Reserve(16);

		auto rootEndIndex = BvPathUtils::GetEndOfRoot(path.CStr(), path.Size());;
		normalizedPath.Append(path.CStr(), 0, rootEndIndex);

		auto pRootEnd = path.CStr() + rootEndIndex;
		auto pEnd = path.CStr() + path.Size();
		auto pWalker = pRootEnd;

		// Add the first separator if one exists and skip if there's more than one
		bool hasRootSlash = false;
		if (pWalker != pEnd && BvPathUtils::IsSlash(*pWalker))
		{
			hasRootSlash = true;
			normalizedPath.Append(L'\\');
			while (++pWalker != pEnd && BvPathUtils::IsSlash(*pWalker))
			{
			}
		}

		while (pWalker != pEnd)
		{
			if (BvPathUtils::IsSlash(*pWalker))
			{
				if (parts.Empty() || !parts.Back().empty())
				{
					parts.EmplaceBack();
				}
				++pWalker;
			}
			else
			{
				auto pNameEnd = std::find_if(pWalker + 1, pEnd, BvPathUtils::IsSlash);
				parts.EmplaceBack(pWalker, static_cast<u32>(pNameEnd - pWalker));
				pWalker = pNameEnd;
			}
		}

		constexpr std::wstring_view dot = L".";
		constexpr std::wstring_view dotDot = L"..";
		u32 indexToDelete = 0u;
		for (auto i = 0u; i < parts.Size();)
		{
			const auto& part = parts[i++];
			if (part == dot)
			{
				if (i == parts.Size())
				{
					break;
				}
			}
			else if (part != dotDot)
			{
				parts[indexToDelete++] = part;
				if (i == parts.Size())
				{
					break;
				}
				++indexToDelete;
			}
			else
			{
				if (indexToDelete > 0 && parts[indexToDelete - 2] != dotDot)
				{
					indexToDelete -= 2;
					if (i == parts.Size())
					{
						break;
					}
				}
				else if (!hasRootSlash)
				{
					parts[indexToDelete++] = dotDot;
					if (i == parts.Size())
					{
						break;
					}
					++indexToDelete;
				}
				else
				{
					if (i == parts.Size())
					{
						break;
					}
				}
			}
			++i;
		}
		parts.Erase(parts.begin() + indexToDelete, parts.end());

		if (parts.Size() >= 2 && parts.Back().empty() && parts.end()[-2] == dotDot)
		{
			parts.PopBack();
		}

		for (const auto& part : parts)
		{
			if (part.empty())
			{
				normalizedPath.Append(L'\\');
			}
			else
			{
				normalizedPath.Append(part.data(), 0, part.size());
			}
		}

		if (normalizedPath.Empty())
		{
			normalizedPath.Append(L'\\');
		}
	}


	u32 GetParentPathEnd(const wchar_t* pPath, u32 size)
	{
		const wchar_t* pLast = pPath + size;

		while (pLast != pPath && !BvPathUtils::IsSlash(pLast[-1]))
		{
			--pLast;
		}

		while (pLast != pPath && BvPathUtils::IsSlash(pLast[-1]))
		{
			--pLast;
		}

		return static_cast<u32>(pLast - pPath);
	}


	void AppendPath(BvWString& result, const wchar_t* pPath)
	{
		if (!BvPathUtils::IsSlash(result.GetLastChar()) && !BvPathUtils::IsSlash(*pPath))
		{
			result.Append(L'\\');
		}

		result.Append(pPath);
	}


	void AppendPath(BvWString& result, const BvWString& path)
	{
		if (!BvPathUtils::IsSlash(result.GetLastChar()) && !BvPathUtils::IsSlash(path[0]))
		{
			result.Append(L'\\');
		}

		result.Append(path);
	}
}