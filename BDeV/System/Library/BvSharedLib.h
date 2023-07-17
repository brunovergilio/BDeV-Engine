#pragma once


#include "BDeV/Utils/BvUtils.h"


class BvSharedLib
{
	BV_NOCOPY(BvSharedLib);

public:
	BvSharedLib();
	BvSharedLib(const char* pFilename);
	BvSharedLib(BvSharedLib&& rhs) noexcept;
	BvSharedLib& operator=(BvSharedLib&& rhs) noexcept;
	~BvSharedLib();

	void* GetProcAddress(const char* const pFunctionName) const;

	template<typename PFN>
	BV_INLINE PFN GetProcAddressT(const char* const pFunctionName) const
	{
		return static_cast<PFN>(GetProcAddress(pFunctionName));
	}

private:
	void* m_hLib = nullptr;
};