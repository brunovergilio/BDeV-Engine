#include "BDeV/Core/System/File/BvFileSystem.h"
#include "BDeV/Core/Utils/BvText.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include <winioctl.h>


bool BvFileSystem::FileExists(const char * pFilename)
{
	DWORD attrib = 0;
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, pFilenameW, sizeNeeded);
		attrib = GetFileAttributesW(pFilenameW);
	}

	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
		{
			BV_SYS_ERROR();
		}
		return false;
	}

	return (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
}


bool BvFileSystem::DeleteFile(const char* const pFilename)
{
	BOOL result = TRUE;
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, pFilenameW, sizeNeeded);
		result = DeleteFileW(pFilenameW);
	}

	if (!result)
	{
		BV_SYS_ERROR();

		return false;
	}

	return true;
}


bool BvFileSystem::DirectoryExists(const char * const pDirName)
{
	DWORD attrib = 0;
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pDirName, 0, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pDirName, 0, pFilenameW, sizeNeeded);
		attrib = GetFileAttributesW(pFilenameW);
	}

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


bool BvFileSystem::CreateDirectory(const char* const pDirName)
{
	BOOL result = TRUE;
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pDirName, 0, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pDirName, 0, pFilenameW, sizeNeeded);
		result = CreateDirectoryW(pFilenameW, nullptr);
	}

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


bool BvFileSystem::DeleteDirectory(const char* const pDirName, bool recurse)
{
	bool result = true;
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pDirName, 0, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pDirName, 0, pFilenameW, sizeNeeded);
		result = Internal::DeleteDirectoryInternal(pFilenameW, recurse);
	}

	return result;
}


u32 BvFileSystem::GetPhysicalSectorSize()
{
	static u32 sectorSize = []() -> u32
	{
		auto sizeNeeded = GetCurrentDirectoryW(0, nullptr);
		wchar_t drivePath[8]{};
		wcsncat_s(drivePath, 8, L"\\\\.\\", 4);

		if (!GetCurrentDirectoryW(3, drivePath + 4))
		{
			BV_SYS_ERROR();
			return 0;
		}

		HANDLE hDevice = CreateFileW(drivePath, 0, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			BV_SYS_ERROR();
			return 0;
		}

		// Now that we have the device handle for the disk, let us get the disk's metadata
		DWORD outsize;
		STORAGE_PROPERTY_QUERY storageQuery{};
		storageQuery.PropertyId = StorageAccessAlignmentProperty;
		storageQuery.QueryType = PropertyStandardQuery;

		STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR diskAlignment{};
		if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storageQuery, sizeof(STORAGE_PROPERTY_QUERY),
			&diskAlignment, sizeof(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR), &outsize, nullptr))
		{
			BV_SYS_ERROR();
			return 0;
		}
		CloseHandle(hDevice);

		return diskAlignment.BytesPerPhysicalSector;
	}();

	return sectorSize;
}