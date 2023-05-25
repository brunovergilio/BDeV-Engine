#include "BDeV/System/File/BvFileSystem.h"
#include "BDeV/System/File/Windows/BvFileUtilsWindows.h"
#include <Windows.h>
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


bool BvFileSystem::DelFile(const char* const pFileName)
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


bool BvFileSystem::DelFile(const wchar_t* const pFileName)
{
	if (!DeleteFileW(pFileName))
	{
		DWORD error = GetLastError();

		// TO-DO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::DirExists(const char * const pDirName)
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


bool BvFileSystem::MakeDir(const char* const pDirName)
{
	if (!CreateDirectoryA(pDirName, nullptr))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::DelDir(const char* const pDirName, bool recurse)
{
	if (recurse)
	{
		auto attributes = GetFileAttributesA(pDirName);
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			// TODO: Handle error
			return false;
		}
		bool isFile = (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;

		HANDLE hFind = nullptr;
		WIN32_FIND_DATAA findData;
		BvString filePath(pDirName);
		if (!isFile && filePath[filePath.Size() - 1] != L'\\')
		{
			filePath.Append(R"(\)");
		}

		{
			char filePathWithFilter[kMaxPathSize];
			strncpy_s(filePathWithFilter, filePath.CStr(), kMaxPathSize - 1);
			strncat_s(filePathWithFilter, R"(*.*)", 4);

			hFind = FindFirstFileA(filePathWithFilter, &findData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				DWORD error = GetLastError();
				// TODO: Handle error
				return false;
			}
		}

		do
		{
			BvString filename(findData.cFileName);
			if (filename != "." && filename != "..")
			{
				filename.Insert(filePath, 0);
				isFile = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
				if (isFile)
				{
					DeleteFileA(filename.CStr());
				}
				else
				{
					filename.Append(R"(\)");
					DelDir(filename.CStr(), recurse);
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


bool BvFileSystem::DirExists(const wchar_t* const pDirName)
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


bool BvFileSystem::MakeDir(const wchar_t* const pDirName)
{
	if (!CreateDirectoryW(pDirName, nullptr))
	{
		DWORD error = GetLastError();
		// TODO: Handle error

		return false;
	}

	return true;
}


bool BvFileSystem::DelDir(const wchar_t* const pDirName, bool recurse)
{
	if (recurse)
	{
		auto attributes = GetFileAttributesW(pDirName);
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			// TODO: Handle error
			return false;
		}
		bool isFile = (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;

		HANDLE hFind = nullptr;
		WIN32_FIND_DATAW findData;
		BvWString filePath(pDirName);
		if (!isFile && filePath[filePath.Size() - 1] != L'\\')
		{
			filePath.Append(LR"(\)");
		}

		{
			wchar_t filePathWithFilter[kMaxPathSize];
			wcsncpy_s(filePathWithFilter, filePath.CStr(), kMaxPathSize - 1);
			wcsncat_s(filePathWithFilter, LR"(*.*)", 4);

			hFind = FindFirstFileW(filePathWithFilter, &findData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				DWORD error = GetLastError();
				// TODO: Handle error
				return false;
			}
		}

		do 
		{
			BvWString filename(findData.cFileName);
			if (filename != L"." && filename != L"..")
			{
				filename.Insert(filePath, 0);
				isFile = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
				if (isFile)
				{
					DeleteFileW(filename.CStr());
				}
				else
				{
					filename.Append(LR"(\)");
					DelDir(filename.CStr(), recurse);
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
		HANDLE hDevice;

		wchar_t systemPath[kMaxPathSize]{};
		wcsncat_s(systemPath, kMaxPathSize, LR"(\\.\)", 4);

		if (!GetCurrentDirectoryW(kMaxPathSize - 1, systemPath + 4))
		{
			DWORD error = GetLastError();
			// TODO: Handle error
			return 0;
		}

		systemPath[6] = 0;
		hDevice = CreateFileW(systemPath, 0, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			DWORD error = GetLastError();
			// TODO: Handle error
			return 0;
		}

		// Now that we have the device handle for the disk, let us get disk's metadata
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