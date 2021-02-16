#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/Utils/BvUtils.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
using LibModule = HMODULE;
#else
#error "Platform not yet supported"
#endif


class BvSharedLib
{
	BV_NOCOPY(BvSharedLib);

public:
	BvSharedLib();
	BvSharedLib(const char * const pFilename);
	BvSharedLib(BvSharedLib && rhs);
	BvSharedLib & operator=(BvSharedLib && rhs);
	~BvSharedLib();

	template<typename PFN>
	inline PFN GetProcAddress(const char * const pFunctionName) const
	{
		return static_cast<PFN>(GetProcAddressInternal(pFunctionName));
	}

private:
	void * GetProcAddressInternal(const char * const pFunctionName) const;

private:
	LibModule m_hLib{};
};