#include "BvFileUtilsWin32.h"
#include "BvCore/System/File/BvFileCommon.h"


namespace Internal
{
	void ConvertToWidePath(wchar_t* pWidePath, const char* pPath)
	{
		std::mbstate_t state{};
		std::mbrtowc(pWidePath, pPath, kMaxPathSize - 1, &state);
	}


	// The reason I'm using wchar_t for paths is because on Windows the maximum
	// path size for multibyte paths is MAX_PATH (260), but on wchar_t it's
	// 32767 if you prepend "\\?\" with it
	void ConvertToWidePathWithPrefix(wchar_t* pWidePath, const char* pPath)
	{
		wcsncat_s(pWidePath, kMaxPathSize, kPrefix, kPrefixSize);
		std::mbstate_t state{};
		std::mbrtowc(pWidePath + 4, pPath, kMaxPathSize - kPrefixSize - 1, &state);
	}
}