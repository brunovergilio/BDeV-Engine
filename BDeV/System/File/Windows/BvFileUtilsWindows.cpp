#include "BvFileUtilsWindows.h"
#include "BDeV/System/File/BvFileCommon.h"


void ConvertToWidePath(wchar_t* pWidePath, const char* pPath)
{
	std::mbstate_t state{};
	std::mbsrtowcs(pWidePath, &pPath, kMaxPathSize - 1, &state);
}


// The reason I'm using wchar_t for paths is because on Windows the maximum
// path size for multibyte paths is MAX_PATH (260), but on wchar_t it's
// 32767 if you prepend "\\?\" with it
void ConvertToWidePathWithPrefix(wchar_t* pWidePath, const char* pPath)
{
	wcsncat_s(pWidePath, kMaxPathSize, kWideFilePrefix, kWideFilePrefixSize);
	std::mbstate_t state{};
	std::mbsrtowcs(pWidePath + 4, &pPath, kMaxPathSize - kWideFilePrefixSize - 1, &state);
}


void AddPrefixForMaxPathLimit(wchar_t* pDstPath, const wchar_t* pSourcePath)
{
	wcsncat_s(pDstPath, kMaxPathSize, kWideFilePrefix, kWideFilePrefixSize);
	wcscat_s(pDstPath, kMaxPathSize - kWideFilePrefixSize, pSourcePath);
}