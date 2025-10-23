#include "BDeV/Core/System/File/BvPath.h"
#include "BDeV/Core/System/File/BvFileCommon.h"
#include "BDeV/Core/Utils/BvText.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include <algorithm>


namespace BvPathUtils
{
	bool IsSlash(wchar_t ch);
	u32 GetEndOfRoot(const char* pPath, u32 size);
	bool HasRootName(const char* pPath, u32 size);
	std::string_view GetRootName(const char* pPath, u32 size);
	bool HasRootSlash(const char* pPath, u32 size);
	bool HasDriveLetter(const char* pPath, u32 size);
	bool IsPathAbsolute(const char* pPath, u32 size);
	u32 FindRelativePath(const char* pPath, u32 size);
	u32 FindName(const char* pPath, u32 size);
	u32 FindExtension(const char* pPath, u32 size);
	u32 GetFileAttributeFlags(const BvWString& path);
	bool HasUNCPathAndDrive(const char* pPath, u32 size);
	bool IsRelativePathWithRootName(const char* pPath, u32 size);
	void NormalizePath(const BvString& path, BvString& normalizedPath);
	u32 GetParentPathEnd(const char* pPath, u32 size);
	void AppendPath(BvString& result, const char* pPath);
	void AppendPath(BvString& result, const BvString& path);
}


BvPath::BvPath()
{
}


BvPath::BvPath(const char* pPath)
	: m_Path(pPath)
{
}


BvPath::BvPath(const char* pPath, u32 count)
	: m_Path(pPath, 0, count)
{
}


BvPath::BvPath(const BvString& path)
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
	m_Path = pPath;

	return *this;
}


BvPath& BvPath::operator=(const BvString& path)
{
	m_Path = path;

	return *this;
}


BvPath::BvPath(BvPath&& rhs) noexcept
	: m_Path(std::move(rhs.m_Path))
{
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
		BV_SYS_ERROR();

		return BvPath();
	}

	BvPath currentPath;
	{
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));

		if (!GetCurrentDirectoryW(sizeNeeded, pFilenameW))
		{
			BV_SYS_ERROR();

			return BvPath();
		}

		auto sizeNeededForUTF8 = BvTextUtilities::ConvertWideCharToUTF8Char(pFilenameW, sizeNeeded, nullptr, 0);
		currentPath.m_Path.Resize(sizeNeededForUTF8 - 1, ' ');
		BvTextUtilities::ConvertWideCharToUTF8Char(pFilenameW, sizeNeeded, &currentPath.m_Path[0], sizeNeededForUTF8);
	}

	return currentPath;
}


BvPath BvPath::FromCurrentDrive()
{
	auto sizeNeeded = GetCurrentDirectoryW(0, nullptr);
	if (!sizeNeeded)
	{
		BV_SYS_ERROR();

		return BvPath();
	}

	BvPath currentPath;
	{
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));

		if (!GetCurrentDirectoryW(sizeNeeded, pFilenameW))
		{
			BV_SYS_ERROR();

			return BvPath();
		}

		auto sizeNeededForUTF8 = BvTextUtilities::ConvertWideCharToUTF8Char(pFilenameW, sizeNeeded, nullptr, 0);
		currentPath.m_Path.Resize(sizeNeededForUTF8 - 1);
		BvTextUtilities::ConvertWideCharToUTF8Char(pFilenameW, sizeNeeded, &currentPath.m_Path[0], sizeNeededForUTF8);
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
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, pFilenameW, sizeNeeded);
		WIN32_FILE_ATTRIBUTE_DATA fad;
		return GetFileAttributesExW(pFilenameW, GetFileExInfoStandard, &fad);
	}
}


bool BvPath::IsFile() const
{
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, pFilenameW, sizeNeeded);
		WIN32_FILE_ATTRIBUTE_DATA fad;
		return GetFileAttributesExW(pFilenameW, GetFileExInfoStandard, &fad) ? (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 : false;
	}
}


bool BvPath::IsDirectory() const
{
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, pFilenameW, sizeNeeded);
		WIN32_FILE_ATTRIBUTE_DATA fad;
		return GetFileAttributesExW(pFilenameW, GetFileExInfoStandard, &fad) ? (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 : false;
	}
}


BvString BvPath::GetName() const
{
	auto nameIndex = BvPathUtils::FindName(m_Path.CStr(), m_Path.Size());
	auto extIndex = BvPathUtils::FindExtension(m_Path.CStr(), m_Path.Size());
	auto count = extIndex - nameIndex;

	return BvString(m_Path.CStr(), nameIndex, count);
}


BvString BvPath::GetNameAndExtension() const
{
	auto nameIndex = BvPathUtils::FindName(m_Path.CStr(), m_Path.Size());
	return BvString(m_Path.CStr(), nameIndex, m_Path.Size() - nameIndex);
}


BvString BvPath::GetExtension() const
{
	auto extIndex = BvPathUtils::FindExtension(m_Path.CStr(), m_Path.Size());
	return BvString(m_Path.CStr(), extIndex, m_Path.Size() - extIndex);
}


BvString BvPath::GetRoot() const
{
	auto rootDirEnd = BvPathUtils::GetEndOfRoot(m_Path.CStr(), m_Path.Size());
	return BvString(m_Path, 0, rootDirEnd + (BvPathUtils::IsSlash(m_Path[rootDirEnd]) ? 1 : 0));
}


BvString BvPath::GetRootName() const
{
	auto rootNameView = BvPathUtils::GetRootName(m_Path.CStr(), m_Path.Size());
	return BvString(rootNameView.data(), 0, rootNameView.size());
}


BvString BvPath::GetRootDirectory() const
{
	auto pRootEnd = m_Path.CStr() + BvPathUtils::GetEndOfRoot(m_Path.CStr(), m_Path.Size());
	auto pRelativePath = std::find_if_not(pRootEnd, m_Path.LCStr(), BvPathUtils::IsSlash);
	return BvString(pRootEnd, 0, static_cast<u32>(pRelativePath - pRootEnd));
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

	BvPath path;
	u32 sizeNeeded;
	{
		sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, nullptr, 0);
		wchar_t* pPathNameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(m_Path.CStr(), m_Path.Size() + 1, pPathNameW, sizeNeeded);

		sizeNeeded = GetFullPathNameW(pPathNameW, 0, nullptr, nullptr);
		if (sizeNeeded == 0)
		{
			BV_SYS_ERROR();
			return BvPath();
		}

		BvWString fullPath;
		fullPath.Resize(sizeNeeded - 1);
		GetFullPathNameW(pPathNameW, sizeNeeded, &fullPath[0], nullptr);
		if (sizeNeeded == 0)
		{
			BV_SYS_ERROR();
			return BvPath();
		}
		
		sizeNeeded = BvTextUtilities::ConvertWideCharToUTF8Char(fullPath.CStr(), fullPath.Size() + 1, nullptr, 0);
		path.m_Path.Resize(sizeNeeded);
		BvTextUtilities::ConvertWideCharToUTF8Char(fullPath.CStr(), fullPath.Size() + 1, &path.m_Path[0], sizeNeeded);
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
	BV_ASSERT(pPath != nullptr, "Invalid path string");

	BvPath appendedPath(m_Path);
	BvPathUtils::AppendPath(appendedPath.m_Path, pPath);

	return appendedPath;
}


BvPath BvPath::GetAppendedPath(const BvString& path) const
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
	BV_ASSERT(pPath != nullptr, "Invalid path string");

	BvPath prependedPath(pPath);
	BvPathUtils::AppendPath(prependedPath.m_Path, m_Path);

	return prependedPath;
}


BvPath BvPath::GetPrependedPath(const BvString& path) const
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
	BvString normalizedPath;
	BvPathUtils::NormalizePath(m_Path, normalizedPath);
	std::swap(m_Path, normalizedPath);
}


void BvPath::ConvertToAbsolutePath()
{
	if (IsAbsolute())
	{
		return;
	}

	*this = std::move(GetAbsolutePath());
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
	BV_ASSERT(pPath != nullptr, "Invalid path string");

	BvPathUtils::AppendPath(m_Path, pPath);
}


void BvPath::AppendPath(const BvString& path)
{
	BvPathUtils::AppendPath(m_Path, path);
}


void BvPath::AppendPath(const BvPath& path)
{
	BvPathUtils::AppendPath(m_Path, path.m_Path);
}


void BvPath::PrependPath(const char* pPath)
{
	BV_ASSERT(pPath != nullptr, "Invalid path string");

	BvString path(pPath);
	BvPathUtils::AppendPath(path, m_Path);
	m_Path = std::move(path);
}


void BvPath::PrependPath(const BvString& path)
{
	BvString prependedPath(path);
	BvPathUtils::AppendPath(prependedPath, m_Path);
	m_Path = std::move(prependedPath);
}


void BvPath::PrependPath(const BvPath& path)
{
	BvString prependedPath(path.m_Path);
	BvPathUtils::AppendPath(prependedPath, m_Path);
	m_Path = std::move(prependedPath);
}


BvPath& BvPath::operator/=(const char* pPath)
{
	AppendPath(pPath);

	return *this;
}


BvPath& BvPath::operator/=(const BvString& path)
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


void GetFileListFromPathWithFilter(BvVector<BvPath>& fileList, const BvString& path, const BvString& pathWithFilter)
{
	BvVector<char> filenameBuffer;

	WIN32_FIND_DATAW findData;
	HANDLE hFind;
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pathWithFilter.CStr(), pathWithFilter.Size() + 1, nullptr, 0);
		filenameBuffer.Resize(sizeNeeded * sizeof(wchar_t));
		wchar_t* pFilenameW = (wchar_t*)&filenameBuffer[0];
		BvTextUtilities::ConvertUTF8CharToWideChar(pathWithFilter.CStr(), pathWithFilter.Size() + 1, pFilenameW, sizeNeeded);
		hFind = FindFirstFileW(pFilenameW, &findData);
	}

	if (hFind == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND)
		{
			BV_SYS_FATAL();
		}
		return;
	}

	do
	{
		if (!(wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0))
		{
			BvString filename;
			filename.Append(path);
			filename.Append(L'\\');

			{
				auto sizeNeeded = BvTextUtilities::ConvertWideCharToUTF8Char(findData.cFileName, 0, nullptr, 0);
				filenameBuffer.Resize(sizeNeeded);
				BvTextUtilities::ConvertWideCharToUTF8Char(findData.cFileName, 0, &filenameBuffer[0], sizeNeeded);

				filename.Append(filenameBuffer.Data());
			}

			fileList.EmplaceBack(std::move(filename));
		}
	} while (FindNextFileW(hFind, &findData) != FALSE);

	DWORD error = GetLastError();
	if (error != ERROR_NO_MORE_FILES)
	{
		BV_SYS_FATAL();
		return;
	}

	if (!FindClose(hFind))
	{
		BV_SYS_FATAL();
	}
}


BvVector<BvPath> BvPath::GetFileList(const char* pFilter) const
{
	BvVector<BvPath> fileList;
	if (!IsDirectory())
	{
		return fileList;
	}

	BvString pathWithFilter(m_Path);
	if (m_Path[m_Path.Size() - 1] != '\\')
	{
		pathWithFilter.Append(L'\\');
	}
	pathWithFilter.Append(pFilter);

	GetFileListFromPathWithFilter(fileList, m_Path, pathWithFilter);

	return fileList;
}


namespace BvPathUtils
{
	bool IsSlash(wchar_t ch)
	{
		return ch == L'\\' || ch == L'/';
	}


	u32 GetEndOfRoot(const char* pPath, u32 size)
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


	bool HasRootName(const char* pPath, u32 size)
	{
		return !GetRootName(pPath, size).empty();
	}


	std::string_view GetRootName(const char* pPath, u32 size)
	{
		return std::string_view(pPath, GetEndOfRoot(pPath, size));
	}


	bool HasRootSlash(const char* pPath, u32 size)
	{
		auto pEndOfRoot = pPath + GetEndOfRoot(pPath, size);
		auto pRelPath = std::find_if_not(pEndOfRoot, pPath + size, IsSlash);

		return static_cast<u32>(pRelPath - pEndOfRoot) > 0;
	}


	bool HasDriveLetter(const char* pPath, u32 size)
	{
		return size >= 2 && iswalpha(pPath[0]) && pPath[1] == L':';
	}


	bool IsPathAbsolute(const char* pPath, u32 size)
	{
		if (HasDriveLetter(pPath, size))
		{
			return size >= 3 && IsSlash(pPath[2]);
		}

		return GetEndOfRoot(pPath, size) != 0;
	}


	u32 FindRelativePath(const char* pPath, u32 size)
	{
		auto pFirst = pPath + GetEndOfRoot(pPath, size);
		return static_cast<u32>(std::find_if_not(pFirst, pPath + size, IsSlash) - pPath);
	}


	u32 FindName(const char* pPath, u32 size)
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


	u32 FindExtension(const char* pPath, u32 size)
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


	bool HasUNCPathAndDrive(const char* pPath, u32 size)
	{
		std::string_view view(pPath, size);
		return size >= 6 && view.starts_with("\\\\?\\") && iswalpha(pPath[4]) && pPath[5] == ':';
	}


	bool IsRelativePathWithRootName(const char* pPath, u32 size)
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

	void NormalizePath(const BvString& path, BvString& normalizedPath)
	{
		if (path.Empty())
		{
			return;
		}

		BvVector<std::string_view> parts;
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
			normalizedPath.Append('\\');
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

		constexpr std::string_view dot = ".";
		constexpr std::string_view dotDot = "..";
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
				normalizedPath.Append('\\');
			}
			else
			{
				normalizedPath.Append(part.data(), 0, part.size());
			}
		}

		if (normalizedPath.Empty())
		{
			normalizedPath.Append('\\');
		}
	}


	u32 GetParentPathEnd(const char* pPath, u32 size)
	{
		const char* pLast = pPath + size;

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


	void AppendPath(BvString& result, const char* pPath)
	{
		if (!BvPathUtils::IsSlash(result.GetLastChar()) && !BvPathUtils::IsSlash(*pPath))
		{
			result.Append('\\');
		}

		result.Append(pPath);
	}


	void AppendPath(BvString& result, const BvString& path)
	{
		if (!BvPathUtils::IsSlash(result.GetLastChar()) && !BvPathUtils::IsSlash(path[0]))
		{
			result.Append('\\');
		}

		result.Append(path);
	}
}