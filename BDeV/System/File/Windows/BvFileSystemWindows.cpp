#include "BDeV/System/File/BvFileSystem.h"
#include "BDeV/System/Windows/BvWindowsHeader.h"
#include <winioctl.h>


bool BvFileSystem::FileExists(const char * pFileName)
{
	auto attrib = GetFileAttributesA(pFileName);
	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
		{
			BvAssert(error != 0, "Some other error happened");
		}
		return false;
	}

	return (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
}


bool BvFileSystem::DeleteFile(const char* const pFileName)
{
	if (!DeleteFileA(pFileName))
	{
		DWORD error = GetLastError();

		// TO-DO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::FileExists(const wchar_t* const pFileName)
{
	auto attrib = GetFileAttributesW(pFileName);
	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
		{
			BvAssert(error != 0, "Some other error happened");
		}
		return false;
	}

	return (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
}


bool BvFileSystem::DeleteFile(const wchar_t* const pFileName)
{
	if (!DeleteFileW(pFileName))
	{
		DWORD error = GetLastError();

		// TO-DO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::DirectoryExists(const char * const pDirName)
{
	auto attrib = GetFileAttributesA(pDirName);
	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
		{
			BvAssert(error != 0, "Some other error happened");
		}
		return false;
	}

	return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


bool BvFileSystem::CreateDirectory(const char* const pDirName)
{
	if (!CreateDirectoryA(pDirName, nullptr))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::DeleteDirectory(const char* const pDirName, bool recurse)
{
	if (recurse)
	{
		auto attributes = GetFileAttributesA(pDirName);
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			// TODO: Handle error
			return false;
		}
		else if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			return false;
		}

		HANDLE hFind = nullptr;
		WIN32_FIND_DATAA findData;
		BvString filePath(pDirName);
		if (filePath[filePath.Size() - 1] != '\\')
		{
			filePath.Append("\\");
		}
		filePath.Append("*.*");

		hFind = FindFirstFileA(filePath.CStr(), &findData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			DWORD error = GetLastError();
			// TODO: Handle error
			return false;
		}

		do
		{
			BvString filename(findData.cFileName);
			if (filename != "." && filename != "..")
			{
				filename.Insert(filePath, 0);
				if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					DeleteFileA(filename.CStr());
				}
				else
				{
					filename.Append("\\");
					DeleteDirectory(filename.CStr(), recurse);
				}
			}
		} while (FindNextFileA(hFind, &findData) != FALSE);

		DWORD error = GetLastError();
		if (error != ERROR_NO_MORE_FILES)
		{
			// TODO: Handle error
			return false;
		}
		if (!FindClose(hFind))
		{
			// TODO: Handle error
			return false;
		}
	}

	if (!RemoveDirectoryA(pDirName))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::DirectoryExists(const wchar_t* const pDirName)
{
	auto attrib = GetFileAttributesW(pDirName);
	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
		{
			BvAssert(error != 0, "Some other error happened");
		}
		return false;
	}

	return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


bool BvFileSystem::CreateDirectory(const wchar_t* const pDirName)
{
	if (!CreateDirectoryW(pDirName, nullptr))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::DeleteDirectory(const wchar_t* const pDirName, bool recurse)
{
	if (recurse)
	{
		auto attributes = GetFileAttributesW(pDirName);
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			// TODO: Handle error
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
			DWORD error = GetLastError();
			// TODO: Handle error
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
					DeleteFileW(filename.CStr());
				}
				else
				{
					filename.Append(L"\\");
					DeleteDirectory(filename.CStr(), recurse);
				}
			}
		} while (FindNextFileW(hFind, &findData) != FALSE);

		DWORD error = GetLastError();
		if (error != ERROR_NO_MORE_FILES)
		{
			// TODO: Handle error
			return false;
		}
		if (!FindClose(hFind))
		{
			// TODO: Handle error
			return false;
		}
	}

	if (!RemoveDirectoryW(pDirName))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return false;
	}

	return true;
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
			DWORD error = GetLastError();
			// TODO: Handle error
			return 0;
		}

		HANDLE hDevice = CreateFileW(drivePath, 0, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			DWORD error = GetLastError();
			// TODO: Handle error
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
			DWORD error = GetLastError();
			// TODO: Handle error
			return 0;
		}
		CloseHandle(hDevice);

		return diskAlignment.BytesPerPhysicalSector;
	}();

	return sectorSize;
}