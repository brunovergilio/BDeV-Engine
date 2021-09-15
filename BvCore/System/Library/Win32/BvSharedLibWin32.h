#pragma once


#include "BvCore/Utils/BvUtils.h"
#include <Windows.h>


class BvSharedLib
{
	BV_NOCOPY(BvSharedLib);

public:
	BvSharedLib();
	BvSharedLib(const char* const pFilename);
	BvSharedLib(BvSharedLib&& rhs);
	BvSharedLib& operator=(BvSharedLib&& rhs);
	~BvSharedLib();

	template<typename PFN>
	inline PFN GetProcAddress(const char* const pFunctionName) const
	{
		return static_cast<PFN>(GetProcAddressInternal(pFunctionName));
	}

private:
	void* GetProcAddressInternal(const char* const pFunctionName) const;

private:
	HMODULE m_hLib{};
};