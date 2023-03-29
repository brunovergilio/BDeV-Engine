#pragma once


#include "BDeV/Utils/BvUtils.h"
#include <Windows.h>


class BV_API BvSharedLib
{
	BV_NOCOPY(BvSharedLib);

public:
	BvSharedLib();
	BvSharedLib(const char* const pFilename);
	BvSharedLib(BvSharedLib&& rhs) noexcept;
	BvSharedLib& operator=(BvSharedLib&& rhs) noexcept;
	~BvSharedLib();

	template<typename PFN>
	BV_INLINE PFN GetProcAddress(const char* const pFunctionName) const
	{
		return static_cast<PFN>(GetProcAddressInternal(pFunctionName));
	}

private:
	void* GetProcAddressInternal(const char* const pFunctionName) const;

private:
	HMODULE m_hLib{};
};