#include "BDeV/Core/System/File/BvDirectory.h"
#include "BDeV/Core/Utils/BvUTF.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include "BDeV/Core/Container/BvString.h"


bool BvDirectory::Exists(const char * const pDirName)
{
	std::string_view sv(pDirName);
	auto sizeNeeded = BvUTFCharTraits::LengthFor<wchar_t>(sv.begin(), sv.end()) + 1;
	wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
	auto it = BvUTFCharTraits::GetStr(sv.begin(), sv.end(), pFilenameW, pFilenameW + sizeNeeded);
	*it = 0;
	DWORD attrib = GetFileAttributesW(pFilenameW);

	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
		{
			BV_SYS_ERROR();
		}
		return false;
	}

	return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


bool BvDirectory::Create(const char* const pDirName)
{
	std::string_view sv(pDirName);
	auto sizeNeeded = BvUTFCharTraits::LengthFor<wchar_t>(sv.begin(), sv.end()) + 1;
	wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
	auto it = BvUTFCharTraits::GetStr(sv.begin(), sv.end(), pFilenameW, pFilenameW + sizeNeeded);
	*it = 0;
	BOOL result = CreateDirectoryW(pFilenameW, nullptr);

	if (!result)
	{
		BV_SYS_ERROR();

		return false;
	}

	return true;
}


namespace Internal
{
	bool DeleteDirectoryInternal(const wchar_t* const pDirName, bool recurse)
	{
		if (recurse)
		{
			auto attributes = GetFileAttributesW(pDirName);
			if (attributes == INVALID_FILE_ATTRIBUTES)
			{
				BV_SYS_ERROR();
				return false;
			}
			else if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				return false;
			}

			HANDLE hFind = nullptr;
			WIN32_FIND_DATAW findData;
			BvWString filePath(pDirName);
			if (filePath[filePath.Size() - 1] != L'\\')
			{
				filePath.Append(L"\\");
			}
			filePath.Append(L"*.*");

			hFind = FindFirstFileW(filePath.CStr(), &findData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				BV_SYS_ERROR();
				return false;
			}

			do
			{
				BvWString filename(findData.cFileName);
				if (filename != L"." && filename != L"..")
				{
					filename.Insert(filePath, 0);
					if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						if (!DeleteFileW(filename.CStr()))
						{
							BV_SYS_ERROR();
							return false;
						}
					}
					else
					{
						filename.Append(L"\\");
						return DeleteDirectoryInternal(filename.CStr(), recurse);
					}
				}
			} while (FindNextFileW(hFind, &findData) != FALSE);

			DWORD error = GetLastError();
			if (error != ERROR_NO_MORE_FILES)
			{
				BV_SYS_ERROR();
				return false;
			}
			if (!FindClose(hFind))
			{
				BV_SYS_ERROR();
				return false;
			}
		}

		if (!RemoveDirectoryW(pDirName))
		{
			BV_SYS_ERROR();

			return false;
		}

		return true;
	}
}


bool BvDirectory::Delete(const char* const pDirName, bool recurse)
{
	bool result = true;
	{
		std::string_view sv(pDirName);
		auto sizeNeeded = BvUTFCharTraits::LengthFor<wchar_t>(sv.begin(), sv.end());
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		auto it = BvUTFCharTraits::GetStr(sv.begin(), sv.end(), pFilenameW, pFilenameW + sizeNeeded);
		*it = 0;
		result = Internal::DeleteDirectoryInternal(pFilenameW, recurse);
	}

	return result;
}