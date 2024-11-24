#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"


class BvSharedLib
{
	BV_NOCOPY(BvSharedLib);

public:
	BvSharedLib();
	BvSharedLib(const char* pFilename);
	BvSharedLib(BvSharedLib&& rhs) noexcept;
	BvSharedLib& operator=(BvSharedLib&& rhs) noexcept;
	~BvSharedLib();

	bool Open(const char* pFilename);
	void Close();
	void* GetProcAddress(const char* const pFunctionName) const;

	template<typename PFN>
	BV_INLINE PFN GetProcAddressT(const char* const pFunctionName) const
	{
		return static_cast<PFN>(GetProcAddress(pFunctionName));
	}

	template<typename PFN>
	BV_INLINE void GetProcAddressT(const char* const pFunctionName, PFN& pFn) const
	{
		pFn = static_cast<PFN>(GetProcAddress(pFunctionName));
	}

	BV_INLINE operator bool() const { return m_hLib != kNullOSSharedLibHandle; }

private:
	OSSharedLibHandle m_hLib = kNullOSSharedLibHandle;
};