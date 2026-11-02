#pragma once


class BvDirectory
{
public:
	static bool Exists(const char* pDirName);
	static bool Create(const char* pDirName);
	static bool Delete(const char* pDirName, bool recurse = true);
};