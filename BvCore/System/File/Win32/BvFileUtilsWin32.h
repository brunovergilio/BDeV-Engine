#pragma once


namespace Internal
{
	constexpr const wchar_t* kPrefix = LR"(\\?\)";
	constexpr size_t kPrefixSize = 4;

	void ConvertToWidePath(wchar_t* pWidePath, const char* pPath);
	void ConvertToWidePathWithPrefix(wchar_t* pWidePath, const char* pPath);
}